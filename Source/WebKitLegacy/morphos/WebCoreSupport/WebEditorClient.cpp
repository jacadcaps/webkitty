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

#include "WebEditorClient.h"
#include "WebView.h"
#include <WebCore/Document.h>
#include <WebCore/HTMLElement.h>
#include <WebCore/HTMLInputElement.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/KeyboardEvent.h>
#include <WebCore/LocalizedStrings.h>
#include <WebCore/NotImplemented.h>
#include <WebCore/Page.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include <WebCore/Range.h>
#include <WebCore/Settings.h>
#include <WebCore/UndoStep.h>
#include <WebCore/UserTypingGestureIndicator.h>
#include <WebCore/VisibleSelection.h>
#include <wtf/text/StringView.h>

using namespace WebCore;
using namespace HTMLNames;

// WebEditorClient ------------------------------------------------------------------

WebEditorClient::WebEditorClient(WebView* webView)
    : m_webView(webView)
    , m_undoTarget(0)
{
//    m_undoTarget = new WebEditorUndoTarget();
}

WebEditorClient::~WebEditorClient()
{
}

bool WebEditorClient::isContinuousSpellCheckingEnabled()
{
    notImplemented();
	return false;
}

void WebEditorClient::toggleContinuousSpellChecking()
{
    notImplemented();
}

bool WebEditorClient::isGrammarCheckingEnabled()
{
    notImplemented();
    return false;
}

void WebEditorClient::toggleGrammarChecking()
{
    notImplemented();
}

int WebEditorClient::spellCheckerDocumentTag()
{
    // we don't use the concept of spelling tags
    notImplemented();
    ASSERT_NOT_REACHED();
    return 0;
}

bool WebEditorClient::shouldBeginEditing(WebCore::Range* range)
{
    notImplemented();
    return true;
}

bool WebEditorClient::shouldEndEditing(Range* range)
{
    notImplemented();
    return true;
}

void WebEditorClient::didBeginEditing()
{
    notImplemented();
}

void WebEditorClient::respondToChangedContents()
{
    notImplemented();
}

void WebEditorClient::respondToChangedSelection(Frame*)
{
//    m_webView->selectionChanged();
    notImplemented();
}

void WebEditorClient::discardedComposition(Frame*)
{
    notImplemented();
}

void WebEditorClient::canceledComposition()
{
    notImplemented();
}

void WebEditorClient::didEndEditing()
{
    notImplemented();
}

void WebEditorClient::didWriteSelectionToPasteboard()
{
    notImplemented();
}

void WebEditorClient::willWriteSelectionToPasteboard(WebCore::Range*)
{
    notImplemented();
}

void WebEditorClient::getClientPasteboardDataForRange(WebCore::Range*, Vector<String>&, Vector<RefPtr<WebCore::SharedBuffer> >&)
{
    notImplemented();
}

bool WebEditorClient::shouldDeleteRange(WebCore::Range* range)
{
    notImplemented();
    return true;
}

bool WebEditorClient::shouldInsertNode(WebCore::Node* node, WebCore::Range* insertingRange, WebCore::EditorInsertAction givenAction)
{ 
    notImplemented();
    return true;
}

bool WebEditorClient::shouldInsertText(const String& str, WebCore::Range* insertingRange, WebCore::EditorInsertAction givenAction)
{
    notImplemented();
    return true;
}

bool WebEditorClient::shouldChangeSelectedRange(WebCore::Range* currentRange, WebCore::Range* proposedRange, WebCore::EAffinity selectionAffinity, bool flag)
{
    notImplemented();
    return true;
}

bool WebEditorClient::shouldApplyStyle(StyleProperties*, Range*)
{
    notImplemented();
    return true;
}

void WebEditorClient::didApplyStyle()
{
    notImplemented();
}

bool WebEditorClient::shouldMoveRangeAfterDelete(Range*, Range*)
{
    notImplemented();
    return true;
}

bool WebEditorClient::smartInsertDeleteEnabled(void)
{
    Page* page = m_webView->page();
    if (!page)
        return false;
    return page->settings().smartInsertDeleteEnabled();
}

