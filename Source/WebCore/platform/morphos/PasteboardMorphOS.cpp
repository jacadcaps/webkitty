/*
 * Copyright (C) 2006-2018 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Pasteboard.h"

#if OS(MORPHOS)

#include "DragData.h"
#include "Image.h"
#include "MIMETypeRegistry.h"
#include "PasteboardStrategy.h"
#include "PlatformPasteboard.h"
#include "PlatformStrategies.h"
#include "SharedBuffer.h"
#include <wtf/RetainPtr.h>
#include <wtf/StdLibExtras.h>
#include <wtf/URL.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/unicode/CharacterNames.h>

namespace WebCore {

bool Pasteboard::hasData()
{
	return false;
}

bool Pasteboard::canSmartReplace()
{
	return false;
}

Vector<String> Pasteboard::typesSafeForBindings(const String& origin)
{
	return Vector<String>();
}

Vector<String> Pasteboard::typesForLegacyUnsafeBindings()
{
	return Vector<String>();
}

String Pasteboard::readOrigin()
{
	return String();
}

String Pasteboard::readString(const String& type)
{
	return String();
}

String Pasteboard::readStringInCustomData(const String& type)
{
	return String();
}

#if 0
Vector<String> Pasteboard::readAllStrings(const String& type)
{
	return Vector<String>();
}
#endif

void Pasteboard::writeString(const String& type, const String& data)
{
}

void Pasteboard::clear()
{

}

void Pasteboard::clear(const String& type)
{

}

void Pasteboard::read(PasteboardPlainText&)
{

}

void Pasteboard::read(PasteboardWebContentReader& , WebContentReadingPolicy )
{

}

void Pasteboard::read(PasteboardFileReader&)
{

}

void Pasteboard::write(const Color&)
{

}

void Pasteboard::write(const PasteboardURL&)
{

}

void Pasteboard::writeTrustworthyWebURLsPboardType(const PasteboardURL&)
{

}

void Pasteboard::write(const PasteboardImage&)
{

}

void Pasteboard::write(const PasteboardWebContent&)
{

}

void Pasteboard::writeCustomData(const PasteboardCustomData&)
{

}

Pasteboard::FileContentState Pasteboard::fileContentState()
{
	return FileContentState::NoFileOrImageData;
}

void Pasteboard::writeMarkup(const String& markup)
{

}

void Pasteboard::writePlainText(const String&, SmartReplaceOption)
{

}

std::unique_ptr<Pasteboard> Pasteboard::createForDragAndDrop()
{
	return nullptr;
}

std::unique_ptr<Pasteboard> Pasteboard::createForDragAndDrop(const DragData&)
{
	return nullptr;
}

void Pasteboard::setDragImage(DragImage, const IntPoint& hotSpot)
{

}

}
#endif
