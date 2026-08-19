# ANE compatibility layer (experimental)

This directory contains a clean-room ARMv7 compatibility layer for the JNI
surface exported by the old `TTPixelExtensionAndroid` library.

The original Adobe library contains `DT_TEXTREL` relocations and is rejected by
recent Android linkers. The replacement is compiled as PIC with the Android NDK
and exports the same 119 JNI entry points, allowing the AIR 51 runtime and the
ActionScript extension bootstrap to continue.

The image-processing, encoder, camera, and synchronization methods are
fail-soft placeholders or small buffer helpers. This is not a reimplementation
of Adobe's algorithms and does not yet establish editor functionality.

## Build

With NDK 25.2 installed:

```powershell
& "$env:ANDROID_HOME\ndk\25.2.9519653\ndk-build.cmd" `
  NDK_PROJECT_PATH="$PWD\native\ane-compat" `
  APP_BUILD_SCRIPT="$PWD\native\ane-compat\Android.mk" `
  NDK_APPLICATION_MK="$PWD\native\ane-compat\Application.mk"
```

The output is `libs/armeabi-v7a/`. The generated `.so` files are deliberately
ignored by Git; only source and build metadata belong in the repository.