bool WebEditorClient::isSelectTrailingWhitespaceEnabled(void) const
{
    Page* page = m_webView->page();
    if (!page)
        return false;
    return page->settings().selectTrailingWhitespaceEnabled();
}

void WebEditorClient::textFieldDidBeginEditing(Element* e)
{
    notImplemented();
}

void WebEditorClient::textFieldDidEndEditing(Element* e)
{
    notImplemented();
}

void WebEditorClient::textDidChangeInTextField(Element* e)
{
    if (!UserTypingGestureIndicator::processingUserTypingGesture() || UserTypingGestureIndicator::focusedElementAtGestureStart() != e)
        return;
    notImplemented();
}

bool WebEditorClient::doTextFieldCommandFromEvent(Element* e, KeyboardEvent* ke)
{
    bool result = false;
    notImplemented();
    return !!result;
}

void WebEditorClient::textWillBeDeletedInTextField(Element* e)
{
    notImplemented();
}

void WebEditorClient::textDidChangeInTextArea(Element* e)
{
    notImplemented();
}

#if 0
class WebEditorUndoCommand final : public IWebUndoCommand
{
public:
    WebEditorUndoCommand(UndoStep&, bool isUndo);
    void execute();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

private:
    ULONG m_refCount;
    Ref<UndoStep> m_step;
    bool m_isUndo;
};

WebEditorUndoCommand::WebEditorUndoCommand(UndoStep& step, bool isUndo)
    : m_refCount(1)
    , m_step(step)
    , m_isUndo(isUndo)
{ 
}

void WebEditorUndoCommand::execute()
{
    if (m_isUndo)
        m_step->unapply();
    else
        m_step->reapply();
}

HRESULT WebEditorUndoCommand::QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;
    *ppvObject = nullptr;
    if (IsEqualGUID(riid, IID_IUnknown))
        *ppvObject = static_cast<IWebUndoCommand*>(this);
    else if (IsEqualGUID(riid, IID_IWebUndoCommand))
        *ppvObject = static_cast<IWebUndoCommand*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG WebEditorUndoCommand::AddRef()
{
    return ++m_refCount;
}

ULONG WebEditorUndoCommand::Release()
{
    ULONG newRef = --m_refCount;
    if (!newRef)
        delete(this);

    return newRef;
}
#endif

