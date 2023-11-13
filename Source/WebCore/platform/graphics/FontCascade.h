/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Holger Hans Peter Freyther
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "CharacterProperties.h"
#include "DashArray.h"
#include "Font.h"
#include "FontCascadeDescription.h"
#include "FontCascadeFonts.h"
#include "Path.h"
#include <optional>
#include <wtf/HashSet.h>
#include <wtf/WeakPtr.h>
#include <wtf/unicode/CharacterNames.h>

// "X11/X.h" defines Complex to 0 and conflicts
// with Complex value in CodePath enum.
#ifdef Complex
#undef Complex
#endif

namespace WebCore {

class GraphicsContext;
class LayoutRect;
class RenderText;
class TextLayout;
class TextRun;

namespace DisplayList {
class InMemoryDisplayList;
}
    
struct GlyphData;

struct GlyphOverflow {
    bool isEmpty() const
    {
        return !left && !right && !top && !bottom;
    }

    void extendTo(const GlyphOverflow& other)
    {
        left = std::max(left, other.left);
        right = std::max(right, other.right);
        top = std::max(top, other.top);
        bottom = std::max(bottom, other.bottom);
    }

    void extendTop(float extendTo)
    {
        top = std::max(top, LayoutUnit(ceilf(extendTo)));
    }

    void extendBottom(float extendTo)
    {
        bottom = std::max(bottom, LayoutUnit(ceilf(extendTo)));
    }

    bool operator!=(const GlyphOverflow& other)
    {
        // FIXME: Probably should name this rather than making it the != operator since it ignores the value of computeBounds.
        return left != other.left || right != other.right || top != other.top || bottom != other.bottom;
    }

    LayoutUnit left;
    LayoutUnit right;
    LayoutUnit top;
    LayoutUnit bottom;
    bool computeBounds { false };
};

#if USE(CORE_TEXT)
AffineTransform computeBaseOverallTextMatrix(const std::optional<AffineTransform>& syntheticOblique);
AffineTransform computeOverallTextMatrix(const Font&);
AffineTransform computeBaseVerticalTextMatrix(const AffineTransform& previousTextMatrix);
AffineTransform computeVerticalTextMatrix(const Font&, const AffineTransform& previousTextMatrix);
#endif

class TextLayoutDeleter {
public:
    void operator()(TextLayout*) const;
};

class FontCascade : public CanMakeWeakPtr<FontCascade> {
public:
    WEBCORE_EXPORT FontCascade();
    WEBCORE_EXPORT FontCascade(FontCascadeDescription&&, float letterSpacing = 0, float wordSpacing = 0);
    // This constructor is only used if the platform wants to start with a native font.
    WEBCORE_EXPORT FontCascade(const FontPlatformData&, FontSmoothingMode = FontSmoothingMode::AutoSmoothing);

    WEBCORE_EXPORT FontCascade(const FontCascade&);
    WEBCORE_EXPORT FontCascade& operator=(const FontCascade&);

    WEBCORE_EXPORT bool operator==(const FontCascade& other) const;

    const FontCascadeDescription& fontDescription() const { return m_fontDescription; }

    float size() const { return fontDescription().computedSize(); }

    bool isCurrent(const FontSelector&) const;
    void updateFonts(Ref<FontCascadeFonts>&&) const;
    WEBCORE_EXPORT void update(RefPtr<FontSelector>&& = nullptr) const;

    enum CustomFontNotReadyAction { DoNotPaintIfFontNotReady, UseFallbackIfFontNotReady };
    WEBCORE_EXPORT FloatSize drawText(GraphicsContext&, const TextRun&, const FloatPoint&, unsigned from = 0, std::optional<unsigned> to = std::nullopt, CustomFontNotReadyAction = DoNotPaintIfFontNotReady) const;
    static void drawGlyphs(GraphicsContext&, const Font&, const GlyphBufferGlyph*, const GlyphBufferAdvance*, unsigned numGlyphs, const FloatPoint&, FontSmoothingMode);
    void drawEmphasisMarks(GraphicsContext&, const TextRun&, const AtomString& mark, const FloatPoint&, unsigned from = 0, std::optional<unsigned> to = std::nullopt) const;

