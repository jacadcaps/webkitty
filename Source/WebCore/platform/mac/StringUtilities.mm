/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#import "config.h"
#import "StringUtilities.h"

#import <JavaScriptCore/RegularExpression.h>
#import <wtf/text/StringBuilder.h>

namespace WebCore {
    
static String wildcardRegexPatternString(const String& string)
{
    String metaCharacters = ".|+?()[]{}^$"_s;
    UChar escapeCharacter = '\\';
    UChar wildcardCharacter = '*';
    
    StringBuilder stringBuilder;
    
    stringBuilder.append('^');
    for (unsigned i = 0; i < string.length(); i++) {
        auto character = string[i];
        if (metaCharacters.contains(character) || character == escapeCharacter)
            stringBuilder.append(escapeCharacter);
        else if (character == wildcardCharacter)
            stringBuilder.append('.');
        
        stringBuilder.append(character);
    }
    stringBuilder.append('$');
        
    return stringBuilder.toString();
}
    
bool stringMatchesWildcardString(const String& string, const String& wildcardString)
{
    return JSC::Yarr::RegularExpression(wildcardRegexPatternString(wildcardString), { JSC::Yarr::Flags::IgnoreCase }).match(string) != -1;
}

}
