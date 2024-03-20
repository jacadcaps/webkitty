/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !__has_feature(objc_arc)
#error This file requires ARC. Add the "-fobjc-arc" compiler flag for this file.
#endif

#import "config.h"
#import "WebExtensionContext.h"

#if ENABLE(WK_WEB_EXTENSIONS)

#import "CocoaHelpers.h"
#import "WebExtensionConstants.h"
#import "WebExtensionContextProxy.h"
#import "WebExtensionContextProxyMessages.h"
#import "WebExtensionStorageAccessLevel.h"
#import "WebExtensionStorageType.h"
#import "WebExtensionUtilities.h"
#import "_WKWebExtensionStorageSQLiteStore.h"
#import <wtf/BlockPtr.h>
#import <wtf/cocoa/VectorCocoa.h>

namespace WebKit {

void WebExtensionContext::storageGet(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, const Vector<String>& keys, CompletionHandler<void(std::optional<String> dataJSON, ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = [NSString stringWithFormat:@"%@.get()", (NSString *)toAPIPrefixString(storageType)];

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(std::nullopt, toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    auto storage = storageForType(storageType);
    [storage getValuesForKeys:createNSArray(keys).get() completionHandler:makeBlockPtr([&, completionHandler = WTFMove(completionHandler)](NSDictionary<NSString *, NSString *> *values, NSString *errorMessage) mutable {
        if (errorMessage)
            completionHandler(std::nullopt, toErrorString(callingAPIName, nil, errorMessage));
        else
            completionHandler(encodeJSONString(values), std::nullopt);
    }).get()];
}

void WebExtensionContext::storageGetBytesInUse(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, const Vector<String>& keys, CompletionHandler<void(std::optional<size_t> size, ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = [NSString stringWithFormat:@"%@.getBytesInUse()", (NSString *)toAPIPrefixString(storageType)];

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(std::nullopt, toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    auto storage = storageForType(storageType);
    [storage getStorageSizeForKeys:createNSArray(keys).get() completionHandler:makeBlockPtr([&, completionHandler = WTFMove(completionHandler)](size_t size, NSString *errorMessage) mutable {
        if (errorMessage)
            completionHandler(std::nullopt, toErrorString(callingAPIName, nil, errorMessage));
        else
            completionHandler(size, std::nullopt);
    }).get()];
}

void WebExtensionContext::storageSet(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, const String& dataJSON, CompletionHandler<void(ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = [NSString stringWithFormat:@"%@.set()", (NSString *)toAPIPrefixString(storageType)];

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    NSDictionary *data = parseJSON(dataJSON);

    [storageForType(storageType) getStorageSizeForAllKeysIncludingKeyedData:data withCompletionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, storageType, retainData = RetainPtr { data }, completionHandler = WTFMove(completionHandler)](size_t size, NSUInteger numberOfKeys, NSDictionary<NSString *, NSString *> *existingKeysAndValues, NSString *errorMessage) mutable {
        if (errorMessage) {
            completionHandler(toErrorString(callingAPIName, nil, errorMessage));
            return;
        }

        if (size > quoataForStorageType(storageType)) {
            completionHandler(toErrorString(callingAPIName, nil, @"exceeded storage quota"));
            return;
        }

        if (storageType == WebExtensionStorageType::Sync && numberOfKeys > webExtensionStorageAreaSyncMaximumItems) {
            completionHandler(toErrorString(callingAPIName, nil, @"exceeded maximum number of items"));
            return;
        }

        [storageForType(storageType) setKeyedData:retainData.get() completionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, retainData, storageType, existingKeysAndValues = RetainPtr { existingKeysAndValues }, completionHandler = WTFMove(completionHandler)](NSArray *keysSuccessfullySet, NSString *errorMessage) mutable {
            if (errorMessage)
                completionHandler(toErrorString(callingAPIName, nil, errorMessage));
            else
                completionHandler(std::nullopt);

            // Only fire an onChanged event for the keys that were successfully set.
            if (!keysSuccessfullySet.count)
                return;

            auto *data = retainData.get();
            if (keysSuccessfullySet.count != data.allKeys.count)
                data = dictionaryWithKeys(data, keysSuccessfullySet);

            fireStorageChangedEventIfNeeded(existingKeysAndValues.get(), data, storageType);
        }).get()];
    }).get()];
}

void WebExtensionContext::storageRemove(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, const Vector<String>& keys, CompletionHandler<void(ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = [NSString stringWithFormat:@"%@.remove()", (NSString *)toAPIPrefixString(storageType)];

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    [storageForType(storageType) getValuesForKeys:createNSArray(keys).get() completionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, keys, storageType, completionHandler = WTFMove(completionHandler)](NSDictionary<NSString *, NSString *> *oldValuesAndKeys, NSString *errorMessage) mutable {
        if (errorMessage) {
            completionHandler(toErrorString(callingAPIName, nil, errorMessage));
            return;
        }

        [storageForType(storageType) deleteValuesForKeys:createNSArray(keys).get() completionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, storageType, oldValuesAndKeys = RetainPtr { oldValuesAndKeys }, completionHandler = WTFMove(completionHandler)](NSString *errorMessage) mutable {
            if (errorMessage) {
                completionHandler(toErrorString(callingAPIName, nil, errorMessage));
                return;
            }

            fireStorageChangedEventIfNeeded(oldValuesAndKeys.get(), nil, storageType);
            completionHandler(std::nullopt);
        }).get()];
    }).get()];
}