    DashArray dashesForIntersectionsWithRect(const TextRun&, const FloatPoint& textOrigin, const FloatRect& lineExtents) const;

    float widthOfTextRange(const TextRun&, unsigned from, unsigned to, HashSet<const Font*>* fallbackFonts = 0, float* outWidthBeforeRange = nullptr, float* outWidthAfterRange = nullptr) const;
    WEBCORE_EXPORT float width(const TextRun&, HashSet<const Font*>* fallbackFonts = 0, GlyphOverflow* = 0) const;
    float widthForSimpleText(StringView text, TextDirection = TextDirection::LTR) const;

    std::unique_ptr<TextLayout, TextLayoutDeleter> createLayout(RenderText&, float xPos, bool collapseWhiteSpace) const;
    static float width(TextLayout&, unsigned from, unsigned len, HashSet<const Font*>* fallbackFonts = 0);
    float widthOfSpaceString() const
    {
        return width(TextRun { String { &space, 1 } });
    }

    int offsetForPosition(const TextRun&, float position, bool includePartialGlyphs) const;
    void adjustSelectionRectForText(const TextRun&, LayoutRect& selectionRect, unsigned from = 0, std::optional<unsigned> to = std::nullopt) const;

    Vector<LayoutRect> characterSelectionRectsForText(const TextRun&, const LayoutRect& selectionRect, unsigned from, std::optional<unsigned> to) const;

    bool isSmallCaps() const { return m_fontDescription.variantCaps() == FontVariantCaps::Small; }

    float wordSpacing() const { return m_wordSpacing; }
    float letterSpacing() const { return m_letterSpacing; }
    void setWordSpacing(float s) { m_wordSpacing = s; }
    void setLetterSpacing(float s) { m_letterSpacing = s; }
    bool isFixedPitch() const;
    
    bool enableKerning() const { return m_enableKerning; }
    bool requiresShaping() const { return m_requiresShaping; }

    const AtomString& firstFamily() const { return m_fontDescription.firstFamily(); }
    unsigned familyCount() const { return m_fontDescription.familyCount(); }
    const AtomString& familyAt(unsigned i) const { return m_fontDescription.familyAt(i); }

    // A std::nullopt return value indicates "font-style: normal".
    std::optional<FontSelectionValue> italic() const { return m_fontDescription.italic(); }
    FontSelectionValue weight() const { return m_fontDescription.weight(); }
    FontWidthVariant widthVariant() const { return m_fontDescription.widthVariant(); }

    bool isPlatformFont() const { return m_fonts->isForPlatformFont(); }

    const FontMetrics& metricsOfPrimaryFont() const { return primaryFont().fontMetrics(); }
    float tabWidth(const Font&, const TabSize&, float, Font::SyntheticBoldInclusion) const;
    bool hasValidAverageCharWidth() const;
    bool fastAverageCharWidthIfAvailable(float &width) const; // returns true on success

    int emphasisMarkAscent(const AtomString&) const;
    int emphasisMarkDescent(const AtomString&) const;
    int emphasisMarkHeight(const AtomString&) const;
    float floatEmphasisMarkHeight(const AtomString&) const;

    const Font& primaryFont() const;
    const FontRanges& fallbackRangesAt(unsigned) const;
    GlyphData glyphDataForCharacter(UChar32, bool mirror, FontVariant = AutoVariant) const;

    const Font* fontForCombiningCharacterSequence(const UChar*, size_t length) const;

    static bool isCJKIdeograph(UChar32);
    static bool isCJKIdeographOrSymbol(UChar32);

    // Returns (the number of opportunities, whether the last expansion is a trailing expansion)
    // If there are no opportunities, the bool will be true iff we are forbidding leading expansions.
    static std::pair<unsigned, bool> expansionOpportunityCount(StringView, TextDirection, ExpansionBehavior);

    // Whether or not there is an expansion opportunity just before the first character
    // Note that this does not take a isAfterExpansion flag; this assumes that isAfterExpansion is false
    static bool leftExpansionOpportunity(StringView, TextDirection);
    static bool rightExpansionOpportunity(StringView, TextDirection);

    WEBCORE_EXPORT static void setDisableFontSubpixelAntialiasingForTesting(bool);
    WEBCORE_EXPORT static bool shouldDisableFontSubpixelAntialiasingForTesting();

