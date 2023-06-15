// GENERATED FILE - DO NOT EDIT.
// Generated by gen_extensions.py using data from registry_xml.py and gl.xml
//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gles_extensions_autogen.h: GLES extension strings information.

#include "anglebase/no_destructor.h"
#include "libANGLE/Caps.h"

namespace gl
{
const ExtensionInfoMap &GetExtensionInfoMap()
{
    auto buildExtensionInfoMap = []() {
        auto enableableExtension = [](ExtensionBool member) {
            ExtensionInfo info;
            info.Requestable      = true;
            info.ExtensionsMember = member;
            return info;
        };

        auto enableableDisablableExtension = [&](ExtensionBool member) {
            ExtensionInfo info = enableableExtension(member);
            info.Disablable    = true;
            return info;
        };

        auto esOnlyExtension = [](ExtensionBool member) {
            ExtensionInfo info;
            info.ExtensionsMember = member;
            return info;
        };

        // clang-format off
        ExtensionInfoMap map;

        // GLES 2.0 extension strings
        // --------------------------
        map["GL_EXT_base_instance"] = enableableExtension(&Extensions::baseInstanceEXT);
        map["GL_KHR_blend_equation_advanced"] = esOnlyExtension(&Extensions::blendEquationAdvancedKHR);
        map["GL_EXT_blend_func_extended"] = enableableExtension(&Extensions::blendFuncExtendedEXT);
        map["GL_EXT_blend_minmax"] = enableableExtension(&Extensions::blendMinmaxEXT);
        map["GL_EXT_buffer_storage"] = enableableExtension(&Extensions::bufferStorageEXT);
        map["GL_EXT_clip_control"] = enableableExtension(&Extensions::clipControlEXT);
        map["GL_EXT_clip_cull_distance"] = enableableExtension(&Extensions::clipCullDistanceEXT);
        map["GL_APPLE_clip_distance"] = enableableExtension(&Extensions::clipDistanceAPPLE);
        map["GL_EXT_color_buffer_float"] = enableableExtension(&Extensions::colorBufferFloatEXT);
        map["GL_EXT_color_buffer_half_float"] = enableableExtension(&Extensions::colorBufferHalfFloatEXT);
        map["GL_OES_compressed_EAC_R11_signed_texture"] = enableableExtension(&Extensions::compressedEACR11SignedTextureOES);
        map["GL_OES_compressed_EAC_R11_unsigned_texture"] = enableableExtension(&Extensions::compressedEACR11UnsignedTextureOES);
        map["GL_OES_compressed_EAC_RG11_signed_texture"] = enableableExtension(&Extensions::compressedEACRG11SignedTextureOES);
        map["GL_OES_compressed_EAC_RG11_unsigned_texture"] = enableableExtension(&Extensions::compressedEACRG11UnsignedTextureOES);
        map["GL_EXT_compressed_ETC1_RGB8_sub_texture"] = enableableExtension(&Extensions::compressedETC1RGB8SubTextureEXT);
        map["GL_OES_compressed_ETC1_RGB8_texture"] = enableableExtension(&Extensions::compressedETC1RGB8TextureOES);
        map["GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture"] = enableableExtension(&Extensions::compressedETC2PunchthroughARGBA8TextureOES);
        map["GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture"] = enableableExtension(&Extensions::compressedETC2PunchthroughASRGB8AlphaTextureOES);
        map["GL_OES_compressed_ETC2_RGB8_texture"] = enableableExtension(&Extensions::compressedETC2RGB8TextureOES);
        map["GL_OES_compressed_ETC2_RGBA8_texture"] = enableableExtension(&Extensions::compressedETC2RGBA8TextureOES);
        map["GL_OES_compressed_ETC2_sRGB8_alpha8_texture"] = enableableExtension(&Extensions::compressedETC2SRGB8Alpha8TextureOES);
        map["GL_OES_compressed_ETC2_sRGB8_texture"] = enableableExtension(&Extensions::compressedETC2SRGB8TextureOES);
        map["GL_OES_compressed_paletted_texture"] = enableableExtension(&Extensions::compressedPalettedTextureOES);
        map["GL_EXT_copy_image"] = enableableExtension(&Extensions::copyImageEXT);
        map["GL_OES_copy_image"] = enableableExtension(&Extensions::copyImageOES);
        map["GL_KHR_debug"] = esOnlyExtension(&Extensions::debugKHR);
        map["GL_EXT_debug_label"] = esOnlyExtension(&Extensions::debugLabelEXT);
        map["GL_EXT_debug_marker"] = esOnlyExtension(&Extensions::debugMarkerEXT);
        map["GL_OES_depth24"] = esOnlyExtension(&Extensions::depth24OES);
        map["GL_OES_depth32"] = esOnlyExtension(&Extensions::depth32OES);
        map["GL_NV_depth_buffer_float2"] = enableableExtension(&Extensions::depthBufferFloat2NV);
        map["GL_ANGLE_depth_texture"] = esOnlyExtension(&Extensions::depthTextureANGLE);
        map["GL_OES_depth_texture"] = esOnlyExtension(&Extensions::depthTextureOES);
        map["GL_OES_depth_texture_cube_map"] = enableableExtension(&Extensions::depthTextureCubeMapOES);
        map["GL_EXT_discard_framebuffer"] = esOnlyExtension(&Extensions::discardFramebufferEXT);
        map["GL_EXT_disjoint_timer_query"] = enableableExtension(&Extensions::disjointTimerQueryEXT);
        map["GL_EXT_draw_buffers"] = enableableExtension(&Extensions::drawBuffersEXT);
        map["GL_EXT_draw_buffers_indexed"] = enableableExtension(&Extensions::drawBuffersIndexedEXT);
        map["GL_OES_draw_buffers_indexed"] = enableableExtension(&Extensions::drawBuffersIndexedOES);
        map["GL_EXT_draw_elements_base_vertex"] = enableableExtension(&Extensions::drawElementsBaseVertexEXT);
        map["GL_OES_draw_elements_base_vertex"] = enableableExtension(&Extensions::drawElementsBaseVertexOES);
        map["GL_OES_EGL_image"] = enableableExtension(&Extensions::EGLImageOES);
        map["GL_EXT_EGL_image_array"] = enableableExtension(&Extensions::EGLImageArrayEXT);
        map["GL_OES_EGL_image_external"] = enableableExtension(&Extensions::EGLImageExternalOES);
        map["GL_OES_EGL_image_external_essl3"] = enableableExtension(&Extensions::EGLImageExternalEssl3OES);
        map["GL_EXT_EGL_image_external_wrap_modes"] = enableableExtension(&Extensions::EGLImageExternalWrapModesEXT);
        map["GL_EXT_EGL_image_storage"] = enableableExtension(&Extensions::EGLImageStorageEXT);
        map["GL_NV_EGL_stream_consumer_external"] = enableableExtension(&Extensions::EGLStreamConsumerExternalNV);
        map["GL_OES_EGL_sync"] = esOnlyExtension(&Extensions::EGLSyncOES);
        map["GL_OES_element_index_uint"] = enableableExtension(&Extensions::elementIndexUintOES);
        map["GL_ANDROID_extension_pack_es31a"] = esOnlyExtension(&Extensions::extensionPackEs31aANDROID);
        map["GL_EXT_external_buffer"] = enableableExtension(&Extensions::externalBufferEXT);
        map["GL_OES_fbo_render_mipmap"] = enableableExtension(&Extensions::fboRenderMipmapOES);
        map["GL_NV_fence"] = esOnlyExtension(&Extensions::fenceNV);
        map["GL_EXT_float_blend"] = enableableExtension(&Extensions::floatBlendEXT);
        map["GL_EXT_frag_depth"] = enableableExtension(&Extensions::fragDepthEXT);
        map["GL_ANGLE_framebuffer_blit"] = enableableExtension(&Extensions::framebufferBlitANGLE);
        map["GL_NV_framebuffer_blit"] = enableableExtension(&Extensions::framebufferBlitNV);
        map["GL_MESA_framebuffer_flip_y"] = enableableExtension(&Extensions::framebufferFlipYMESA);
        map["GL_EXT_geometry_shader"] = enableableExtension(&Extensions::geometryShaderEXT);
        map["GL_OES_geometry_shader"] = enableableExtension(&Extensions::geometryShaderOES);
        map["GL_OES_get_program_binary"] = enableableExtension(&Extensions::getProgramBinaryOES);
        map["GL_EXT_gpu_shader5"] = enableableExtension(&Extensions::gpuShader5EXT);
        map["GL_ANGLE_instanced_arrays"] = enableableExtension(&Extensions::instancedArraysANGLE);
        map["GL_EXT_instanced_arrays"] = enableableExtension(&Extensions::instancedArraysEXT);
        map["GL_OES_mapbuffer"] = enableableExtension(&Extensions::mapbufferOES);
        map["GL_EXT_map_buffer_range"] = enableableExtension(&Extensions::mapBufferRangeEXT);
        map["GL_EXT_memory_object"] = enableableExtension(&Extensions::memoryObjectEXT);
        map["GL_EXT_memory_object_fd"] = enableableExtension(&Extensions::memoryObjectFdEXT);
        map["GL_EXT_multi_draw_indirect"] = enableableExtension(&Extensions::multiDrawIndirectEXT);
        map["GL_EXT_multisample_compatibility"] = esOnlyExtension(&Extensions::multisampleCompatibilityEXT);
        map["GL_EXT_multisampled_render_to_texture"] = enableableExtension(&Extensions::multisampledRenderToTextureEXT);
        map["GL_EXT_multisampled_render_to_texture2"] = enableableExtension(&Extensions::multisampledRenderToTexture2EXT);
        map["GL_OVR_multiview"] = enableableExtension(&Extensions::multiviewOVR);
        map["GL_OVR_multiview2"] = enableableExtension(&Extensions::multiview2OVR);
        map["GL_KHR_no_error"] = esOnlyExtension(&Extensions::noErrorKHR);
        map["GL_EXT_occlusion_query_boolean"] = enableableExtension(&Extensions::occlusionQueryBooleanEXT);
        map["GL_OES_packed_depth_stencil"] = esOnlyExtension(&Extensions::packedDepthStencilOES);
        map["GL_ANGLE_pack_reverse_row_order"] = enableableExtension(&Extensions::packReverseRowOrderANGLE);
        map["GL_NV_pack_subimage"] = enableableExtension(&Extensions::packSubimageNV);
        map["GL_KHR_parallel_shader_compile"] = enableableExtension(&Extensions::parallelShaderCompileKHR);
        map["GL_AMD_performance_monitor"] = esOnlyExtension(&Extensions::performanceMonitorAMD);
        map["GL_NV_pixel_buffer_object"] = enableableExtension(&Extensions::pixelBufferObjectNV);
        map["GL_EXT_polygon_offset_clamp"] = enableableExtension(&Extensions::polygonOffsetClampEXT);
        map["GL_EXT_primitive_bounding_box"] = esOnlyExtension(&Extensions::primitiveBoundingBoxEXT);
        map["GL_OES_primitive_bounding_box"] = esOnlyExtension(&Extensions::primitiveBoundingBoxOES);
        map["GL_EXT_protected_textures"] = enableableExtension(&Extensions::protectedTexturesEXT);
        map["GL_EXT_pvrtc_sRGB"] = enableableExtension(&Extensions::pvrtcSRGBEXT);
        map["GL_NV_read_depth"] = enableableExtension(&Extensions::readDepthNV);
        map["GL_NV_read_depth_stencil"] = enableableExtension(&Extensions::readDepthStencilNV);
        map["GL_EXT_read_format_bgra"] = enableableExtension(&Extensions::readFormatBgraEXT);
        map["GL_NV_read_stencil"] = enableableExtension(&Extensions::readStencilNV);
        map["GL_OES_rgb8_rgba8"] = enableableExtension(&Extensions::rgb8Rgba8OES);
        map["GL_KHR_robust_buffer_access_behavior"] = esOnlyExtension(&Extensions::robustBufferAccessBehaviorKHR);
        map["GL_EXT_robustness"] = esOnlyExtension(&Extensions::robustnessEXT);
        map["GL_NV_robustness_video_memory_purge"] = esOnlyExtension(&Extensions::robustnessVideoMemoryPurgeNV);
        map["GL_OES_sample_shading"] = enableableExtension(&Extensions::sampleShadingOES);
        map["GL_OES_sample_variables"] = enableableExtension(&Extensions::sampleVariablesOES);
        map["GL_EXT_semaphore"] = enableableExtension(&Extensions::semaphoreEXT);
        map["GL_EXT_semaphore_fd"] = enableableExtension(&Extensions::semaphoreFdEXT);
        map["GL_EXT_separate_shader_objects"] = enableableExtension(&Extensions::separateShaderObjectsEXT);
        map["GL_ARM_shader_framebuffer_fetch"] = enableableExtension(&Extensions::shaderFramebufferFetchARM);
        map["GL_EXT_shader_framebuffer_fetch"] = enableableExtension(&Extensions::shaderFramebufferFetchEXT);
        map["GL_EXT_shader_framebuffer_fetch_non_coherent"] = enableableExtension(&Extensions::shaderFramebufferFetchNonCoherentEXT);
        map["GL_OES_shader_image_atomic"] = enableableExtension(&Extensions::shaderImageAtomicOES);
        map["GL_EXT_shader_io_blocks"] = enableableExtension(&Extensions::shaderIoBlocksEXT);
        map["GL_OES_shader_io_blocks"] = enableableExtension(&Extensions::shaderIoBlocksOES);
        map["GL_OES_shader_multisample_interpolation"] = enableableExtension(&Extensions::shaderMultisampleInterpolationOES);
        map["GL_EXT_shader_non_constant_global_initializers"] = enableableExtension(&Extensions::shaderNonConstantGlobalInitializersEXT);
        map["GL_NV_shader_noperspective_interpolation"] = enableableExtension(&Extensions::shaderNoperspectiveInterpolationNV);
        map["GL_EXT_shader_texture_lod"] = enableableExtension(&Extensions::shaderTextureLodEXT);
        map["GL_QCOM_shading_rate"] = enableableExtension(&Extensions::shadingRateQCOM);
        map["GL_EXT_shadow_samplers"] = enableableExtension(&Extensions::shadowSamplersEXT);
        map["GL_EXT_sRGB"] = enableableExtension(&Extensions::sRGBEXT);
        map["GL_EXT_sRGB_write_control"] = esOnlyExtension(&Extensions::sRGBWriteControlEXT);
        map["GL_OES_standard_derivatives"] = enableableExtension(&Extensions::standardDerivativesOES);
        map["GL_OES_surfaceless_context"] = esOnlyExtension(&Extensions::surfacelessContextOES);
        map["GL_ARB_sync"] = enableableExtension(&Extensions::syncARB);
        map["GL_EXT_tessellation_shader"] = enableableExtension(&Extensions::tessellationShaderEXT);
        map["GL_OES_texture_3D"] = enableableExtension(&Extensions::texture3DOES);
        map["GL_EXT_texture_border_clamp"] = enableableExtension(&Extensions::textureBorderClampEXT);
        map["GL_OES_texture_border_clamp"] = enableableExtension(&Extensions::textureBorderClampOES);
        map["GL_EXT_texture_buffer"] = enableableExtension(&Extensions::textureBufferEXT);
        map["GL_OES_texture_buffer"] = enableableExtension(&Extensions::textureBufferOES);
        map["GL_OES_texture_compression_astc"] = enableableExtension(&Extensions::textureCompressionAstcOES);
        map["GL_KHR_texture_compression_astc_hdr"] = enableableExtension(&Extensions::textureCompressionAstcHdrKHR);
        map["GL_KHR_texture_compression_astc_ldr"] = enableableExtension(&Extensions::textureCompressionAstcLdrKHR);
        map["GL_KHR_texture_compression_astc_sliced_3d"] = enableableExtension(&Extensions::textureCompressionAstcSliced3dKHR);
        map["GL_EXT_texture_compression_bptc"] = enableableExtension(&Extensions::textureCompressionBptcEXT);
        map["GL_EXT_texture_compression_dxt1"] = enableableExtension(&Extensions::textureCompressionDxt1EXT);
        map["GL_IMG_texture_compression_pvrtc"] = enableableExtension(&Extensions::textureCompressionPvrtcIMG);
        map["GL_IMG_texture_compression_pvrtc2"] = enableableExtension(&Extensions::textureCompressionPvrtc2IMG);
        map["GL_EXT_texture_compression_rgtc"] = enableableExtension(&Extensions::textureCompressionRgtcEXT);
        map["GL_EXT_texture_compression_s3tc"] = enableableExtension(&Extensions::textureCompressionS3tcEXT);
        map["GL_EXT_texture_compression_s3tc_srgb"] = enableableExtension(&Extensions::textureCompressionS3tcSrgbEXT);
        map["GL_EXT_texture_cube_map_array"] = enableableExtension(&Extensions::textureCubeMapArrayEXT);
        map["GL_OES_texture_cube_map_array"] = enableableExtension(&Extensions::textureCubeMapArrayOES);
        map["GL_EXT_texture_filter_anisotropic"] = enableableExtension(&Extensions::textureFilterAnisotropicEXT);
        map["GL_OES_texture_float"] = enableableExtension(&Extensions::textureFloatOES);
        map["GL_OES_texture_float_linear"] = enableableExtension(&Extensions::textureFloatLinearOES);
        map["GL_EXT_texture_format_BGRA8888"] = enableableExtension(&Extensions::textureFormatBGRA8888EXT);
        map["GL_EXT_texture_format_sRGB_override"] = esOnlyExtension(&Extensions::textureFormatSRGBOverrideEXT);
        map["GL_OES_texture_half_float"] = enableableExtension(&Extensions::textureHalfFloatOES);
        map["GL_OES_texture_half_float_linear"] = enableableExtension(&Extensions::textureHalfFloatLinearOES);
        map["GL_EXT_texture_norm16"] = enableableExtension(&Extensions::textureNorm16EXT);
        map["GL_OES_texture_npot"] = enableableExtension(&Extensions::textureNpotOES);
        map["GL_EXT_texture_rg"] = enableableExtension(&Extensions::textureRgEXT);
        map["GL_EXT_texture_sRGB_decode"] = esOnlyExtension(&Extensions::textureSRGBDecodeEXT);
        map["GL_EXT_texture_sRGB_R8"] = enableableExtension(&Extensions::textureSRGBR8EXT);
        map["GL_EXT_texture_sRGB_RG8"] = enableableExtension(&Extensions::textureSRGBRG8EXT);
        map["GL_OES_texture_stencil8"] = enableableExtension(&Extensions::textureStencil8OES);
        map["GL_EXT_texture_storage"] = enableableExtension(&Extensions::textureStorageEXT);
        map["GL_OES_texture_storage_multisample_2d_array"] = enableableExtension(&Extensions::textureStorageMultisample2dArrayOES);
        map["GL_EXT_texture_type_2_10_10_10_REV"] = enableableExtension(&Extensions::textureType2101010REVEXT);
        map["GL_ANGLE_texture_usage"] = enableableExtension(&Extensions::textureUsageANGLE);
        map["GL_ANGLE_translated_shader_source"] = esOnlyExtension(&Extensions::translatedShaderSourceANGLE);
        map["GL_EXT_unpack_subimage"] = enableableExtension(&Extensions::unpackSubimageEXT);
        map["GL_OES_vertex_array_object"] = enableableExtension(&Extensions::vertexArrayObjectOES);
        map["GL_OES_vertex_half_float"] = enableableExtension(&Extensions::vertexHalfFloatOES);
        map["GL_OES_vertex_type_10_10_10_2"] = enableableExtension(&Extensions::vertexType1010102OES);
        map["GL_WEBGL_video_texture"] = enableableExtension(&Extensions::videoTextureWEBGL);
        map["GL_EXT_YUV_target"] = enableableExtension(&Extensions::YUVTargetEXT);

        // ANGLE unofficial extension strings
        // ----------------------------------
        map["GL_ANGLE_base_vertex_base_instance"] = enableableExtension(&Extensions::baseVertexBaseInstanceANGLE);
        map["GL_ANGLE_base_vertex_base_instance_shader_builtin"] = enableableExtension(&Extensions::baseVertexBaseInstanceShaderBuiltinANGLE);
        map["GL_CHROMIUM_bind_generates_resource"] = esOnlyExtension(&Extensions::bindGeneratesResourceCHROMIUM);
        map["GL_CHROMIUM_bind_uniform_location"] = esOnlyExtension(&Extensions::bindUniformLocationCHROMIUM);
        map["GL_ANGLE_client_arrays"] = esOnlyExtension(&Extensions::clientArraysANGLE);
        map["GL_ANGLE_clip_cull_distance"] = enableableExtension(&Extensions::clipCullDistanceANGLE);
        map["GL_CHROMIUM_color_buffer_float_rgb"] = enableableExtension(&Extensions::colorBufferFloatRgbCHROMIUM);
        map["GL_CHROMIUM_color_buffer_float_rgba"] = enableableExtension(&Extensions::colorBufferFloatRgbaCHROMIUM);
        map["GL_ANGLE_compressed_texture_etc"] = enableableExtension(&Extensions::compressedTextureEtcANGLE);
        map["GL_CHROMIUM_copy_compressed_texture"] = esOnlyExtension(&Extensions::copyCompressedTextureCHROMIUM);
        map["GL_CHROMIUM_copy_texture"] = esOnlyExtension(&Extensions::copyTextureCHROMIUM);
        map["GL_ANGLE_copy_texture_3d"] = enableableExtension(&Extensions::copyTexture3dANGLE);
        map["GL_CHROMIUM_framebuffer_mixed_samples"] = esOnlyExtension(&Extensions::framebufferMixedSamplesCHROMIUM);
        map["GL_ANGLE_framebuffer_multisample"] = enableableExtension(&Extensions::framebufferMultisampleANGLE);
        map["GL_ANGLE_get_image"] = enableableExtension(&Extensions::getImageANGLE);
        map["GL_ANGLE_get_serialized_context_string"] = esOnlyExtension(&Extensions::getSerializedContextStringANGLE);
        map["GL_ANGLE_get_tex_level_parameter"] = enableableExtension(&Extensions::getTexLevelParameterANGLE);
        map["GL_ANGLE_logic_op"] = enableableExtension(&Extensions::logicOpANGLE);
        map["GL_CHROMIUM_lose_context"] = enableableExtension(&Extensions::loseContextCHROMIUM);
        map["GL_ANGLE_lossy_etc_decode"] = enableableExtension(&Extensions::lossyEtcDecodeANGLE);
        map["GL_ANGLE_memory_object_flags"] = enableableExtension(&Extensions::memoryObjectFlagsANGLE);
        map["GL_ANGLE_memory_object_fuchsia"] = enableableExtension(&Extensions::memoryObjectFuchsiaANGLE);
        map["GL_ANGLE_memory_size"] = enableableExtension(&Extensions::memorySizeANGLE);
        map["GL_ANGLE_multi_draw"] = enableableExtension(&Extensions::multiDrawANGLE);
        map["GL_ANGLE_multiview_multisample"] = enableableExtension(&Extensions::multiviewMultisampleANGLE);
        map["GL_ANGLE_program_binary"] = esOnlyExtension(&Extensions::programBinaryANGLE);
        map["GL_ANGLE_program_cache_control"] = esOnlyExtension(&Extensions::programCacheControlANGLE);
        map["GL_ANGLE_provoking_vertex"] = enableableExtension(&Extensions::provokingVertexANGLE);
        map["GL_ANGLE_read_only_depth_stencil_feedback_loops"] = enableableExtension(&Extensions::readOnlyDepthStencilFeedbackLoopsANGLE);
        map["GL_ANGLE_relaxed_vertex_attribute_type"] = esOnlyExtension(&Extensions::relaxedVertexAttributeTypeANGLE);
        map["GL_ANGLE_request_extension"] = esOnlyExtension(&Extensions::requestExtensionANGLE);
        map["GL_ANGLE_rgbx_internal_format"] = esOnlyExtension(&Extensions::rgbxInternalFormatANGLE);
        map["GL_ANGLE_robust_client_memory"] = esOnlyExtension(&Extensions::robustClientMemoryANGLE);
        map["GL_ANGLE_robust_fragment_shader_output"] = enableableExtension(&Extensions::robustFragmentShaderOutputANGLE);
        map["GL_ANGLE_robust_resource_initialization"] = esOnlyExtension(&Extensions::robustResourceInitializationANGLE);
        map["GL_ANGLE_semaphore_fuchsia"] = enableableExtension(&Extensions::semaphoreFuchsiaANGLE);
        map["GL_ANGLE_shader_binary"] = esOnlyExtension(&Extensions::shaderBinaryANGLE);
        map["GL_ANGLE_shader_pixel_local_storage"] = esOnlyExtension(&Extensions::shaderPixelLocalStorageANGLE);
        map["GL_ANGLE_shader_pixel_local_storage_coherent"] = esOnlyExtension(&Extensions::shaderPixelLocalStorageCoherentANGLE);
        map["GL_CHROMIUM_sync_query"] = enableableExtension(&Extensions::syncQueryCHROMIUM);
        map["GL_ANGLE_texture_compression_dxt3"] = enableableExtension(&Extensions::textureCompressionDxt3ANGLE);
        map["GL_ANGLE_texture_compression_dxt5"] = enableableExtension(&Extensions::textureCompressionDxt5ANGLE);
        map["GL_ANGLE_texture_external_update"] = enableableExtension(&Extensions::textureExternalUpdateANGLE);
        map["GL_CHROMIUM_texture_filtering_hint"] = enableableExtension(&Extensions::textureFilteringHintCHROMIUM);
        map["GL_ANGLE_texture_multisample"] = enableableExtension(&Extensions::textureMultisampleANGLE);
        map["GL_ANGLE_texture_rectangle"] = enableableDisablableExtension(&Extensions::textureRectangleANGLE);
        map["GL_ANGLE_vulkan_image"] = enableableExtension(&Extensions::vulkanImageANGLE);
        map["GL_ANGLE_webgl_compatibility"] = esOnlyExtension(&Extensions::webglCompatibilityANGLE);
        map["GL_ANGLE_yuv_internal_format"] = enableableExtension(&Extensions::yuvInternalFormatANGLE);

        // GLES 1.0 and 1.1 extension strings
        // ----------------------------------
        map["GL_OES_draw_texture"] = enableableExtension(&Extensions::drawTextureOES);
        map["GL_OES_framebuffer_object"] = enableableExtension(&Extensions::framebufferObjectOES);
        map["GL_OES_matrix_palette"] = enableableExtension(&Extensions::matrixPaletteOES);
        map["GL_OES_point_size_array"] = enableableExtension(&Extensions::pointSizeArrayOES);
        map["GL_OES_point_sprite"] = enableableExtension(&Extensions::pointSpriteOES);
        map["GL_OES_query_matrix"] = enableableExtension(&Extensions::queryMatrixOES);
        map["GL_OES_texture_cube_map"] = enableableExtension(&Extensions::textureCubeMapOES);
        // clang-format on

#if defined(ANGLE_ENABLE_ASSERTS)
        // Verify all extension strings start with GL_
        for (const auto &extension : map)
        {
            ASSERT(extension.first.rfind("GL_", 0) == 0);
        }
#endif

        return map;
    };

    static const angle::base::NoDestructor<ExtensionInfoMap> extensionInfo(buildExtensionInfoMap());
    return *extensionInfo;
}
}  // namespace gl
