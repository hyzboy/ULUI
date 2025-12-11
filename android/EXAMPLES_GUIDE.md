# Complete Android Integration Examples Guide

This document provides detailed implementation guidance for all 6 example projects.

## Project Templates

Each example requires these core files:
- `build.gradle` (project root)
- `settings.gradle`  
- `app/build.gradle`
- `app/src/main/AndroidManifest.xml`
- `app/src/main/res/values/strings.xml`

## Example 1: NativeActivity - SO initializes GL

**Complete implementation in**: `example-nativeactivity-so-gl/`

### Key Files:

**AndroidManifest.xml**:
```xml
<activity android:name="android.app.NativeActivity">
    <meta-data android:name="android.app.lib_name" android:value="ului_app" />
</activity>
```

**Native Code**:
- Exports: `android_main(android_app*)`
- Creates EGL display/context internally
- Manages full GL lifecycle

---

## Example 2: NativeActivity - App initializes GL

**Complete implementation in**: `example-nativeactivity-app-gl/`

### Differences from Example 1:

**AndroidManifest.xml**: Same structure but different package

**Native Code Changes**:
- Exports: `android_main(android_app*)`
- Does NOT create EGL context
- Uses context provided by NativeActivity framework
- Calls `eglGetCurrentContext()` to get existing context

**Key Point**: Set window format before surface creation:
```cpp
ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);
```

---

## Example 3: GameActivity - SO initializes GL

**Complete implementation in**: `example-gameactivity-so-gl/`

### Additional Dependencies:

**app/build.gradle**:
```gradle
dependencies {
    implementation 'androidx.games:games-activity:2.0.2'
}
```

**AndroidManifest.xml**:
```xml
<activity android:name="com.google.androidgamesdk.GameActivity">
    <meta-data android:name="android.app.lib_name" android:value="ului_app" />
</activity>
```

**Native Code**:
- Same as Example 1 but uses GameActivity features
- Better input handling through game-activity API
- Touch events via GameActivityMotionEvent

---

## Example 4: GameActivity - App initializes GL  

**Complete implementation in**: `example-gameactivity-app-gl/`

### Combines:
- GameActivity framework (like Example 3)
- App-managed GL context (like Example 2)

**Native Code**:
- Uses existing GL context from GameActivity
- Better integration with Android GL management

---

## Example 5: Java/Kotlin - SO initializes GL

**Complete implementation in**: `example-java-so-gl/`

### Java Activity:

**MainActivity.java**:
```java
public class MainActivity extends Activity {
    static {
        System.loadLibrary("ului_app");
    }
    
    private GLSurfaceView glView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(3);
        glView.setRenderer(new NativeRenderer());
        setContentView(glView);
    }
    
    private static class NativeRenderer implements GLSurfaceView.Renderer {
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            nativeOnSurfaceCreated();
        }
        
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            nativeOnSurfaceChanged(width, height);
        }
        
        public void onDrawFrame(GL10 gl) {
            nativeOnDrawFrame();
        }
    }
    
    private static native void nativeOnSurfaceCreated();
    private static native void nativeOnSurfaceChanged(int width, int height);
    private static native void nativeOnDrawFrame();
}
```

**Native JNI Code**:
```cpp
extern "C" {
    JNIEXPORT void JNICALL
    Java_com_example_ului_MainActivity_nativeOnSurfaceCreated(JNIEnv* env, jclass clazz) {
        // Get current EGL context (already created by GLSurfaceView)
        // But we create our own if needed - this is "SO initializes GL" mode
    }
    
    JNIEXPORT void JNICALL
    Java_com_example_ului_MainActivity_nativeOnSurfaceChanged(
        JNIEnv* env, jclass clazz, jint width, jint height) {
        // Resize viewport
    }
    
    JNIEXPORT void JNICALL
    Java_com_example_ului_MainActivity_nativeOnDrawFrame(JNIEnv* env, jclass clazz) {
        // Render frame
    }
}
```

---

## Example 6: Java/Kotlin - App initializes GL

**Complete implementation in**: `example-java-app-gl/`

### Same as Example 5 BUT:

**Native Code**:
- Does NOT create EGL context
- Uses context created by GLSurfaceView
- Simplest JNI integration

**MainActivity.java**: Same as Example 5

**Key Difference**:
```cpp
// In native code - just use existing context
JNIEXPORT void JNICALL
Java_com_example_ului_MainActivity_nativeOnSurfaceCreated(...) {
    // Context already exists from GLSurfaceView
    // Just initialize your rendering state
    // NO eglCreateContext() calls
}
```

---

## Common Files for All Examples

### Root build.gradle:
```gradle
buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:8.1.0'
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}
```

### settings.gradle:
```gradle
rootProject.name = "ULUI Example"
include ':app'
```

### strings.xml:
```xml
<resources>
    <string name="app_name">ULUI Example</string>
</resources>
```

---

## Building All Examples

```bash
# From repository root
export ANDROID_NDK=/path/to/ndk
./build-android.sh

# Then for each example:
cd android/example-xxx
./gradlew assembleDebug
./gradlew installDebug
```

---

## Choosing the Right Example

| Requirement | Recommended Example |
|-------------|-------------------|
| Pure C++ game | Example 1 or 3 |
| Need Android UI | Example 5 or 6 |
| Modern game project | Example 3 or 4 |
| Integrate into existing app | Example 6 |
| Maximum native control | Example 1 |
| Easiest Java integration | Example 6 |

---

## Testing Checklist

For each example, verify:
- ✅ .so loads successfully
- ✅ OpenGL ES 3.0 context created
- ✅ Shaders compile
- ✅ Triangle renders correctly
- ✅ No crashes on rotation
- ✅ Proper cleanup on exit