    enum class CodePath : uint8_t { Auto, Simple, Complex, SimpleWithGlyphOverflow };
    CodePath codePath(const TextRun&, std::optional<unsigned> from = std::nullopt, std::optional<unsigned> to = std::nullopt) const;
    static CodePath characterRangeCodePath(const LChar*, unsigned) { return CodePath::Simple; }
    static CodePath characterRangeCodePath(const UChar*, unsigned len);

    bool primaryFontIsSystemFont() const;

    static float syntheticObliqueAngle() { return 14; }

    std::unique_ptr<DisplayList::InMemoryDisplayList> displayListForTextRun(GraphicsContext&, const TextRun&, unsigned from = 0, std::optional<unsigned> to = { }, CustomFontNotReadyAction = CustomFontNotReadyAction::DoNotPaintIfFontNotReady) const;

    unsigned generation() const { return m_generation; }

private:
    enum ForTextEmphasisOrNot { NotForTextEmphasis, ForTextEmphasis };

    GlyphBuffer layoutText(CodePath, const TextRun&, unsigned from, unsigned to, ForTextEmphasisOrNot = NotForTextEmphasis) const;
    GlyphBuffer layoutSimpleText(const TextRun&, unsigned from, unsigned to, ForTextEmphasisOrNot = NotForTextEmphasis) const;
    void drawGlyphBuffer(GraphicsContext&, const GlyphBuffer&, FloatPoint&, CustomFontNotReadyAction) const;
    void drawEmphasisMarks(GraphicsContext&, const GlyphBuffer&, const AtomString&, const FloatPoint&) const;
    float floatWidthForSimpleText(const TextRun&, HashSet<const Font*>* fallbackFonts = 0, GlyphOverflow* = 0) const;
    int offsetForPositionForSimpleText(const TextRun&, float position, bool includePartialGlyphs) const;
    void adjustSelectionRectForSimpleText(const TextRun&, LayoutRect& selectionRect, unsigned from, unsigned to) const;

    std::optional<GlyphData> getEmphasisMarkGlyphData(const AtomString&) const;
    const Font* fontForEmphasisMark(const AtomString&) const;

    static bool canReturnFallbackFontsForComplexText();
    static bool canExpandAroundIdeographsInComplexText();

    GlyphBuffer layoutComplexText(const TextRun&, unsigned from, unsigned to, ForTextEmphasisOrNot = NotForTextEmphasis) const;
    float floatWidthForComplexText(const TextRun&, HashSet<const Font*>* fallbackFonts = 0, GlyphOverflow* = 0) const;
    int offsetForPositionForComplexText(const TextRun&, float position, bool includePartialGlyphs) const;
    void adjustSelectionRectForComplexText(const TextRun&, LayoutRect& selectionRect, unsigned from, unsigned to) const;

    static std::pair<unsigned, bool> expansionOpportunityCountInternal(const LChar*, unsigned length, TextDirection, ExpansionBehavior);
    static std::pair<unsigned, bool> expansionOpportunityCountInternal(const UChar*, unsigned length, TextDirection, ExpansionBehavior);

    friend struct WidthIterator;
    friend class ComplexTextController;

public:
#if ENABLE(TEXT_AUTOSIZING)
    bool equalForTextAutoSizing(const FontCascade& other) const
    {
        return m_fontDescription.equalForTextAutoSizing(other.m_fontDescription)
            && m_letterSpacing == other.m_letterSpacing
            && m_wordSpacing == other.m_wordSpacing;
    }
#endif

    // Useful for debugging the different font rendering code paths.
    WEBCORE_EXPORT static void setCodePath(CodePath);
    static CodePath codePath();
    static CodePath s_codePath;

