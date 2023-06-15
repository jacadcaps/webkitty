/*
 * Copyright (C) 2005-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// These are private because callers should be using the cover methods. They are in
// a Private (as opposed to Internal) header file because Safari uses some of them
// for managed preferences.
#define WebKitStandardFontPreferenceKey @"WebKitStandardFont"
#define WebKitFixedFontPreferenceKey @"WebKitFixedFont"
#define WebKitSerifFontPreferenceKey @"WebKitSerifFont"
#define WebKitSansSerifFontPreferenceKey @"WebKitSansSerifFont"
#define WebKitCursiveFontPreferenceKey @"WebKitCursiveFont"
#define WebKitFantasyFontPreferenceKey @"WebKitFantasyFont"
#define WebKitPictographFontPreferenceKey @"WebKitPictographFont"
#define WebKitMinimumFontSizePreferenceKey @"WebKitMinimumFontSize"
#define WebKitMinimumLogicalFontSizePreferenceKey @"WebKitMinimumLogicalFontSize"
#define WebKitDefaultFontSizePreferenceKey @"WebKitDefaultFontSize"
#define WebKitDefaultFixedFontSizePreferenceKey @"WebKitDefaultFixedFontSize"
#define WebKitDefaultTextEncodingNamePreferenceKey @"WebKitDefaultTextEncodingName"
#define WebKitUsesEncodingDetectorPreferenceKey @"WebKitUsesEncodingDetector"
#define WebKitUserStyleSheetEnabledPreferenceKey @"WebKitUserStyleSheetEnabledPreferenceKey"
#define WebKitUserStyleSheetLocationPreferenceKey @"WebKitUserStyleSheetLocationPreferenceKey"
#define WebKitShouldPrintBackgroundsPreferenceKey @"WebKitShouldPrintBackgroundsPreferenceKey"
#if !TARGET_OS_IPHONE
#define WebKitTextAreasAreResizablePreferenceKey @"WebKitTextAreasAreResizable"
#endif
#define WebKitShrinksStandaloneImagesToFitPreferenceKey @"WebKitShrinksStandaloneImagesToFit"
#define WebKitJavaScriptEnabledPreferenceKey @"WebKitJavaScriptEnabled"
#define WebKitJavaScriptMarkupEnabledPreferenceKey @"WebKitJavaScriptMarkupEnabled"
#define WebKitWebSecurityEnabledPreferenceKey @"WebKitWebSecurityEnabled"
#define WebKitAllowUniversalAccessFromFileURLsPreferenceKey @"WebKitAllowUniversalAccessFromFileURLs"
#define WebKitAllowFileAccessFromFileURLsPreferenceKey @"WebKitAllowFileAccessFromFileURLs"
#define WebKitAllowCrossOriginSubresourcesToAskForCredentialsKey @"WebKitAllowCrossOriginSubresourcesToAskForCredentials"
#define WebKitAllowTopNavigationToDataURLsPreferenceKey @"WebKitAllowTopNavigationToDataURLs"
#define WebKitNeedsStorageAccessFromFileURLsQuirkKey @"WebKitNeedsStorageAccessFromFileURLsQuirk"
#define WebKitJavaScriptCanOpenWindowsAutomaticallyPreferenceKey @"WebKitJavaScriptCanOpenWindowsAutomatically"
#define WebKitPluginsEnabledPreferenceKey @"WebKitPluginsEnabled"
#define WebKitDatabasesEnabledPreferenceKey @"WebKitDatabasesEnabledPreferenceKey"
#define WebKitLocalStorageEnabledPreferenceKey @"WebKitLocalStorageEnabledPreferenceKey"
#define WebKitAllowAnimatedImagesPreferenceKey @"WebKitAllowAnimatedImagesPreferenceKey"
#define WebKitAllowAnimatedImageLoopingPreferenceKey @"WebKitAllowAnimatedImageLoopingPreferenceKey"
#define WebKitDisplayImagesKey @"WebKitDisplayImagesKey"
#define WebKitAdditionalSupportedImageTypesKey  @"WebKitAdditionalSupportedImageTypesKey"
#define WebKitLoadSiteIconsKey @"WebKitLoadSiteIconsKey"
#define WebKitBackForwardCacheExpirationIntervalKey @"WebKitBackForwardCacheExpirationIntervalKey"
#if !TARGET_OS_IPHONE
#define WebKitTabToLinksPreferenceKey @"WebKitTabToLinksPreferenceKey"
#endif
#define WebKitPrivateBrowsingEnabledPreferenceKey @"WebKitPrivateBrowsingEnabled"
#define WebSmartInsertDeleteEnabled @"WebSmartInsertDeleteEnabled"
#if !TARGET_OS_IPHONE
#define WebContinuousSpellCheckingEnabled @"WebContinuousSpellCheckingEnabled"
#define WebGrammarCheckingEnabled @"WebGrammarCheckingEnabled"
#endif
#define WebAutomaticQuoteSubstitutionEnabled @"WebAutomaticQuoteSubstitutionEnabled"
#define WebAutomaticLinkDetectionEnabled @"WebAutomaticLinkDetectionEnabled"
#define WebAutomaticDashSubstitutionEnabled @"WebAutomaticDashSubstitutionEnabled"
#define WebAutomaticTextReplacementEnabled @"WebAutomaticTextReplacementEnabled"
#define WebAutomaticSpellingCorrectionEnabled @"WebAutomaticSpellingCorrectionEnabled"
#define WebKitDOMPasteAllowedPreferenceKey @"WebKitDOMPasteAllowedPreferenceKey"
#define WebKitUsesPageCachePreferenceKey @"WebKitUsesPageCachePreferenceKey"
#define WebKitPageCacheSupportsPluginsPreferenceKey @"WebKitPageCacheSupportsPluginsPreferenceKey"
#define WebKitFTPDirectoryTemplatePath @"WebKitFTPDirectoryTemplatePath"
#define WebKitForceFTPDirectoryListings @"WebKitForceFTPDirectoryListings"
#define WebKitDeveloperExtrasEnabledPreferenceKey @"WebKitDeveloperExtrasEnabledPreferenceKey"
#define WebKitJavaScriptRuntimeFlagsPreferenceKey @"WebKitJavaScriptRuntimeFlagsPreferenceKey"
#define WebKitAuthorAndUserStylesEnabledPreferenceKey @"WebKitAuthorAndUserStylesEnabledPreferenceKey"
#define WebKitDOMTimersThrottlingEnabledPreferenceKey @"WebKitDOMTimersThrottlingEnabledPreferenceKey"
#define WebKitWebArchiveDebugModeEnabledPreferenceKey @"WebKitWebArchiveDebugModeEnabledPreferenceKey"
#define WebKitLocalFileContentSniffingEnabledPreferenceKey @"WebKitLocalFileContentSniffingEnabledPreferenceKey"
#define WebKitLocalStorageDatabasePathPreferenceKey @"WebKitLocalStorageDatabasePathPreferenceKey"
#define WebKitEnableFullDocumentTeardownPreferenceKey @"WebKitEnableFullDocumentTeardown"
#define WebKitOfflineWebApplicationCacheEnabledPreferenceKey @"WebKitOfflineWebApplicationCacheEnabled"
#define WebKitApplicationCacheTotalQuota @"WebKitApplicationCacheTotalQuota"
#define WebKitApplicationCacheDefaultOriginQuota @"WebKitApplicationCacheDefaultOriginQuota"
#define WebKitZoomsTextOnlyPreferenceKey @"WebKitZoomsTextOnly"
#define WebKitJavaScriptCanAccessClipboardPreferenceKey @"WebKitJavaScriptCanAccessClipboard"
#define WebKitAcceleratedDrawingEnabledPreferenceKey @"WebKitAcceleratedDrawingEnabled"
#define WebKitDisplayListDrawingEnabledPreferenceKey @"WebKitDisplayListDrawingEnabled"
#define WebKitCanvasUsesAcceleratedDrawingPreferenceKey @"WebKitCanvasUsesAcceleratedDrawing"
#define WebKitAcceleratedCompositingEnabledPreferenceKey @"WebKitAcceleratedCompositingEnabled"
#define WebKitShowDebugBordersPreferenceKey @"WebKitShowDebugBorders"
#define WebKitSubpixelAntialiasedLayerTextEnabledPreferenceKey @"WebKitSubpixelAntialiasedLayerTextEnabled"
#define WebKitSimpleLineLayoutEnabledPreferenceKey @"WebKitSimpleLineLayoutEnabled"
#define WebKitLegacyLineLayoutVisualCoverageEnabledPreferenceKey @"WebKitLegacyLineLayoutVisualCoverageEnabled"
#define WebKitContentChangeObserverEnabledPreferenceKey @"WebKitContentChangeObserverEnabled"
#define WebKitShowRepaintCounterPreferenceKey @"WebKitShowRepaintCounter"
#define WebKitWebAudioEnabledPreferenceKey @"WebKitWebAudioEnabled"
#define WebKitWebGLEnabledPreferenceKey @"WebKitWebGLEnabled"
#define WebKitForceWebGLUsesLowPowerPreferenceKey @"WebKitForceWebGLUsesLowPower"
#define WebKitAccelerated2dCanvasEnabledPreferenceKey @"WebKitAccelerated2dCanvasEnabled"
#define WebKitFrameFlatteningPreferenceKey @"WebKitFrameFlattening"
#define WebKitAsyncFrameScrollingEnabledPreferenceKey @"WebKitAsyncFrameScrollingEnabled"
#define WebKitSpatialNavigationEnabledPreferenceKey @"WebKitSpatialNavigationEnabled"
#define WebKitDNSPrefetchingEnabledPreferenceKey @"WebKitDNSPrefetchingEnabled"
#define WebKitFullScreenEnabledPreferenceKey @"WebKitFullScreenEnabled"
#define WebKitAsynchronousSpellCheckingEnabledPreferenceKey @"WebKitAsynchronousSpellCheckingEnabled"
#define WebKitHyperlinkAuditingEnabledPreferenceKey @"WebKitHyperlinkAuditingEnabled"
#define WebKitAVFoundationEnabledKey @"WebKitAVFoundationEnabled"
#define WebKitRequiresUserGestureForMediaPlaybackPreferenceKey @"WebKitMediaPlaybackRequiresUserGesture"
#define WebKitRequiresUserGestureForVideoPlaybackPreferenceKey @"WebKitVideoPlaybackRequiresUserGesture"
#define WebKitRequiresUserGestureForAudioPlaybackPreferenceKey @"WebKitAudioPlaybackRequiresUserGesture"
#define WebKitMainContentUserGestureOverrideEnabledPreferenceKey @"WebKitMainContentUserGestureOverrideEnabled"
#define WebKitAllowsInlineMediaPlaybackPreferenceKey @"WebKitMediaPlaybackAllowsInline"
#define WebKitAllowsInlineMediaPlaybackAfterFullscreenPreferenceKey @"WebKitAllowsInlineMediaPlaybackAfterFullscreen"
#define WebKitInlineMediaPlaybackRequiresPlaysInlineAttributeKey @"InlineMediaPlaybackRequiresPlaysInlineAttribute"
#define WebKitInvisibleAutoplayNotPermittedKey @"InvisibleAutoplayNotPermitted"
#define WebKitAllowsPictureInPictureMediaPlaybackPreferenceKey @"WebKitAllowsPictureInPictureMediaPlayback"
#define WebKitAllowsAirPlayForMediaPlaybackPreferenceKey @"WebKitMediaPlaybackAllowsAirPlay"
#define WebKitMediaControlsScaleWithPageZoomPreferenceKey @"WebKitMediaControlsScaleWithPageZoom"
#define WebKitMockScrollbarsEnabledPreferenceKey @"WebKitMockScrollbarsEnabled"
#define WebKitShouldDisplaySubtitlesPreferenceKey @"WebKitShouldDisplaySubtitles"
#define WebKitShouldDisplayCaptionsPreferenceKey @"WebKitShouldDisplayCaptions"
#define WebKitShouldDisplayTextDescriptionsPreferenceKey @"WebKitShouldDisplayTextDescriptions"
#define WebKitNotificationsEnabledKey @"WebKitNotificationsEnabled"
#define WebKitSuppressesIncrementalRenderingKey @"WebKitSuppressesIncrementalRendering"
#define WebKitResourceLoadStatisticsEnabledPreferenceKey @"WebKitResourceLoadStatisticsEnabled"
#define WebKitLargeImageAsyncDecodingEnabledPreferenceKey @"WebKitLargeImageAsyncDecodingEnabled"
#define WebKitAnimatedImageAsyncDecodingEnabledPreferenceKey @"WebKitAnimatedImageAsyncDecodingEnabled"
#if TARGET_OS_IPHONE
#define WebKitAudioSessionCategoryOverride @"WebKitAudioSessionCategoryOverride"
#endif
#define WebKitShouldRespectImageOrientationKey @"WebKitShouldRespectImageOrientation"
#define WebKitRequestAnimationFrameEnabledPreferenceKey @"WebKitRequestAnimationFrameEnabled"
#define WebKitDiagnosticLoggingEnabledKey @"WebKitDiagnosticLoggingEnabled"
#define WebKitStorageBlockingPolicyKey @"WebKitStorageBlockingPolicy"
#define WebKitPlugInSnapshottingEnabledPreferenceKey @"WebKitPlugInSnapshottingEnabled"
#define WebKitHiddenPageDOMTimerThrottlingEnabledPreferenceKey @"WebKitHiddenPageDOMTimerThrottlingEnabled"
#define WebKitHiddenPageCSSAnimationSuspensionEnabledPreferenceKey @"WebKitHiddenPageCSSAnimationSuspensionEnabled"
#define WebKitLowPowerVideoAudioBufferSizeEnabledPreferenceKey @"WebKitLowPowerVideoAudioBufferSizeEnabled"
#define WebKitUseLegacyTextAlignPositionedElementBehaviorPreferenceKey @"WebKitUseLegacyTextAlignPositionedElementBehavior"
#define WebKitMediaSourceEnabledPreferenceKey @"WebKitMediaSourceEnabled"
#define WebKitSourceBufferChangeTypeEnabledPreferenceKey @"WebKitSourceBufferChangeTypeEnabled"
#define WebKitShouldConvertPositionStyleOnCopyPreferenceKey @"WebKitShouldConvertPositionStyleOnCopy"
#define WebKitGamepadsEnabledPreferenceKey @"WebKitGamepadsEnabled"
#define WebKitServiceControlsEnabledPreferenceKey @"WebKitServiceControlsEnabled"
#define WebKitMediaKeysStorageDirectoryKey @"WebKitMediaKeysStorageDirectory"
#define WebKitDataTransferItemsEnabledPreferenceKey @"WebKitDataTransferItemsEnabled"
#define WebKitCustomPasteboardDataEnabledPreferenceKey @"WebKitCustomPasteboardDataEnabled"
#define WebKitKeygenElementEnabledPreferenceKey @"WebKitKeygenElementEnabledPreferenceKey"
#define WebKitCacheAPIEnabledPreferenceKey @"WebKitCacheAPIEnabled"
#define WebKitDownloadAttributeEnabledPreferenceKey @"WebKitDownloadAttributeEnabled"
#define WebKitDirectoryUploadEnabledPreferenceKey @"WebKitDirectoryUploadEnabled"
#define WebKitCSSOMViewScrollingAPIEnabledPreferenceKey @"WebKitCSSOMViewScrollingAPIEnabled"
#define WebKitSubtleCryptoEnabledPreferenceKey @"WebKitSubtleCryptoEnabled"
#define WebKitMediaDevicesEnabledPreferenceKey @"WebKitMediaDevicesEnabled"
#define WebKitMediaStreamEnabledPreferenceKey @"WebKitMediaStreamEnabled"
#define WebKitPeerConnectionEnabledPreferenceKey @"WebKitPeerConnectionEnabled"
#define WebKitLinkPreloadEnabledPreferenceKey @"WebKitLinkPreloadEnabled"
#define WebKitMediaPreloadingEnabledPreferenceKey @"WebKitMediaPreloadingEnabled"
#define WebKitMediaUserGestureInheritsFromDocument @"WebKitMediaUserGestureInheritsFromDocument"
#define WebKitConstantPropertiesEnabledPreferenceKey @"WebKitConstantPropertiesEnabled"
#define WebKitColorFilterEnabledPreferenceKey @"WebKitColorFilterEnabled"
#define WebKitPunchOutWhiteBackgroundsInDarkModePreferenceKey @"WebKitPunchOutWhiteBackgroundsInDarkMode"
#define WebKitLayoutFormattingContextIntegrationEnabledPreferenceKey @"WebKitLayoutFormattingContextIntegrationEnabled"
#define WebKitWebSQLEnabledPreferenceKey @"WebKitWebSQLEnabled"

#if !TARGET_OS_IPHONE
// These are private both because callers should be using the cover methods and because the
// cover methods themselves are private.
#define WebKitRespectStandardStyleKeyEquivalentsPreferenceKey @"WebKitRespectStandardStyleKeyEquivalents"
#define WebKitShowsURLsInToolTipsPreferenceKey @"WebKitShowsURLsInToolTips"
#define WebKitShowsToolTipOverTruncatedTextPreferenceKey @"WebKitShowsToolTipOverTruncatedText"
#define WebKitPDFDisplayModePreferenceKey @"WebKitPDFDisplayMode"
#define WebKitPDFScaleFactorPreferenceKey @"WebKitPDFScaleFactor"
#endif

#define WebKitUseSiteSpecificSpoofingPreferenceKey @"WebKitUseSiteSpecificSpoofing"
#define WebKitEditableLinkBehaviorPreferenceKey @"WebKitEditableLinkBehavior"
#define WebKitCacheModelPreferenceKey @"WebKitCacheModelPreferenceKey"
#define WebKitTextDirectionSubmenuInclusionBehaviorPreferenceKey @"WebKitTextDirectionSubmenuInclusionBehaviorPreferenceKey"
#define WebKitUsePreHTML5ParserQuirksKey @"WebKitUsePreHTML5ParserQuirks"
#define WebKitBackspaceKeyNavigationEnabledKey @"WebKitBackspaceKeyNavigationEnabled"
#define WebKitIncrementalRenderingSuppressionTimeoutInSecondsKey @"WebKitIncrementalRenderingSuppressionTimeoutInSeconds"
#define WebKitWantsBalancedSetDefersLoadingBehaviorKey @"WebKitWantsBalancedSetDefersLoadingBehavior"
#define WebKitDebugFullPageZoomPreferenceKey @"WebKitDebugFullPageZoomPreferenceKey"
#define WebKitMinimumZoomFontSizePreferenceKey @"WebKitMinimumZoomFontSizePreferenceKey"
#define WebKitTextAutosizingEnabledPreferenceKey @"WebKitTextAutosizingEnabled"
#define WebKitHTTPEquivEnabledPreferenceKey @"WebKitHTTPEquivEnabled"

#if TARGET_OS_IPHONE
#define WebKitStandalonePreferenceKey @"WebKitStandalonePreferenceKey"
#define WebKitTelephoneParsingEnabledPreferenceKey @"WebKitTelephoneParsingEnabledPreferenceKey"
#define WebKitAllowMultiElementImplicitFormSubmissionPreferenceKey @"WebKitAllowMultiElementImplicitFormSubmissionPreferenceKey"
#define WebKitAlwaysRequestGeolocationPermissionPreferenceKey @"WebKitAlwaysRequestGeolocationPermission"
#define WebKitMaxParseDurationPreferenceKey @"WebKitMaxParseDurationPreferenceKey"
#define WebKitStorageTrackerEnabledPreferenceKey @"WebKitStorageTrackerEnabledPreferenceKey"
#define WebKitInterpolationQualityPreferenceKey @"WebKitInterpolationQualityPreferenceKey"
#define WebKitPasswordEchoEnabledPreferenceKey @"WebKitEnablePasswordEchoPreferenceKey"
#define WebKitPasswordEchoDurationPreferenceKey @"WebKitPasswordEchoDurationPreferenceKey"
#define WebKitNetworkDataUsageTrackingEnabledPreferenceKey @"WebKitNetworkDataUsageTrackingEnabledPreferenceKey"
#define WebKitNetworkInterfaceNamePreferenceKey @"WebKitNetworkInterfaceNamePreferenceKey"
#define WebKitQuickLookDocumentSavingPreferenceKey @"WebKitQuickLookDocumentSavingPreferenceKey"
#endif

#define WebKitEnableInheritURIQueryComponentPreferenceKey @"WebKitEnableInheritURIQueryComponent"
#define WebKitMediaDataLoadsAutomaticallyPreferenceKey @"WebKitMediaDataLoadsAutomatically"
#define WebKitMockCaptureDevicesEnabledPreferenceKey @"WebKitMockCaptureDevicesEnabled"
#define WebKitMockCaptureDevicesPromptEnabledPreferenceKey @"WebKitMockCaptureDevicesPromptEnabled"
#define WebKitEnumeratingAllNetworkInterfacesEnabledPreferenceKey @"WebKitEnumeratingAllNetworkInterfacesEnabled"
#define WebKitICECandidateFilteringEnabledPreferenceKey @"WebKitICECandidateFilteringEnabled"
#define WebKitMediaCaptureRequiresSecureConnectionPreferenceKey @"WebKitMediaCaptureRequiresSecureConnection"
#define WebKitAttachmentElementEnabledPreferenceKey @"WebKitAttachmentElementEnabled"
#define WebKitMenuItemElementEnabledPreferenceKey @"WebKitMenuItemElementEnabled"
#define WebKitUserTimingEnabledPreferenceKey @"WebKitUserTimingEnabled"
#define WebKitResourceTimingEnabledPreferenceKey @"WebKitResourceTimingEnabled"
#define WebKitMediaContentTypesRequiringHardwareSupportPreferenceKey @"WebKitMediaContentTypesRequiringHardwareSupport"
#define WebKitLegacyEncryptedMediaAPIEnabledKey @"WebKitLegacyEncryptedMediaAPIEnabled"
#define WebKitEncryptedMediaAPIEnabledKey @"WebKitEncryptedMediaAPIEnabled"
#define WebKitPictureInPictureAPIEnabledKey @"WebKitPictureInPictureAPIEnabled"
#define WebKitAllowMediaContentTypesRequiringHardwareSupportAsFallbackKey @"WebKitAllowMediaContentTypesRequiringHardwareSupportAsFallback"
#define WebKitMediaCapabilitiesEnabledPreferenceKey @"WebKitMediaCapabilitiesEnabled"
#define WebKitLineHeightUnitsEnabledPreferenceKey @"WebKitLineHeightUnitsEnabled"
#define WebKitDebugInAppBrowserPrivacyEnabledPreferenceKey @"WebKitDebugInAppBrowserPrivacyEnabled"


// These are all now generated via GeneratePreferences.rb
// FIXME: If these are not used anywhere, we should remove them and only use WebFeature mechanism for the preference.
#define WebKitUserGesturePromisePropagationEnabledPreferenceKey @"WebKitUserGesturePromisePropagationEnabled"
#define WebKitRequestIdleCallbackEnabledPreferenceKey @"WebKitRequestIdleCallbackEnabled"
#define WebKitHighlightAPIEnabledPreferenceKey @"WebKitHighlightAPIEnabled"
#define WebKitAsyncClipboardAPIEnabledPreferenceKey @"WebKitAsyncClipboardAPIEnabled"
#define WebKitIntersectionObserverEnabledPreferenceKey @"WebKitIntersectionObserverEnabled"
#define WebKitVisualViewportAPIEnabledPreferenceKey @"WebKitVisualViewportAPIEnabled"
#define WebKitSyntheticEditingCommandsEnabledPreferenceKey @"WebKitSyntheticEditingCommandsEnabled"
#define WebKitCSSOMViewSmoothScrollingEnabledPreferenceKey @"WebKitCSSOMViewSmoothScrollingEnabled"
#define WebKitWebAnimationsCompositeOperationsEnabledPreferenceKey @"WebKitWebAnimationsCompositeOperationsEnabled"
#define WebKitWebAnimationsMutableTimelinesEnabledPreferenceKey @"WebKitWebAnimationsMutableTimelinesEnabled"
#define WebKitMaskWebGLStringsEnabledPreferenceKey @"WebKitMaskWebGLStringsEnabled"
#define WebKitServerTimingEnabledPreferenceKey @"WebKitServerTimingEnabled"
#define WebKitCSSCustomPropertiesAndValuesEnabledPreferenceKey @"WebKitCSSCustomPropertiesAndValuesEnabled"
#define WebKitResizeObserverEnabledPreferenceKey @"WebKitResizeObserverEnabled"
#define WebKitPrivateClickMeasurementEnabledPreferenceKey @"WebKitPrivateClickMeasurementEnabled"
#define WebKitFetchAPIKeepAliveEnabledPreferenceKey @"WebKitFetchAPIKeepAliveEnabled"
#define WebKitGenericCueAPIEnabledKey @"WebKitGenericCueAPIEnabled"
#define WebKitAspectRatioOfImgFromWidthAndHeightEnabledPreferenceKey @"WebKitAspectRatioOfImgFromWidthAndHeightEnabled"
#define WebKitReferrerPolicyAttributeEnabledPreferenceKey @"WebKitReferrerPolicyAttributeEnabled"
#define WebKitCoreMathMLEnabledPreferenceKey @"WebKitCoreMathMLEnabled"
#define WebKitLinkPreloadResponsiveImagesEnabledPreferenceKey @"WebKitLinkPreloadResponsiveImagesEnabled"
#define WebKitRemotePlaybackEnabledPreferenceKey @"WebKitRemotePlaybackEnabled"
#define WebKitReadableByteStreamAPIEnabledPreferenceKey @"WebKitReadableByteStreamAPIEnabled"
#define WebKitTransformStreamAPIEnabledPreferenceKey @"WebKitTransformStreamAPIEnabled"
#define WebKitMediaRecorderEnabledPreferenceKey @"WebKitMediaRecorderEnabled"
#define WebKitCSSIndividualTransformPropertiesEnabledPreferenceKey @"WebKitCSSIndividualTransformPropertiesEnabled"
#define WebKitContactPickerAPIEnabledPreferenceKey @"WebKitContactPickerAPIEnabled"
#define WebKitSpeechRecognitionEnabledPreferenceKey @"WebKitSpeechRecognitionEnabled"
#define WebKitPitchCorrectionAlgorithmPreferenceKey @"WebKitPitchCorrectionAlgorithm"

// The preference keys below this point are deprecated and have no effect. They should
// be removed when it is considered safe to do so.
#define WebKitShadowDOMEnabledPreferenceKey @"WebKitShadowDOMEnabled"
#define WebKitHixie76WebSocketProtocolEnabledKey @"WebKitHixie76WebSocketProtocolEnabled"
#define WebKitCustomElementsEnabledPreferenceKey @"WebKitCustomElementsEnabled"
#define WebKitFetchAPIEnabledPreferenceKey @"WebKitFetchAPIEnabled"
#define WebKitIsSecureContextAttributeEnabledPreferenceKey @"WebKitIsSecureContextAttributeEnabled"
#define WebKitCSSShadowPartsEnabledPreferenceKey @"WebKitCSSShadowPartsEnabled"
#define WebKitSubpixelCSSOMElementMetricsEnabledPreferenceKey @"WebKitSubpixelCSSOMElementMetricsEnabled"
#define WebKitExperimentalNotificationsEnabledPreferenceKey @"WebKitExperimentalNotificationsEnabledPreferenceKey"
#define WebKitXSSAuditorEnabledPreferenceKey @"WebKitXSSAuditorEnabled"
#define WebKitAVFoundationNSURLSessionEnabledKey @"WebKitAVFoundationNSURLSessionEnabled"
