#include <jni.h>
#include <android/log.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#define TTPIXEL_TRACE_TAG "TTPixelCompat"

static void trace_call(const char *name) {
    __android_log_print(ANDROID_LOG_INFO, TTPIXEL_TRACE_TAG, "native call: %s", name);
}

__attribute__((constructor))
static void trace_library_load(void) {
    __android_log_print(ANDROID_LOG_INFO, TTPIXEL_TRACE_TAG, "TTPixel compatibility library loaded");
}

static uint8_t *direct_bytes(JNIEnv *env, jobject buffer, jlong *capacity) {
    if (buffer == NULL) {
        return NULL;
    }
    jlong size = (*env)->GetDirectBufferCapacity(env, buffer);
    void *address = (*env)->GetDirectBufferAddress(env, buffer);
    if (size <= 0 || address == NULL) {
        return NULL;
    }
    if (capacity != NULL) {
        *capacity = size;
    }
    return (uint8_t *)address;
}

static int valid_region(jlong capacity, int stride, int x, int y, int width, int height) {
    if (stride < 0 || x < 0 || y < 0 || width < 0 || height < 0) {
        return 0;
    }
    if ((int64_t)y * stride > capacity || (int64_t)height * stride > capacity - (int64_t)y * stride) {
        return 0;
    }
    if ((int64_t)x * 4 + (int64_t)width * 4 > stride) {
        return 0;
    }
    return 1;
}

static uint8_t clamp_u8(int value) {
    return (uint8_t)(value < 0 ? 0 : value > 255 ? 255 : value);
}

static void premultiply_pixel(uint8_t *pixel) {
    int alpha = pixel[3];
    pixel[0] = (uint8_t)((pixel[0] * alpha + 127) / 255);
    pixel[1] = (uint8_t)((pixel[1] * alpha + 127) / 255);
    pixel[2] = (uint8_t)((pixel[2] * alpha + 127) / 255);
}

static void unpremultiply_pixel(uint8_t *pixel) {
    int alpha = pixel[3];
    if (alpha == 0) {
        pixel[0] = pixel[1] = pixel[2] = 0;
        return;
    }
    pixel[0] = clamp_u8((pixel[0] * 255 + alpha / 2) / alpha);
    pixel[1] = clamp_u8((pixel[1] * 255 + alpha / 2) / alpha);
    pixel[2] = clamp_u8((pixel[2] * 255 + alpha / 2) / alpha);
}

static void copy_pixels(uint8_t *dst, int dst_stride, int dst_x, int dst_y,
        const uint8_t *src, int src_stride, int src_x, int src_y,
        int width, int height, int premultiplied) {
    for (int row = 0; row < height; ++row) {
        const uint8_t *source = src + (size_t)(src_y + row) * src_stride + (size_t)src_x * 4;
        uint8_t *target = dst + (size_t)(dst_y + row) * dst_stride + (size_t)dst_x * 4;
        memmove(target, source, (size_t)width * 4);
        if (premultiplied) {
            for (int column = 0; column < width; ++column) {
                premultiply_pixel(target + column * 4);
            }
        }
    }
}

/*
 * Clean-room compatibility layer for the obsolete TTPixel JNI surface.
 *
 * The original library cannot be loaded by current Android linkers because it
 * contains text relocations. The implemented entries below cover the first
 * bitmap-buffer and import/export contract; the remaining entries stay
 * explicit fail-soft stubs until their SWF call sites are observed.
 */

#define STUB_VOID(symbol) \
    JNIEXPORT void JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
    }

#define STUB_BOOL(symbol) \
    JNIEXPORT jboolean JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return JNI_FALSE; \
    }

#define STUB_INT(symbol) \
    JNIEXPORT jint JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return 0; \
    }

#define STUB_LONG(symbol) \
    JNIEXPORT jlong JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return 0; \
    }

#define STUB_FLOAT(symbol) \
    JNIEXPORT jfloat JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return 0.0f; \
    }

#define STUB_DOUBLE(symbol) \
    JNIEXPORT jdouble JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return 0.0; \
    }

