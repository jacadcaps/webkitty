/*
 * Copyright (C) 2006-2017 Apple Inc.  All rights reserved.
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

#pragma once

#include "WebKit.h"
#include <WebCore/DOMPasteAccess.h>
#include <WebCore/EditorClient.h>
#include <WebCore/TextCheckerClient.h>
#include <WebCore/UndoStep.h>
#include <wtf/HashSet.h>

namespace WebCore {
class Element;
}

namespace WebKit {

class WebPage;
class WebNotification;
struct WebEditorClientCleanup;

class WebEditorUndoStep : public WTF::RefCounted<WebEditorUndoStep>
{
public:
    static Ref<WebEditorUndoStep> create(WebCore::UndoStep& step)
    {
        return adoptRef(*new WebEditorUndoStep(step));
    }
	
	WebEditorUndoStep(WebCore::UndoStep& step)
		: m_undoStep(step)
	{
	}
	
	void unapply()
	{
		m_undoStep->unapply();
	}
	
	void reapply()
	{
		m_undoStep->reapply();
	}

private:
	Ref<WebCore::UndoStep> m_undoStep;
};

class WebEditorClient final : public WebCore::EditorClient, public WebCore::TextCheckerClient {
	WTF_MAKE_FAST_ALLOCATED;
public:
	friend WebEditorClientCleanup;

    WebEditorClient(WebPage*);
    ~WebEditorClient();

	WebPage *webPage() { return m_webPage; }
	
	static void setSpellCheckingEnabled(bool enabled);
	static bool getSpellCheckingEnabled() { return m_globalSpellDictionary != nullptr; }
	static void setSpellCheckingLanguage(const WTF::String &language);
	static WTF::String getSpellCheckingLanguage() { return m_globalLanguage; }
	static void getAvailableDictionaries(WTF::Vector<WTF::String> &outDictionaries, WTF::String &outDefault);

	void setSpellCheckingLanguages(const WTF::String &language, const WTF::String &languageAdditional);
	WTF::String primarySpellCheckingLanguage() { return m_language; }
	const WTF::String &additionalSpellCheckingLanguage() const { return m_additionalLanguage; }
	void onSpellcheckingLanguageChanged();
	void getGuessesForWord(const WTF::String &word, WTF::Vector<WTF::String> &outGuesses);

    void ignoreWordInSpellDocument(const WTF::String&) final;
    void learnWord(const WTF::String&) final;
    void replaceMisspelledWord(const WTF::String&);

private:
    bool isContinuousSpellCheckingEnabled() final;
    void toggleGrammarChecking() final;
    bool isGrammarCheckingEnabled() final;
    void toggleContinuousSpellChecking() final;
    int spellCheckerDocumentTag() final;

    bool shouldBeginEditing(const WebCore::SimpleRange&) final;
    bool shouldEndEditing(const WebCore::SimpleRange&) final;
    bool shouldInsertNode(WebCore::Node&, const std::optional<WebCore::SimpleRange>&, WebCore::EditorInsertAction) final;
    bool shouldInsertText(const String&, const std::optional<WebCore::SimpleRange>&, WebCore::EditorInsertAction) final;
    bool shouldChangeSelectedRange(const std::optional<WebCore::SimpleRange>& fromRange, const std::optional<WebCore::SimpleRange>& toRange, WebCore::Affinity, bool stillSelecting) final;

    void didBeginEditing() final;
    void didEndEditing() final;
    void willWriteSelectionToPasteboard(const std::optional<WebCore::SimpleRange>&) final;
    void didWriteSelectionToPasteboard() final;
    void getClientPasteboardData(const std::optional<WebCore::SimpleRange>&, Vector<String>& pasteboardTypes, Vector<RefPtr<WebCore::SharedBuffer>>& pasteboardData) final;

    void didEndUserTriggeredSelectionChanges() final { }
    void respondToChangedContents() final;
    void respondToChangedSelection(WebCore::Frame*) final;
    void updateEditorStateAfterLayoutIfEditabilityChanged() final { } 
    void canceledComposition() final;
    void discardedComposition(WebCore::Frame*) final;
    void didUpdateComposition() final { }

    bool shouldDeleteRange(const std::optional<WebCore::SimpleRange>&) final;

    bool shouldApplyStyle(const WebCore::StyleProperties &, const std::optional<WebCore::SimpleRange>&) final;
    void didApplyStyle() final;
    bool shouldMoveRangeAfterDelete(const WebCore::SimpleRange&, const WebCore::SimpleRange&) final;

    bool smartInsertDeleteEnabled() final;
    bool isSelectTrailingWhitespaceEnabled() const final;

    void registerUndoStep(WebCore::UndoStep&) final;
    void registerRedoStep(WebCore::UndoStep&) final;
    void clearUndoRedoOperations();

    bool canCopyCut(WebCore::Frame*, bool defaultValue) const final;
    bool canPaste(WebCore::Frame*, bool defaultValue) const final;
    bool canUndo() const final;
    bool canRedo() const final;
    
    void undo() final;
    void redo() final;

	bool supportsGlobalSelection() final { return true; }

    void textFieldDidBeginEditing(WebCore::Element&) final;
    void textFieldDidEndEditing(WebCore::Element&) final;
    void textDidChangeInTextField(WebCore::Element&) final;
    bool doTextFieldCommandFromEvent(WebCore::Element&, WebCore::KeyboardEvent*) final;
    void textWillBeDeletedInTextField(WebCore::Element& input) final;
    void textDidChangeInTextArea(WebCore::Element&) final;
    void overflowScrollPositionChanged() final { }
    void subFrameScrollPositionChanged() final { }

    void handleKeyboardEvent(WebCore::KeyboardEvent&) final;
    void handleInputMethodKeydown(WebCore::KeyboardEvent&) final;

    bool shouldEraseMarkersAfterChangeSelection(WebCore::TextCheckingType) const final;
    void checkSpellingOfString(StringView, int* misspellingLocation, int* misspellingLength) final;
    void checkGrammarOfString(StringView, Vector<WebCore::GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength) final;
    void updateSpellingUIWithGrammarString(const WTF::String&, const WebCore::GrammarDetail&) final;
    void updateSpellingUIWithMisspelledWord(const WTF::String&) final;
    void showSpellingUI(bool show) final;
    bool spellingUIIsShowing() final;
    void getGuessesForWord(const WTF::String& word, const WTF::String& context, const WebCore::VisibleSelection& currentSelection, WTF::Vector<WTF::String>& guesses) final;

    void willSetInputMethodState() final;
    void setInputMethodState(WebCore::Element*) final;
    void requestCheckingOfString(WebCore::TextCheckingRequest&, const WebCore::VisibleSelection&) final { }
    bool performTwoStepDrop(WebCore::DocumentFragment&, const WebCore::SimpleRange& , bool ) final { return false; }

    WebCore::DOMPasteAccessResponse requestDOMPasteAccess(WebCore::DOMPasteAccessCategory, const String&) final { return WebCore::DOMPasteAccessResponse::DeniedForGesture; }

    WebCore::TextCheckerClient* textChecker() final { return this; }

//    bool canShowFontPanel() const final { return false; }

protected:
    WebPage* m_webPage;
    WTF::HashSet<WTF::String> m_ignoredWords;
    WTF::Vector<WTF::RefPtr<WebEditorUndoStep>> m_undo;
    WTF::Vector<WTF::RefPtr<WebEditorUndoStep>> m_redo;
    WTF::String m_language;
    WTF::String m_additionalLanguage;
    APTR m_spellDictionary = nullptr;
    APTR m_additionalSpellDictionary = nullptr;
	
    static struct Library *m_spellcheckerLibrary;
    static APTR m_globalSpellDictionary;
    static WTF::String m_globalLanguage;
    static WTF::HashSet<WebEditorClient*> m_editors;
};

}
