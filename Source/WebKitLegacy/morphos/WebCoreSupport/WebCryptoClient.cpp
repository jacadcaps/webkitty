#include "WebKit.h"
#include "WebCryptoClient.h"
#import <WebCore/CryptoKey.h>
#import <WebCore/SerializedCryptoKeyWrap.h>
#import <WebCore/SerializedScriptValue.h>
#import <WebCore/WrappedCryptoKey.h>
#import <optional>
#import <wtf/TZoneMallocInlines.h>

namespace WebKit {

WTF_MAKE_TZONE_ALLOCATED_IMPL(WebCryptoClient);

std::optional<Vector<uint8_t>> WebCryptoClient::wrapCryptoKey(const Vector<uint8_t>& key) const
{
    auto masterKey = WebCore::defaultWebCryptoMasterKey();
    if (!masterKey)
        return std::nullopt;
    Vector<uint8_t> wrappedKey;
    if (!WebCore::wrapSerializedCryptoKey(WTFMove(*masterKey), key, wrappedKey))
        return std::nullopt;
    return wrappedKey;
}

std::optional<Vector<uint8_t>> WebCryptoClient::unwrapCryptoKey(const Vector<uint8_t>& serializedKey) const
{
    auto wrappedKey = WebCore::readSerializedCryptoKey(serializedKey);
    if (!wrappedKey)
        return std::nullopt;
    if (auto masterKey = WebCore::defaultWebCryptoMasterKey())
        return WebCore::unwrapCryptoKey(*masterKey, *wrappedKey);
    return std::nullopt;
}

std::optional<Vector<uint8_t>> WebCryptoClient::serializeAndWrapCryptoKey(WebCore::CryptoKeyData&& keyData) const
{
    auto key = WebCore::CryptoKey::create(WTFMove(keyData));
    if (!key)
        return std::nullopt;

    auto serializedKey = WebCore::SerializedScriptValue::serializeCryptoKey(*key);
    return wrapCryptoKey(serializedKey);
}

}
