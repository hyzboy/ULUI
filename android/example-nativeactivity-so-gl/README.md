# Example 1: NativeActivity - OpenGL Initialized by .so

This example demonstrates using Android NativeActivity where the OpenGL ES context is created and managed entirely by the native .so library.

## Overview

- **Activity Type**: NativeActivity
- **OpenGL Initialization**: By .so library
- **Java/Kotlin Code**: None required
- **Use Case**: Pure native C++ applications

## Key Characteristics

- Entire app runs in native C++ code
- No Java/Kotlin layer needed
- .so creates and manages EGL context
- Direct access to all native APIs
- Minimal overhead

## AndroidManifest.xml

```xml
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.example.ului.nativeactivity.so">

    <uses-feature android:glEsVersion="0x00030000" android:required="true" />

    <application
        android:label="ULUI NativeActivity (SO-GL)"
        android:hardwareAccelerated="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">

        <activity
            android:name="android.app.NativeActivity"
            android:configChanges="orientation|keyboardHidden"
            android:exported="true">
            
            <meta-data
                android:name="android.app.lib_name"
                android:value="ului_app" />

            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

## app/build.gradle

```gradle
plugins {
    id 'com.android.application'
}

android {
    compileSdk 33
    ndkVersion "25.1.8937393"

    defaultConfig {
        applicationId "com.example.ului.nativeactivity.so"
        minSdk 21
        targetSdk 33
        versionCode 1
        versionName "1.0"
        
        ndk {
            abiFilters 'arm64-v8a'
        }
    }

    buildTypes {
        release {
            minifyEnabled false
        }
    }
}
```

## How It Works

1. Android launches NativeActivity
2. NativeActivity loads `libului_app.so`
3. Calls `android_main(android_app*)` entry point
4. Native code creates EGL display and context
5. Native code initializes OpenGL ES 3.0
6. Native code handles rendering loop
7. All input/lifecycle events handled in native code

## Advantages

✅ No Java/Kotlin overhead
✅ Full control over GL context
✅ Simplest native integration
✅ Best for pure C++ games/apps

## Disadvantages

❌ No access to Java Android APIs directly
❌ Harder to integrate with Android UI
❌ Must handle all platform features in C++

## Building

```bash
./gradlew assembleDebug
./gradlew installDebug
```

## Native Code Structure

The .so library must export:
- `android_main(struct android_app*)` - Main entry point
- Handle EGL initialization internally
- Create OpenGL ES 3.0 context
- Manage rendering loop