#define STUB_OBJECT(symbol) \
    JNIEXPORT jobject JNICALL symbol(JNIEnv *env, jclass clazz, ...) { \
        (void)env; (void)clazz; \
        trace_call(#symbol); \
        return NULL; \
    }

JNIEXPORT jobject JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_allocateDirect(
        JNIEnv *env, jclass clazz, jint capacity) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_ByteBufferFactory_allocateDirect");
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
    trace_call("Java_com_adobe_ttpixel_extension_ByteBufferFactory_freeDirect");
    if (buffer != NULL) {
        void *memory = (*env)->GetDirectBufferAddress(env, buffer);
        free(memory);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_ByteBufferFactory_copyDirect(
        JNIEnv *env, jclass clazz, jobject source, jobject destination) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_ByteBufferFactory_copyDirect");
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
    trace_call("Java_com_adobe_ttpixel_extension_ByteBufferFactory_copySwapColorChannels");
    return Java_com_adobe_ttpixel_extension_ByteBufferFactory_copyDirect(
            env, clazz, source, destination);
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataCopy(
        JNIEnv *env, jclass clazz, jobject source, jint source_stride,
        jobject destination, jint destination_stride, jint source_x, jint source_y,
        jint width, jint height, jint destination_x, jint destination_y) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataCopy");
    jlong source_capacity = 0;
    jlong destination_capacity = 0;
    uint8_t *source_bytes = direct_bytes(env, source, &source_capacity);
    uint8_t *destination_bytes = direct_bytes(env, destination, &destination_capacity);
    if (source_bytes == NULL || destination_bytes == NULL ||
            !valid_region(source_capacity, source_stride, source_x, source_y, width, height) ||
            !valid_region(destination_capacity, destination_stride, destination_x, destination_y,
                    width, height)) {
        return JNI_FALSE;
    }
    copy_pixels(destination_bytes, destination_stride, destination_x, destination_y,
            source_bytes, source_stride,
            source_x, source_y, width, height, 0);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getPixelsEx(
        JNIEnv *env, jclass clazz, jobject output, jobject bitmap, jint bitmap_stride,
        jint x, jint y, jint width, jint height, jboolean premultiplied) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_getPixelsEx");
    jlong output_capacity = 0;
    jlong bitmap_capacity = 0;
    uint8_t *output_bytes = direct_bytes(env, output, &output_capacity);
    uint8_t *bitmap_bytes = direct_bytes(env, bitmap, &bitmap_capacity);
    if (output_bytes == NULL || bitmap_bytes == NULL || width < 0 || height < 0 ||
            !valid_region(bitmap_capacity, bitmap_stride, x, y, width, height) ||
            (int64_t)width * height * 4 > output_capacity) {
        return JNI_FALSE;
    }
    copy_pixels(output_bytes, width * 4, 0, 0, bitmap_bytes, bitmap_stride,
            x, y, width, height, 0);
    if (premultiplied) {
        for (int row = 0; row < height; ++row) {
            for (int column = 0; column < width; ++column) {
                unpremultiply_pixel(output_bytes + ((size_t)row * width + column) * 4);
            }
        }
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_setPixelsEx(
        JNIEnv *env, jclass clazz, jobject bitmap, jobject input, jint bitmap_width,
        jint bitmap_height, jint bitmap_stride, jint x, jint y, jint width, jint height,
        jboolean premultiplied) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_setPixelsEx");
    jlong bitmap_capacity = 0;
    jlong input_capacity = 0;
    uint8_t *bitmap_bytes = direct_bytes(env, bitmap, &bitmap_capacity);
    uint8_t *input_bytes = direct_bytes(env, input, &input_capacity);
    if (bitmap_bytes == NULL || input_bytes == NULL || bitmap_width < 0 || bitmap_height < 0 ||
            !valid_region(bitmap_capacity, bitmap_stride, x, y, width, height) ||
            (int64_t)width * height * 4 > input_capacity) {
        return JNI_FALSE;
    }
    copy_pixels(bitmap_bytes, bitmap_stride, x, y, input_bytes, width * 4,
            0, 0, width, height, 0);
    if (premultiplied) {
        for (int row = 0; row < height; ++row) {
            for (int column = 0; column < width; ++column) {
                premultiply_pixel(bitmap_bytes + (size_t)(y + row) * bitmap_stride + (size_t)(x + column) * 4);
            }
        }
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_moveBitmapDataEx(
        JNIEnv *env, jclass clazz, jobject bitmap, jint stride, jint source_x, jint source_y,
        jint width, jint height, jint destination_x, jint destination_y) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_moveBitmapDataEx");
    jlong capacity = 0;
    uint8_t *bytes = direct_bytes(env, bitmap, &capacity);
    if (bytes == NULL || !valid_region(capacity, stride, source_x, source_y, width, height) ||
            !valid_region(capacity, stride, destination_x, destination_y, width, height)) {
        return JNI_FALSE;
    }
    if (destination_y > source_y) {
        for (int row = height - 1; row >= 0; --row) {
            memmove(bytes + (size_t)(destination_y + row) * stride + (size_t)destination_x * 4,
                    bytes + (size_t)(source_y + row) * stride + (size_t)source_x * 4,
                    (size_t)width * 4);
        }
    } else {
        for (int row = 0; row < height; ++row) {
            memmove(bytes + (size_t)(destination_y + row) * stride + (size_t)destination_x * 4,
                    bytes + (size_t)(source_y + row) * stride + (size_t)source_x * 4,
                    (size_t)width * 4);
        }
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataResample(
        JNIEnv *env, jclass clazz, jobject source, jint source_stride, jobject destination,
        jint destination_stride, jint source_x, jint source_y, jint source_width, jint source_height,
        jint destination_x, jint destination_y, jint destination_width, jint destination_height) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataResample");
    jlong source_capacity = 0;
    jlong destination_capacity = 0;
    uint8_t *source_bytes = direct_bytes(env, source, &source_capacity);
    uint8_t *destination_bytes = direct_bytes(env, destination, &destination_capacity);
    if (source_bytes == NULL || destination_bytes == NULL || source_width <= 0 || source_height <= 0 ||
            destination_width <= 0 || destination_height <= 0 ||
            !valid_region(source_capacity, source_stride, source_x, source_y, source_width, source_height) ||
            !valid_region(destination_capacity, destination_stride, destination_x, destination_y,
                    destination_width, destination_height)) {
        return JNI_FALSE;
    }
    for (int y = 0; y < destination_height; ++y) {
        int source_row = source_y + (int)((int64_t)y * source_height / destination_height);
        uint8_t *target = destination_bytes + (size_t)(destination_y + y) * destination_stride + (size_t)destination_x * 4;
        const uint8_t *row = source_bytes + (size_t)source_row * source_stride;
        for (int x = 0; x < destination_width; ++x) {
            int source_column = source_x + (int)((int64_t)x * source_width / destination_width);
            memcpy(target + x * 4, row + source_column * 4, 4);
        }
    }
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getScaledPixelsEx(
        JNIEnv *env, jclass clazz, jobject source, jint source_width, jint source_height,
        jint source_stride, jint source_y, jint source_x, jint source_bottom, jint source_right,
        jobject destination, jint destination_stride, jint destination_width, jint destination_height,
        jint premultiplied) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_getScaledPixelsEx");
    jlong source_capacity = 0;
    jlong destination_capacity = 0;
    uint8_t *source_bytes = direct_bytes(env, source, &source_capacity);
    uint8_t *destination_bytes = direct_bytes(env, destination, &destination_capacity);
    int source_rect_width = source_right - source_x;
    int source_rect_height = source_bottom - source_y;
    if (source_bytes == NULL || destination_bytes == NULL || source_width <= 0 || source_height <= 0 ||
            source_rect_width <= 0 || source_rect_height <= 0 || destination_width <= 0 || destination_height <= 0 ||
            !valid_region(source_capacity, source_stride, source_x, source_y, source_rect_width, source_rect_height) ||
            !valid_region(destination_capacity, destination_stride, 0, 0,
                    destination_width, destination_height)) {
        return;
    }
    for (int y = 0; y < destination_height; ++y) {
        int source_row = source_y + (int)((int64_t)y * source_rect_height / destination_height);
        uint8_t *target = destination_bytes + (size_t)y * destination_stride;
        const uint8_t *row = source_bytes + (size_t)source_row * source_stride;
        for (int x = 0; x < destination_width; ++x) {
            int source_column = source_x + (int)((int64_t)x * source_rect_width / destination_width);
            memcpy(target + x * 4, row + source_column * 4, 4);
            if (premultiplied) {
                premultiply_pixel(target + x * 4);
            }
        }
    }
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_uncompressBitmapDataEx(
        JNIEnv *env, jclass clazz, jobject compressed, jint compressed_length,
        jobject destination, jint width, jint height) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_uncompressBitmapDataEx");
    jlong compressed_capacity = 0;
    jlong destination_capacity = 0;
    uint8_t *compressed_bytes = direct_bytes(env, compressed, &compressed_capacity);
    uint8_t *destination_bytes = direct_bytes(env, destination, &destination_capacity);
    uLongf output_length = (uLongf)((int64_t)width * height * 4);
    if (compressed_bytes == NULL || destination_bytes == NULL || compressed_length < 0 ||
            compressed_length > compressed_capacity || output_length > destination_capacity) {
        return JNI_FALSE;
    }
    return uncompress(destination_bytes, &output_length, compressed_bytes, (uLong)compressed_length) == Z_OK
            ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4GetMaxCompressDestLength(
        JNIEnv *env, jclass clazz, jint input_length) {
    (void)env; (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4GetMaxCompressDestLength");
    if (input_length < 0) return 0;
    return input_length + input_length / 255 + 16;
}

static int compress_lz4_buffer(const uint8_t *input, size_t input_length,
        uint8_t **encoded_out, size_t *encoded_length_out);

JNIEXPORT jobject JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Compress(
        JNIEnv *env, jclass clazz, jobject input, jobject destination, jboolean high_compression) {
    (void)clazz;
    (void)high_compression;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Compress");
    jlong input_capacity = 0;
    uint8_t *input_bytes = direct_bytes(env, input, &input_capacity);
    uint8_t *encoded = NULL;
    size_t encoded_length = 0;
    if (input_bytes == NULL || !compress_lz4_buffer(input_bytes, (size_t)input_capacity,
            &encoded, &encoded_length)) return NULL;

    jobject compressed_buffer = destination;
    int owns_buffer = 0;
    if (compressed_buffer != NULL) {
        jlong destination_capacity = (*env)->GetDirectBufferCapacity(env, compressed_buffer);
        uint8_t *destination_bytes = (uint8_t *)(*env)->GetDirectBufferAddress(env, compressed_buffer);
        if (destination_bytes == NULL || destination_capacity < (jlong)encoded_length) {
            free(encoded);
            return NULL;
        }
        memcpy(destination_bytes, encoded, encoded_length);
        free(encoded);
    } else {
        compressed_buffer = (*env)->NewDirectByteBuffer(env, encoded, (jlong)encoded_length);
        if (compressed_buffer == NULL) {
            free(encoded);
            return NULL;
        }
        owns_buffer = 1;
    }

    jclass result_class = (*env)->FindClass(env, "com/adobe/ttpixel/extension/utils/Lz4CompressResult");
    if (result_class == NULL) {
        if (owns_buffer) free(encoded);
        if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
        return NULL;
    }
    jmethodID constructor = (*env)->GetMethodID(env, result_class, "<init>",
            "(ILjava/nio/ByteBuffer;)V");
    jobject result = NULL;
    if (constructor != NULL) {
        result = (*env)->NewObject(env, result_class, constructor, (jint)encoded_length, compressed_buffer);
    }
    if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, result_class);
    if (destination == NULL && result == NULL) {
        free(encoded);
    }
    return result;
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Free(JNIEnv *env, jclass clazz, jobject buffer) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Free");
    if (buffer != NULL) {
        free((*env)->GetDirectBufferAddress(env, buffer));
    }
}

JNIEXPORT jobject JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Uncompress(
        JNIEnv *env, jclass clazz, jobject compressed, jobject destination) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_lz4Uncompress");
    jlong compressed_capacity = 0;
    jlong destination_capacity = 0;
    uint8_t *input = direct_bytes(env, compressed, &compressed_capacity);
    uint8_t *output = direct_bytes(env, destination, &destination_capacity);
    if (input == NULL || output == NULL) return NULL;
    size_t input_position = 0;
    size_t output_position = 0;
    while (input_position < (size_t)compressed_capacity) {
        uint8_t token = input[input_position++];
        size_t literal_length = token >> 4;
        if (literal_length == 15) {
            uint8_t length_byte;
            do {
                if (input_position >= (size_t)compressed_capacity) return NULL;
                length_byte = input[input_position++];
                literal_length += length_byte;
            } while (length_byte == 255);
        }
        if (input_position + literal_length > (size_t)compressed_capacity ||
                output_position + literal_length > (size_t)destination_capacity) return NULL;
        memcpy(output + output_position, input + input_position, literal_length);
        input_position += literal_length;
        output_position += literal_length;
        if (input_position >= (size_t)compressed_capacity) break;
        if (input_position + 2 > (size_t)compressed_capacity) return NULL;
        size_t offset = input[input_position] | ((size_t)input[input_position + 1] << 8);
        input_position += 2;
        if (offset == 0 || offset > output_position) return NULL;
        size_t match_length = token & 0x0f;
        if (match_length == 15) {
            uint8_t length_byte;
            do {
                if (input_position >= (size_t)compressed_capacity) return NULL;
                length_byte = input[input_position++];
                match_length += length_byte;
            } while (length_byte == 255);
        }
        match_length += 4;
        if (output_position + match_length > (size_t)destination_capacity) return NULL;
        for (size_t i = 0; i < match_length; ++i) {
            output[output_position + i] = output[output_position - offset + i];
        }
        output_position += match_length;
    }
    return destination;
}

typedef struct {
    uint8_t *encoded;
    size_t encoded_length;
    jint progress;
    int finished;
    int cancelled;
} EncoderState;

static jfieldID exporter_field(JNIEnv *env, jobject context) {
    if (context == NULL) return NULL;
    jclass context_class = (*env)->GetObjectClass(env, context);
    if (context_class == NULL) return NULL;
    jfieldID field = (*env)->GetFieldID(env, context_class, "exporterPtr", "J");
    (*env)->DeleteLocalRef(env, context_class);
    if (field == NULL && (*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }
    return field;
}

static EncoderState *get_encoder_state(JNIEnv *env, jobject context) {
    jfieldID field = exporter_field(env, context);
    if (field == NULL) return NULL;
    return (EncoderState *)(uintptr_t)(*env)->GetLongField(env, context, field);
}

static void free_encoder_state(EncoderState *state) {
    if (state == NULL) return;
    free(state->encoded);
    free(state);
}

static int set_encoder_state(JNIEnv *env, jobject context, uint8_t *encoded, size_t encoded_length) {
    jfieldID field = exporter_field(env, context);
    if (field == NULL) {
        free(encoded);
        return 0;
    }
    EncoderState *old_state = (EncoderState *)(uintptr_t)(*env)->GetLongField(env, context, field);
    free_encoder_state(old_state);
    EncoderState *state = (EncoderState *)calloc(1, sizeof(EncoderState));
    if (state == NULL) {
        free(encoded);
        (*env)->SetLongField(env, context, field, (jlong)0);
        return 0;
    }
    state->encoded = encoded;
    state->encoded_length = encoded_length;
    state->progress = 100;
    state->finished = 1;
    (*env)->SetLongField(env, context, field, (jlong)(uintptr_t)state);
    return 1;
}

static void dispatch_encoding_complete(JNIEnv *env, jobject context) {
    if (context == NULL) return;
    jclass context_class = (*env)->GetObjectClass(env, context);
    if (context_class == NULL) return;
    jmethodID progress = (*env)->GetMethodID(env, context_class, "onEncodingProgress", "(I)Z");
    if (progress == NULL && (*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    jmethodID finished = (*env)->GetMethodID(env, context_class, "onEncodingFinished", "(I)V");
    if (finished == NULL && (*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    if (progress != NULL) {
        (*env)->CallBooleanMethod(env, context, progress, 100);
        if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    }
    if (finished != NULL) {
        (*env)->CallVoidMethod(env, context, finished, 0);
        if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, context_class);
}

static int install_encoded_result(JNIEnv *env, jobject context, uint8_t *encoded, size_t encoded_length) {
    if (!set_encoder_state(env, context, encoded, encoded_length)) return 0;
    dispatch_encoding_complete(env, context);
    return 1;
}

static int compress_zlib_buffer(const uint8_t *input, size_t input_length,
        uint8_t **encoded_out, size_t *encoded_length_out) {
    if (input == NULL || encoded_out == NULL || encoded_length_out == NULL ||
            input_length > (size_t)ULONG_MAX) {
        return 0;
    }
    uLongf bound = compressBound((uLong)input_length);
    uint8_t *encoded = (uint8_t *)malloc((size_t)bound);
    if (encoded == NULL) return 0;
    uLongf encoded_length = bound;
    int result = compress2(encoded, &encoded_length, input, (uLong)input_length, Z_DEFAULT_COMPRESSION);
    if (result != Z_OK) {
        free(encoded);
        return 0;
    }
    *encoded_out = encoded;
    *encoded_length_out = (size_t)encoded_length;
    return 1;
}

/* Literal-only LZ4 block encoder. It is deliberately simple but emits a valid
 * LZ4 block accepted by the matching decoder and by standard LZ4 decoders. */
static int compress_lz4_buffer(const uint8_t *input, size_t input_length,
        uint8_t **encoded_out, size_t *encoded_length_out) {
    if (input == NULL || encoded_out == NULL || encoded_length_out == NULL) return 0;
    size_t extra = input_length / 255 + 32;
    if (input_length > SIZE_MAX - extra) return 0;
    uint8_t *encoded = (uint8_t *)malloc(input_length + extra);
    if (encoded == NULL) return 0;
    size_t position = 0;
    size_t literal_length = input_length;
    encoded[position++] = (uint8_t)((literal_length < 15 ? literal_length : 15) << 4);
    if (literal_length >= 15) {
        size_t remaining = literal_length - 15;
        while (remaining >= 255) {
            encoded[position++] = 255;
            remaining -= 255;
        }
        encoded[position++] = (uint8_t)remaining;
    }
    if (input_length > 0) {
        memcpy(encoded + position, input, input_length);
        position += input_length;
    }
    *encoded_out = encoded;
    *encoded_length_out = position;
    return 1;
}

static int encode_bitmap_with_android(JNIEnv *env, jobject pixels, jint width, jint height,
        jboolean has_alpha, jboolean premultiplied, jint stride, int jpeg, int quality,
        uint8_t **encoded_out, size_t *encoded_length_out) {
    (void)has_alpha;
    if (pixels == NULL || encoded_out == NULL || encoded_length_out == NULL || width <= 0 ||
            height <= 0 || stride < width * 4) {
        return 0;
    }
    jlong source_capacity = 0;
    uint8_t *source = direct_bytes(env, pixels, &source_capacity);
    if (source == NULL || !valid_region(source_capacity, stride, 0, 0, width, height)) return 0;

    size_t packed_length = (size_t)width * (size_t)height * 4;
    uint8_t *packed = (uint8_t *)malloc(packed_length);
    if (packed == NULL) return 0;
    for (int row = 0; row < height; ++row) {
        memcpy(packed + (size_t)row * (size_t)width * 4,
                source + (size_t)row * (size_t)stride, (size_t)width * 4);
    }

    int ok = 0;
    jobject packed_buffer = (*env)->NewDirectByteBuffer(env, packed, (jlong)packed_length);
    jclass bitmap_class = NULL;
    jclass config_class = NULL;
    jclass format_class = NULL;
    jclass output_class = NULL;
    jobject config = NULL;
    jobject format = NULL;
    jobject bitmap = NULL;
    jobject output = NULL;
    jbyteArray bytes = NULL;
    if (packed_buffer == NULL) goto cleanup;

    bitmap_class = (*env)->FindClass(env, "android/graphics/Bitmap");
    config_class = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
    format_class = (*env)->FindClass(env, "android/graphics/Bitmap$CompressFormat");
    output_class = (*env)->FindClass(env, "java/io/ByteArrayOutputStream");
    if (bitmap_class == NULL || config_class == NULL || format_class == NULL || output_class == NULL) goto cleanup;

    jfieldID argb_field = (*env)->GetStaticFieldID(env, config_class, "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;");
    jfieldID format_field = (*env)->GetStaticFieldID(env, format_class,
            jpeg ? "JPEG" : "PNG", "Landroid/graphics/Bitmap$CompressFormat;");
    jmethodID create_bitmap = (*env)->GetStaticMethodID(env, bitmap_class, "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jmethodID copy_pixels = (*env)->GetMethodID(env, bitmap_class, "copyPixelsFromBuffer",
            "(Ljava/nio/Buffer;)V");
    jmethodID set_premultiplied = (*env)->GetMethodID(env, bitmap_class, "setPremultiplied", "(Z)V");
    if (set_premultiplied == NULL && (*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    jmethodID output_ctor = (*env)->GetMethodID(env, output_class, "<init>", "()V");
    jmethodID output_bytes = (*env)->GetMethodID(env, output_class, "toByteArray", "()[B");
    jmethodID compress = (*env)->GetMethodID(env, bitmap_class, "compress",
            "(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z");
    if (argb_field == NULL || format_field == NULL || create_bitmap == NULL || copy_pixels == NULL ||
            output_ctor == NULL || output_bytes == NULL || compress == NULL) goto cleanup;

    config = (*env)->GetStaticObjectField(env, config_class, argb_field);
    format = (*env)->GetStaticObjectField(env, format_class, format_field);
    bitmap = (*env)->CallStaticObjectMethod(env, bitmap_class, create_bitmap, width, height, config);
    if ((*env)->ExceptionCheck(env) || bitmap == NULL) goto cleanup;
    (*env)->CallVoidMethod(env, bitmap, copy_pixels, packed_buffer);
    if ((*env)->ExceptionCheck(env)) goto cleanup;
    if (set_premultiplied != NULL) {
        (*env)->CallVoidMethod(env, bitmap, set_premultiplied, premultiplied ? JNI_TRUE : JNI_FALSE);
        if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    }
    output = (*env)->NewObject(env, output_class, output_ctor);
    if ((*env)->ExceptionCheck(env) || output == NULL) goto cleanup;
    int bounded_quality = quality < 0 ? 0 : quality > 100 ? 100 : quality;
    if (!(*env)->CallBooleanMethod(env, bitmap, compress, format, bounded_quality, output) ||
            (*env)->ExceptionCheck(env)) goto cleanup;
    bytes = (jbyteArray)(*env)->CallObjectMethod(env, output, output_bytes);
    if ((*env)->ExceptionCheck(env) || bytes == NULL) goto cleanup;
    jsize byte_length = (*env)->GetArrayLength(env, bytes);
    uint8_t *encoded = (uint8_t *)malloc((size_t)byte_length);
    if (encoded == NULL) goto cleanup;
    (*env)->GetByteArrayRegion(env, bytes, 0, byte_length, (jbyte *)encoded);
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
        free(encoded);
        goto cleanup;
    }
    *encoded_out = encoded;
    *encoded_length_out = (size_t)byte_length;
    ok = 1;

cleanup:
    if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    if (bytes != NULL) (*env)->DeleteLocalRef(env, bytes);
    if (output != NULL) (*env)->DeleteLocalRef(env, output);
    if (bitmap != NULL) (*env)->DeleteLocalRef(env, bitmap);
    if (format != NULL) (*env)->DeleteLocalRef(env, format);
    if (config != NULL) (*env)->DeleteLocalRef(env, config);
    if (output_class != NULL) (*env)->DeleteLocalRef(env, output_class);
    if (format_class != NULL) (*env)->DeleteLocalRef(env, format_class);
    if (config_class != NULL) (*env)->DeleteLocalRef(env, config_class);
    if (bitmap_class != NULL) (*env)->DeleteLocalRef(env, bitmap_class);
    if (packed_buffer != NULL) (*env)->DeleteLocalRef(env, packed_buffer);
    free(packed);
    return ok;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeLz4(
        JNIEnv *env, jclass clazz, jobject context, jobject input) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeLz4");
    jlong input_capacity = 0;
    uint8_t *input_bytes = direct_bytes(env, input, &input_capacity);
    uint8_t *encoded = NULL;
    size_t encoded_length = 0;
    if (input_bytes == NULL || !compress_lz4_buffer(input_bytes, (size_t)input_capacity,
            &encoded, &encoded_length)) return JNI_FALSE;
    return install_encoded_result(env, context, encoded, encoded_length) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeZLib(
        JNIEnv *env, jclass clazz, jobject context, jobject input) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeZLib");
    jlong input_capacity = 0;
    uint8_t *input_bytes = direct_bytes(env, input, &input_capacity);
    uint8_t *encoded = NULL;
    size_t encoded_length = 0;
    if (input_bytes == NULL || !compress_zlib_buffer(input_bytes, (size_t)input_capacity,
            &encoded, &encoded_length)) return JNI_FALSE;
    return install_encoded_result(env, context, encoded, encoded_length) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodePNG(
        JNIEnv *env, jclass clazz, jobject context, jint width, jint height,
        jboolean has_alpha, jboolean premultiplied, jint stride, jobject pixels) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodePNG");
    uint8_t *encoded = NULL;
    size_t encoded_length = 0;
    if (!encode_bitmap_with_android(env, pixels, width, height, has_alpha, premultiplied,
            stride, 0, 100, &encoded, &encoded_length)) return JNI_FALSE;
    return install_encoded_result(env, context, encoded, encoded_length) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeJPEG(
        JNIEnv *env, jclass clazz, jobject context, jint width, jint height,
        jboolean has_alpha, jboolean premultiplied, jint stride, jobject pixels,
        jfloat quality, jstring path) {
    (void)clazz;
    (void)path;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_startEncodeJPEG");
    uint8_t *encoded = NULL;
    size_t encoded_length = 0;
    int quality_percent = (int)(quality * 100.0f + 0.5f);
    if (!encode_bitmap_with_android(env, pixels, width, height, has_alpha, premultiplied,
            stride, 1, quality_percent, &encoded, &encoded_length)) return JNI_FALSE;
    return install_encoded_result(env, context, encoded, encoded_length) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedDataSize(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedDataSize");
    EncoderState *state = get_encoder_state(env, context);
    return state == NULL ? 0 : (jint)state->encoded_length;
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodingProgress(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodingProgress");
    EncoderState *state = get_encoder_state(env, context);
    return state == NULL ? 0 : state->progress;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_hasFinishedEncoding(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_hasFinishedEncoding");
    EncoderState *state = get_encoder_state(env, context);
    return state != NULL && state->finished ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedData(
        JNIEnv *env, jclass clazz, jobject context, jobject output) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_getEncodedData");
    EncoderState *state = get_encoder_state(env, context);
    jlong output_capacity = 0;
    uint8_t *output_bytes = direct_bytes(env, output, &output_capacity);
    if (state == NULL || output_bytes == NULL || state->encoded_length > (size_t)output_capacity) {
        return JNI_FALSE;
    }
    if (state->encoded_length > 0) memcpy(output_bytes, state->encoded, state->encoded_length);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_clearEncodedData(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_clearEncodedData");
    jfieldID field = exporter_field(env, context);
    if (field == NULL) return JNI_FALSE;
    EncoderState *state = (EncoderState *)(uintptr_t)(*env)->GetLongField(env, context, field);
    free_encoder_state(state);
    (*env)->SetLongField(env, context, field, (jlong)0);
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_requestCancel(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_requestCancel");
    EncoderState *state = get_encoder_state(env, context);
    if (state != NULL) state->cancelled = 1;
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_waitFinishedEncoding(
        JNIEnv *env, jclass clazz, jobject context) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_waitFinishedEncoding");
    EncoderState *state = get_encoder_state(env, context);
    if (state == NULL) return -1;
    return state->cancelled ? -1 : 0;
}

JNIEXPORT jboolean JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_isPossiblyPremultipliedData(
        JNIEnv *env, jclass clazz, jint width, jint height, jint stride, jobject pixels) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_isPossiblyPremultipliedData");
    jlong capacity = 0;
    uint8_t *bytes = direct_bytes(env, pixels, &capacity);
    if (bytes == NULL || width < 0 || height < 0 ||
            !valid_region(capacity, stride, 0, 0, width, height)) return JNI_FALSE;
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            const uint8_t *pixel = bytes + (size_t)row * stride + (size_t)column * 4;
            if (pixel[0] > pixel[3] || pixel[1] > pixel[3] || pixel[2] > pixel[3]) return JNI_FALSE;
        }
    }
    return JNI_TRUE;
}

static void transform_pixels(jobject pixels, JNIEnv *env, jint width, jint height, jint stride,
        int unpremultiply) {
    jlong capacity = 0;
    uint8_t *bytes = direct_bytes(env, pixels, &capacity);
    if (bytes == NULL || width < 0 || height < 0 ||
            !valid_region(capacity, stride, 0, 0, width, height)) return;
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            uint8_t *pixel = bytes + (size_t)row * stride + (size_t)column * 4;
            if (unpremultiply) unpremultiply_pixel(pixel);
            else premultiply_pixel(pixel);
        }
    }
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_premultiplyData(
        JNIEnv *env, jclass clazz, jint width, jint height, jint stride, jobject pixels) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_premultiplyData");
    transform_pixels(pixels, env, width, height, stride, 0);
}

JNIEXPORT void JNICALL
Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_unPremultiplyData(
        JNIEnv *env, jclass clazz, jint width, jint height, jint stride, jobject pixels) {
    (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_TTPixelExtensionContextImpExp_unPremultiplyData");
    transform_pixels(pixels, env, width, height, stride, 1);
}

STUB_BOOL(Java_com_adobe_ttpixel_extension_ByteBufferFactory_flipImageHor);

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getInstalledCPUCount(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_getInstalledCPUCount");
    long count = sysconf(_SC_NPROCESSORS_CONF);
    return (jint)(count > 0 ? count : 1);
}

JNIEXPORT jint JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getOnlineCPUCount(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_getOnlineCPUCount");
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    return (jint)(count > 0 ? count : 1);
}

JNIEXPORT jdouble JNICALL
Java_com_adobe_ttpixel_extension_utils_ECUtils_getTimestamp(
        JNIEnv *env, jclass clazz) {
    (void)env; (void)clazz;
    trace_call("Java_com_adobe_ttpixel_extension_utils_ECUtils_getTimestamp");
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (jdouble)now.tv_sec + ((jdouble)now.tv_nsec / 1000000000.0);
}

STUB_BOOL(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_perform);
STUB_BOOL(Java_com_adobe_ttpixel_extension_httpd_Httpd_modifyPasswdFile);
STUB_BOOL(Java_com_adobe_ttpixel_extension_httpd_Httpd_start);
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
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataFromFileEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapDataToFileEx);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileCreateEmpty);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileCreateFromBitmapData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileRead);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileResample);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_bitmapFileWrite);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_copyBitmapData);
STUB_BOOL(Java_com_adobe_ttpixel_extension_utils_ECUtils_getPixelsBitmapEx);
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
STUB_INT(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1getButtonState);
STUB_INT(Java_com_adobe_ttpixel_extension_utils_ECUtils_isolateColor);
STUB_INT(Java_com_adobe_ttpixel_extension_utils_FnCompressBitmapRLE_nativeCompressBitmapRLE);
STUB_LONG(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_create);
STUB_LONG(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1createButtonEventsQueue);
STUB_LONG(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1createPressureEventsQueue);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_outputToString);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_gl_ECGLContext_getStringInfo);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_gl_ECGLFilter_getShaderInfoLog);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_AIRRuntimeHelper_createCompatibleBitmap);
STUB_OBJECT(Java_com_adobe_ttpixel_extension_utils_AIRRuntimeHelper_updateCompatibleBitmap);
STUB_VOID(Java_com_adobe_ttpixel_extension_bigdata_ECBitmapPreflight_dispose);
STUB_VOID(Java_com_adobe_ttpixel_extension_httpd_Httpd_stop);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtension_initIDs);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1destroyButtonEventsQueue);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1destroyPressureEventsQueue);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1setButtonState);
STUB_VOID(Java_com_adobe_ttpixel_extension_TTPixelExtensionContextPressureJaJa_native_1setPressure);
