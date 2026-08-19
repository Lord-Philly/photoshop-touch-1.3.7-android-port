#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
 * Clean-room compatibility layer for the obsolete TTPixel JNI surface.
 *
 * The original library cannot be loaded by current Android linkers because it
 * contains text relocations. These entry points intentionally fail soft while
 * the ActionScript-side replacement is developed. They are not a reimplementation
 * of Adobe's image-processing algorithms.
 */

#define STUB_VOID(symbol) \
    JNIEXPORT void JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
    }

#define STUB_BOOL(symbol) \
    JNIEXPORT jboolean JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return JNI_FALSE; \
    }

#define STUB_INT(symbol) \
    JNIEXPORT jint JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return 0; \
    }

#define STUB_LONG(symbol) \
    JNIEXPORT jlong JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return 0; \
    }

#define STUB_FLOAT(symbol) \
    JNIEXPORT jfloat JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return 0.0f; \
    }

#define STUB_DOUBLE(symbol) \
    JNIEXPORT jdouble JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return 0.0; \
    }

#define STUB_OBJECT(symbol) \
    JNIEXPORT jobject JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        return NULL; \
    }

JNIEXPORT jobject JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_allocateDirect(
        JNIEnv *env, jclass clazz, jint capacity) {
    (void)clazz;
    if (capacity <= 0) {
        return NULL;
    }
    void *memory = calloc(1, (size_t)capacity);
    if (memory == NULL) {
        return NULL;
    }
    jobject buffer = (*env)->NewDirectByteBuffer(env, memory, capacity);
    if (buffer == NULL) {
        free(memory);
    }
    return buffer;
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_freeDirect(
        JNIEnv *env, jclass clazz, jobject buffer) {
    (void)clazz;
    if (buffer != NULL) {
        void *memory = (*env)->GetDirectBufferAddress(env, buffer);
        free(memory);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_copyDirect(
        JNIEnv *env, jclass clazz, jobject source, jobject destination) {
    (void)clazz;
    if (source == NULL || destination == NULL) {
        return JNI_FALSE;
    }
    void *src = (*env)->GetDirectBufferAddress(env, source);
    void *dst = (*env)->GetDirectBufferAddress(env, destination);
    jlong src_size = (*env)->GetDirectBufferCapacity(env, source);
    jlong dst_size = (*env)->GetDirectBufferCapacity(env, destination);
    if (src == NULL || dst == NULL || src_size < 0 || dst_size < 0) {
        return JNI_FALSE;
    }
    memcpy(dst, src, (size_t)(src_size < dst_size ? src_size : dst_size));
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_copySwapColorChannels(
        JNIEnv *env, jclass clazz, jobject source, jobject destination) {
    return Java_com_adobe_ttpixel_extension_ByteBufferFactory_copyDirect(
            env, clazz, source, destination);
}

STUB_BOOL(Java_com_adobe_ttpixel_extension_ByteBufferFactory_flipImageHor);

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getInstalledCPUCount(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    long count = sysconf(_SC_NPROCESSORS_CONF);
    return (jint)(count > 0 ? count : 1);
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getOnlineCPUCount(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    return (jint)(count > 0 ? count : 1);
}

JNIEXPORT jdouble JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getTimestamp(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (jdouble)now.tv_sec + ((jdouble)now.tv_nsec / 1000000000.0);
}

STUB_BOOL(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_perform);
STUB_BOOL(Java_com_adobe_ttpixel_extension_httpd_Httpd_modifyPasswdFile);
STUB_BOOL(Java_com_adobe_ttpixel_extension_httpd_Httpd_start);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_clearEncodedData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_hasFinishedEncoding);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_isPossiblyPremultipliedData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeJPEG);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeLz4);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodePNG);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeZLib);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_brushHit);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_brushHits);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_createQuickSelectTool);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_disposeQuickSelectTool);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_getMask);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_mouseUp);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_qsBrushHits);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_reset);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_setMask);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_setMaskWithARGB);
STUB_BOOL(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextQuickSelection_switchMode);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_alphaBlend);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataCopy);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataFromFileEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataResample);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataToFileEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileCreateEmpty);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileCreateFromBitmapData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileRead);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileResample);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileWrite);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_copyBitmapData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_getPixelsBitmapEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_getPixelsEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_moveBitmapDataEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_setPixelsEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_uncompressBitmapDataEx);
STUB_FLOAT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1getPressure);
STUB_INT(Java_com_adobe_ttpixel_extension_am_ECAlphaMatting_getProgress);
STUB_INT(Java_com_adobe_ttpixel_extension_am_ECAlphaMatting_getResult);
STUB_INT(Java_com_adobe_ttpixel_extension_am_ECAlphaMatting_init);
STUB_INT(Java_com_adobe_ttpixel_extension_am_ECAlphaMatting_release);
STUB_INT(Java_com_adobe_ttpixel_extension_am_ECAlphaMatting_run);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLContext_createContext);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLContext_destroyContext);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLContext_getLastGLErrorCode);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_applyFilter);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_asyncExecuteSequence);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_asyncInterrupt);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_asyncJoin);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_asyncReadPixels);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_clear);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_clearVertexAttribData);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_createFilter);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_destroyFilter);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_drawArrays);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_enableVertexAttrib);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_executeSequence);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_getNumVertices);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_getVertexAttribDataRangeCount);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_getVertexAttribDataRangeStart);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_programCreate);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_programDispose);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_programUse);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_readPixels);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_readPixelsRect);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setDrawMode);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setNumVertices);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setParamFloat);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setParamFloatMatrix);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setParamFloatMulti);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setParamInt);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setParamIntMulti);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setVertexAttribDataFloat);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_setVertexAttribDataRange);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_shaderCreate);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_shaderDispose);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_texturePrepare);
STUB_INT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_textureUploadData);
STUB_INT(Java_com_adobe_ttpixel_extension_ss_ECScribbleSegment_getProgress);
STUB_INT(Java_com_adobe_ttpixel_extension_ss_ECScribbleSegment_getResult);
STUB_INT(Java_com_adobe_ttpixel_extension_ss_ECScribbleSegment_init);
STUB_INT(Java_com_adobe_ttpixel_extension_ss_ECScribbleSegment_release);
STUB_INT(Java_com_adobe_ttpixel_extension_ss_ECScribbleSegment_run);
STUB_INT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedDataSize);
STUB_INT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodingProgress);
STUB_INT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_waitFinishedEncoding);
STUB_INT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1getButtonState);
STUB_INT(Java_com_adobe_ttpixel_extension_utils_ECUtils_isolateColor);
STUB_INT(Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4GetMaxCompressDestLength);
STUB_INT(Java_com_adobe_ttpixel_extension_utils_FnCompressBitmapRLE_nativeCompressBitmapRLE);
STUB_LONG(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_create);
STUB_LONG(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1createButtonEventsQueue);
STUB_LONG(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1createPressureEventsQueue);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_outputToString);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_gl_ECGLContext_getStringInfo);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_getShaderInfoLog);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_AIRRuntimeHelper_createCompatibleBitmap);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_AIRRuntimeHelper_updateCompatibleBitmap);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Compress);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Uncompress);
STUB_VOID(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_dispose);
STUB_VOID(Java_com_adobe_ttpixel_extension_httpd_Httpd_stop);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtension_initIDs);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_premultiplyData);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_requestCancel);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_unPremultiplyData);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1destroyButtonEventsQueue);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1destroyPressureEventsQueue);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1setButtonState);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1setPressure);
STUB_VOID(Java_com_adobe_ttpixel_extension_utils_ECUtils_getScaledPixelsEx);
STUB_VOID(Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Free);