static String undoNameForEditAction(EditAction editAction)
{
    switch (editAction) {
    case EditAction::Unspecified:
    case EditAction::InsertReplacement:
        return String();
    case EditAction::SetColor: return WEB_UI_STRING_KEY("Set Color", "Set Color (Undo action name)", "Undo action name");
    case EditAction::SetBackgroundColor: return WEB_UI_STRING_KEY("Set Background Color", "Set Background Color (Undo action name)", "Undo action name");
    case EditAction::TurnOffKerning: return WEB_UI_STRING_KEY("Turn Off Kerning", "Turn Off Kerning (Undo action name)", "Undo action name");
    case EditAction::TightenKerning: return WEB_UI_STRING_KEY("Tighten Kerning", "Tighten Kerning (Undo action name)", "Undo action name");
    case EditAction::LoosenKerning: return WEB_UI_STRING_KEY("Loosen Kerning", "Loosen Kerning (Undo action name)", "Undo action name");
    case EditAction::UseStandardKerning: return WEB_UI_STRING_KEY("Use Standard Kerning", "Use Standard Kerning (Undo action name)", "Undo action name");
    case EditAction::TurnOffLigatures: return WEB_UI_STRING_KEY("Turn Off Ligatures", "Turn Off Ligatures (Undo action name)", "Undo action name");
    case EditAction::UseStandardLigatures: return WEB_UI_STRING_KEY("Use Standard Ligatures", "Use Standard Ligatures (Undo action name)", "Undo action name");
    case EditAction::UseAllLigatures: return WEB_UI_STRING_KEY("Use All Ligatures", "Use All Ligatures (Undo action name)", "Undo action name");
    case EditAction::RaiseBaseline: return WEB_UI_STRING_KEY("Raise Baseline", "Raise Baseline (Undo action name)", "Undo action name");
    case EditAction::LowerBaseline: return WEB_UI_STRING_KEY("Lower Baseline", "Lower Baseline (Undo action name)", "Undo action name");
    case EditAction::SetTraditionalCharacterShape: return WEB_UI_STRING_KEY("Set Traditional Character Shape", "Set Traditional Character Shape (Undo action name)", "Undo action name");
    case EditAction::SetFont: return WEB_UI_STRING_KEY("Set Font", "Set Font (Undo action name)", "Undo action name");
    case EditAction::ChangeAttributes: return WEB_UI_STRING_KEY("Change Attributes", "Change Attributes (Undo action name)", "Undo action name");
    case EditAction::AlignLeft: return WEB_UI_STRING_KEY("Align Left", "Align Left (Undo action name)", "Undo action name");
    case EditAction::AlignRight: return WEB_UI_STRING_KEY("Align Right", "Align Right (Undo action name)", "Undo action name");
    case EditAction::Center: return WEB_UI_STRING_KEY("Center", "Center (Undo action name)", "Undo action name");
    case EditAction::Justify: return WEB_UI_STRING_KEY("Justify", "Justify (Undo action name)", "Undo action name");
    case EditAction::SetInlineWritingDirection:
    case EditAction::SetBlockWritingDirection:
        return WEB_UI_STRING_KEY("Set Writing Direction", "Set Writing Direction (Undo action name)", "Undo action name");
    case EditAction::Subscript: return WEB_UI_STRING_KEY("Subscript", "Subscript (Undo action name)", "Undo action name");
    case EditAction::Superscript: return WEB_UI_STRING_KEY("Superscript", "Superscript (Undo action name)", "Undo action name");
    case EditAction::Bold: return WEB_UI_STRING_KEY("Bold", "Bold (Undo action name)", "Undo action name");
    case EditAction::Italics: return WEB_UI_STRING_KEY("Italics", "Italics (Undo action name)", "Undo action name");
    case EditAction::Underline: return WEB_UI_STRING_KEY("Underline", "Underline (Undo action name)", "Undo action name");
    case EditAction::Outline: return WEB_UI_STRING_KEY("Outline", "Outline (Undo action name)", "Undo action name");
    case EditAction::Unscript: return WEB_UI_STRING_KEY("Unscript", "Unscript (Undo action name)", "Undo action name");
    case EditAction::DeleteByDrag: return WEB_UI_STRING_KEY("Drag", "Drag (Undo action name)", "Undo action name");
    case EditAction::Cut: return WEB_UI_STRING_KEY("Cut", "Cut (Undo action name)", "Undo action name");
    case EditAction::Paste: return WEB_UI_STRING_KEY("Paste", "Paste (Undo action name)", "Undo action name");
    case EditAction::PasteFont: return WEB_UI_STRING_KEY("Paste Font", "Paste Font (Undo action name)", "Undo action name");
    case EditAction::PasteRuler: return WEB_UI_STRING_KEY("Paste Ruler", "Paste Ruler (Undo action name)", "Undo action name");
    case EditAction::TypingDeleteSelection:
    case EditAction::TypingDeleteBackward:
    case EditAction::TypingDeleteForward:
    case EditAction::TypingDeleteWordBackward:
    case EditAction::TypingDeleteWordForward:
    case EditAction::TypingDeleteLineBackward:
    case EditAction::TypingDeleteLineForward:
    case EditAction::TypingInsertText:
    case EditAction::TypingInsertLineBreak:
    case EditAction::TypingInsertParagraph:
        return WEB_UI_STRING_KEY("Typing", "Typing (Undo action name)", "Undo action name");
    case EditAction::CreateLink: return WEB_UI_STRING_KEY("Create Link", "Create Link (Undo action name)", "Undo action name");
    case EditAction::Unlink: return WEB_UI_STRING_KEY("Unlink", "Unlink (Undo action name)", "Undo action name");
    case EditAction::InsertUnorderedList:
    case EditAction::InsertOrderedList:
        return WEB_UI_STRING_KEY("Insert List", "Insert List (Undo action name)", "Undo action name");
    case EditAction::FormatBlock: return WEB_UI_STRING_KEY("Formatting", "Format Block (Undo action name)", "Undo action name");
    case EditAction::Indent: return WEB_UI_STRING_KEY("Indent", "Indent (Undo action name)", "Undo action name");
    case EditAction::Outdent: return WEB_UI_STRING_KEY("Outdent", "Outdent (Undo action name)", "Undo action name");
    default: return String();
    }
}

