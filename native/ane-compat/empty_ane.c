#include <jni.h>

/* Minimal PIC library for an ANE whose Java/ActionScript side is optional. */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)vm;
    (void)reserved;
    return JNI_VERSION_1_6;
}
