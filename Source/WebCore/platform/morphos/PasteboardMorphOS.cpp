/*
 * Copyright (C) 2014-2015 Igalia S.L.
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

#include "config.h"
#include "Pasteboard.h"

#include "NotImplemented.h"
#include "PasteboardStrategy.h"

#include <proto/exec.h>
#include <proto/clipboard.h>

#include <libraries/charsets.h>
#include <libraries/clipboard.h>

namespace WebCore {

enum ClipboardDataType {
    ClipboardDataTypeText,
    ClipboardDataTypeMarkup,
    ClipboardDataTypeURIList,
    ClipboardDataTypeURL,
    ClipboardDataTypeImage,
    ClipboardDataTypeUnknown
};

std::unique_ptr<Pasteboard> Pasteboard::createForCopyAndPaste()
{
    return std::make_unique<Pasteboard>();
}

Pasteboard::Pasteboard()
{
}

bool Pasteboard::hasData()
{
     notImplemented();
    return false;
}

Vector<String> Pasteboard::typesSafeForBindings(const String&)
{
    notImplemented();
    return { };
}

Vector<String> Pasteboard::typesForLegacyUnsafeBindings()
{
    Vector<String> types;
//    platformStrategies()->pasteboardStrategy()->getTypes(types);
     notImplemented();
    return types;
}

String Pasteboard::readOrigin()
{
    notImplemented(); // webkit.org/b/177633: [GTK] Move to new Pasteboard API
    return { };
}

static ClipboardDataType selectionDataTypeFromHTMLClipboardType(const String& type)
{
    if (type == "text/plain")
        return ClipboardDataTypeText;
    if (type == "text/html")
        return ClipboardDataTypeMarkup;
    if (type == "Files" || type == "text/uri-list")
        return ClipboardDataTypeURIList;
    return ClipboardDataTypeUnknown;
}

String Pasteboard::readString(const String& type)
{
//    return platformStrategies()->pasteboardStrategy()->readStringFromPasteboard(0, type);
     notImplemented();
    return { };
}

String Pasteboard::readStringInCustomData(const String&)
{
    notImplemented();
    return { };
}

void Pasteboard::writeString(const String& type, const String& text)
{
	auto ctype = selectionDataTypeFromHTMLClipboardType(type);
	
	if (ctype == ClipboardDataTypeText || ctype == ClipboardDataTypeUnknown)
	{
		if ((ClipboardBase = OpenLibrary("clipboard.library", 51)))
        {
        	auto utext = text.utf8();
			struct TagItem tags[] = { {CLP_Charset, MIBENUM_UTF_8}, { TAG_DONE }};
			WriteClipTextA(utext.data(), tags);
		}
	}
}

void Pasteboard::clear()
{
}

void Pasteboard::clear(const String&)
{
}

void Pasteboard::read(PasteboardPlainText& text)
{
	const char *clipcontents;

	if ((ClipboardBase = OpenLibrary("clipboard.library", 51)))
	{
		struct TagItem tags[] = { {CLP_Charset, MIBENUM_UTF_8}, { TAG_DONE }};
		clipcontents = (const char *)ReadClipTextA(tags);
		if (nullptr != clipcontents)
		{
			text.text = String::fromUTF8(clipcontents);
			FreeClipText(clipcontents);
		}
		
		CloseLibrary(ClipboardBase);
	}
}

void Pasteboard::read(PasteboardWebContentReader&, WebContentReadingPolicy)
{
    notImplemented();
}

void Pasteboard::read(PasteboardFileReader&)
{
     notImplemented();
}

void Pasteboard::write(const PasteboardURL& url)
{
     notImplemented();
}

void Pasteboard::writeTrustworthyWebURLsPboardType(const PasteboardURL&)
{
    notImplemented();
}

void Pasteboard::write(const PasteboardImage&)
{
     notImplemented();
}

void Pasteboard::write(const PasteboardWebContent& content)
{
//    platformStrategies()->pasteboardStrategy()->writeToPasteboard(content);
     notImplemented();
}

Pasteboard::FileContentState Pasteboard::fileContentState()
{
    notImplemented();
    return FileContentState::NoFileOrImageData;
}

bool Pasteboard::canSmartReplace()
{
     notImplemented();
    return false;
}

void Pasteboard::writeMarkup(const String&)
{
     notImplemented();
}

void Pasteboard::writePlainText(const String& text, SmartReplaceOption)
{
    writeString("text/plain;charset=utf-8", text);
}

void Pasteboard::writeCustomData(const PasteboardCustomData&)
{
     notImplemented();
}

void Pasteboard::write(const Color&)
{
     notImplemented();
}

} // namespace WebCore