void WebExtensionContext::storageClear(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, CompletionHandler<void(ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = [NSString stringWithFormat:@"%@.clear()", (NSString *)toAPIPrefixString(storageType)];

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    [storageForType(storageType) getValuesForKeys:@[ ] completionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, storageType, completionHandler = WTFMove(completionHandler)](NSDictionary<NSString *, NSString *> *oldValuesAndKeys, NSString *errorMessage) mutable {
        if (errorMessage) {
            completionHandler(toErrorString(callingAPIName, nil, errorMessage));
            return;
        }

        [storageForType(storageType) deleteDatabaseWithCompletionHandler:makeBlockPtr([this, protectedThis = Ref { *this }, oldValuesAndKeys = RetainPtr { oldValuesAndKeys }, storageType, completionHandler = WTFMove(completionHandler)](NSString *errorMessage) mutable {
            if (errorMessage) {
                completionHandler(toErrorString(callingAPIName, nil, errorMessage));
                return;
            }

            fireStorageChangedEventIfNeeded(oldValuesAndKeys.get(), nil, storageType);
            completionHandler(std::nullopt);
        }).get()];
    }).get()];
}

void WebExtensionContext::storageSetAccessLevel(WebPageProxyIdentifier webPageProxyIdentifier, WebExtensionStorageType storageType, const WebExtensionStorageAccessLevel accessLevel, CompletionHandler<void(ErrorString)>&& completionHandler)
{
    static NSString * const callingAPIName = @"browser.session.setAccessLevel()";

    if (!extensionCanAccessWebPage(webPageProxyIdentifier)) {
        completionHandler(toErrorString(callingAPIName, nil, @"access not allowed"));
        return;
    }

    setSessionStorageAllowedInContentScripts(accessLevel == WebExtensionStorageAccessLevel::TrustedAndUntrustedContexts);

    completionHandler(std::nullopt);
}

void WebExtensionContext::fireStorageChangedEventIfNeeded(NSDictionary *oldKeysAndValues, NSDictionary *newKeysAndValues, WebExtensionStorageType storageType)
{
    static NSString * const newValueKey = @"newValue";
    static NSString * const oldValueKey = @"oldValue";

    if (!oldKeysAndValues.count && !newKeysAndValues.count)
        return;

    auto *changedData = [NSMutableDictionary dictionary];

    if (!newKeysAndValues) {
        [oldKeysAndValues enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *value, BOOL *) {
            changedData[key] = @{ oldValueKey: parseJSON(value, { JSONOptions::FragmentsAllowed }) };
        }];
    } else {
        [newKeysAndValues enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *value, BOOL *) {
            if (NSString *oldValue = oldKeysAndValues[key]) {
                if (![oldValue isEqualToString:value]) {
                    changedData[key] = @{
                        oldValueKey: parseJSON(oldValue, { JSONOptions::FragmentsAllowed }),
                        newValueKey: parseJSON(value, { JSONOptions::FragmentsAllowed }),
                    };
                }

                return;
            }

            // A new key is being added for the first time.
            changedData[key] = @{ newValueKey: parseJSON(value, { JSONOptions::FragmentsAllowed }) };
        }];
    }

    if (!changedData.count)
        return;

    constexpr auto type = WebExtensionEventListenerType::StorageOnChanged;
    auto jsonString = encodeJSONString(changedData);

    // Unlike other extension events which are only dispatched to the web process that hosts all the extension-related web views (background page, popup, full page extension content),
    // content scripts are allowed to listen to storage.onChanged events.
    sendToContentScriptProcessesForEvent(type, Messages::WebExtensionContextProxy::DispatchStorageChangedEvent(jsonString, storageType, WebExtensionContentWorldType::ContentScript));

    wakeUpBackgroundContentIfNecessaryToFireEvents({ type }, [&] {
        sendToProcessesForEvent(type, Messages::WebExtensionContextProxy::DispatchStorageChangedEvent(jsonString, storageType, WebExtensionContentWorldType::Main));
    });
}

} // namespace WebKit

#endif // ENABLE(WK_WEB_EXTENSIONS)