    FontSelector* fontSelector() const;
    static bool treatAsSpace(UChar32 c) { return c == space || c == tabCharacter || c == newlineCharacter || c == noBreakSpace; }
    static bool isCharacterWhoseGlyphsShouldBeDeletedForTextRendering(UChar32 character)
    {
        // https://drafts.csswg.org/css-text-3/#white-space-processing
        // "Unsupported Default_ignorable characters must be ignored for text rendering."
        return isControlCharacter(character) || isDefaultIgnorableCodePoint(character);
    }
    // FIXME: Callers of treatAsZeroWidthSpace() and treatAsZeroWidthSpaceInComplexScript() should probably be calling isCharacterWhoseGlyphsShouldBeDeletedForTextRendering() instead.
    static bool treatAsZeroWidthSpace(UChar32 c) { return treatAsZeroWidthSpaceInComplexScript(c) || c == zeroWidthNonJoiner || c == zeroWidthJoiner; }
    static bool treatAsZeroWidthSpaceInComplexScript(UChar32 c) { return c < space || (c >= deleteCharacter && c < noBreakSpace) || c == softHyphen || c == zeroWidthSpace || (c >= leftToRightMark && c <= rightToLeftMark) || (c >= leftToRightEmbed && c <= rightToLeftOverride) || c == zeroWidthNoBreakSpace; }
    static bool canReceiveTextEmphasis(UChar32);

    static inline UChar normalizeSpaces(UChar character)
    {
        if (treatAsSpace(character))
            return space;

        if (treatAsZeroWidthSpace(character))
            return zeroWidthSpace;

        return character;
    }

    static String normalizeSpaces(const LChar*, unsigned length);
    static String normalizeSpaces(const UChar*, unsigned length);

    bool useBackslashAsYenSymbol() const { return m_useBackslashAsYenSymbol; }
    FontCascadeFonts* fonts() const { return m_fonts.get(); }
    bool isLoadingCustomFonts() const;

    static ResolvedEmojiPolicy resolveEmojiPolicy(FontVariantEmoji, UChar32);

private:

    bool advancedTextRenderingMode() const
    {
        return m_fontDescription.textRenderingMode() != TextRenderingMode::OptimizeSpeed;
    }

    bool computeEnableKerning() const
    {
        auto kerning = m_fontDescription.kerning();
        if (kerning == Kerning::Normal)
            return true;
        if (kerning == Kerning::NoShift)
            return false;
        return advancedTextRenderingMode();
    }

    bool computeRequiresShaping() const
    {
        if (!m_fontDescription.variantSettings().isAllNormal())
            return true;
        if (m_fontDescription.featureSettings().size())
            return true;
        return advancedTextRenderingMode();
    }

    FontCascadeDescription m_fontDescription;
    mutable RefPtr<FontCascadeFonts> m_fonts;
    float m_letterSpacing { 0 };
    float m_wordSpacing { 0 };
    mutable unsigned m_generation { 0 };
    bool m_useBackslashAsYenSymbol { false };
    bool m_enableKerning { false }; // Computed from m_fontDescription.
    bool m_requiresShaping { false }; // Computed from m_fontDescription.
};

inline const Font& FontCascade::primaryFont() const
{
    ASSERT(m_fonts);
    return m_fonts->primaryFont(m_fontDescription);
}

inline const FontRanges& FontCascade::fallbackRangesAt(unsigned index) const
{
    ASSERT(m_fonts);
    return m_fonts->realizeFallbackRangesAt(m_fontDescription, index);
}

inline bool FontCascade::isFixedPitch() const
{
    ASSERT(m_fonts);
    return m_fonts->isFixedPitch(m_fontDescription);
}

inline FontSelector* FontCascade::fontSelector() const
{
    return m_fonts ? m_fonts->fontSelector() : nullptr;
}

inline float FontCascade::tabWidth(const Font& font, const TabSize& tabSize, float position, Font::SyntheticBoldInclusion syntheticBoldInclusion) const
{
    float baseTabWidth = tabSize.widthInPixels(font.spaceWidth());
    float result = 0;
    if (!baseTabWidth)
        result = letterSpacing();
    else {
        result = baseTabWidth - fmodf(position, baseTabWidth);
        if (result < font.spaceWidth() / 2)
            result += baseTabWidth;
    }
    // If our caller passes in SyntheticBoldInclusion::Exclude, that means they're going to apply synthetic bold themselves later.
    // However, regardless of that, the space characters that are fed into the width calculation need to have their correct width, including the synthetic bold.
    // So, we've already got synthetic bold applied, so if we're supposed to exclude it, we need to subtract it out here.
    return result - (syntheticBoldInclusion == Font::SyntheticBoldInclusion::Exclude ? font.syntheticBoldOffset() : 0);
}

} // namespace WebCore