void WebEditorClient::registerUndoStep(UndoStep& step)
{
	notImplemented();
#if 0
    IWebUIDelegate* uiDelegate = 0;
    if (SUCCEEDED(m_webView->uiDelegate(&uiDelegate))) {
        String actionName = undoNameForEditAction(step.editingAction());
        WebEditorUndoCommand* undoCommand = new WebEditorUndoCommand(step, true);
        if (!undoCommand)
            return;
        uiDelegate->registerUndoWithTarget(m_undoTarget, 0, undoCommand);
        undoCommand->Release(); // the undo manager owns the reference
        if (!actionName.isEmpty())
            uiDelegate->setActionTitle(BString(actionName));
        uiDelegate->Release();
    }
#endif
}

void WebEditorClient::registerRedoStep(UndoStep& step)
{
	notImplemented();
#if 0
    IWebUIDelegate* uiDelegate = 0;
    if (SUCCEEDED(m_webView->uiDelegate(&uiDelegate))) {
        WebEditorUndoCommand* undoCommand = new WebEditorUndoCommand(step, false);
        if (!undoCommand)
            return;
        uiDelegate->registerUndoWithTarget(m_undoTarget, 0, undoCommand);
        undoCommand->Release(); // the undo manager owns the reference
        uiDelegate->Release();
    }
#endif
}

void WebEditorClient::clearUndoRedoOperations()
{
	notImplemented();
}

bool WebEditorClient::canCopyCut(Frame*, bool defaultValue) const
{
    return defaultValue;
}

bool WebEditorClient::canPaste(Frame*, bool defaultValue) const
{
    return defaultValue;
}

bool WebEditorClient::canUndo() const
{
	notImplemented();
	return false;
}

bool WebEditorClient::canRedo() const
{
	notImplemented();
	return false;
}

void WebEditorClient::undo()
{
	notImplemented();
}

void WebEditorClient::redo()
{
	notImplemented();
}

void WebEditorClient::handleKeyboardEvent(KeyboardEvent& event)
{
	notImplemented();
//    if (m_webView->handleEditingKeyboardEvent(event))
//        event.setDefaultHandled();
}

void WebEditorClient::handleInputMethodKeydown(KeyboardEvent&)
{
}

bool WebEditorClient::shouldEraseMarkersAfterChangeSelection(TextCheckingType) const
{
    return true;
}

void WebEditorClient::ignoreWordInSpellDocument(const String& word)
{
	notImplemented();
}

void WebEditorClient::learnWord(const String& word)
{
	notImplemented();
}

void WebEditorClient::checkSpellingOfString(StringView text, int* misspellingLocation, int* misspellingLength)
{
    *misspellingLocation = -1;
    *misspellingLength = 0;

	notImplemented();
}

String WebEditorClient::getAutoCorrectSuggestionForMisspelledWord(const String& inputWord)
{
    // This method can be implemented using customized algorithms for the particular browser.
    // Currently, it computes an empty string.
    return String();
}

void WebEditorClient::checkGrammarOfString(StringView text, Vector<GrammarDetail>& details, int* badGrammarLocation, int* badGrammarLength)
{
	notImplemented();
}

void WebEditorClient::updateSpellingUIWithGrammarString(const String& string, const WebCore::GrammarDetail& detail)
{
	notImplemented();
}

void WebEditorClient::updateSpellingUIWithMisspelledWord(const String& word)
{
	notImplemented();
}

void WebEditorClient::showSpellingUI(bool show)
{
	notImplemented();
}

bool WebEditorClient::spellingUIIsShowing()
{
	notImplemented();
	return false;
}

void WebEditorClient::getGuessesForWord(const String& word, const String& context, const VisibleSelection&, Vector<String>& guesses)
{
    guesses.clear();

	notImplemented();
}

void WebEditorClient::willSetInputMethodState()
{
}

void WebEditorClient::setInputMethodState(bool enabled)
{
}
