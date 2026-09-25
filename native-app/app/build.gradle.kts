plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.lordphilly.pstouch.nativeapp"
    compileSdk = 36

    defaultConfig {
        applicationId = "com.lordphilly.pstouch.nativeapp"
        minSdk = 29
        targetSdk = 36
        versionCode = 1
        versionName = "0.1.0-native"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }
}
