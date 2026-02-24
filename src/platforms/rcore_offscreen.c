/**********************************************************************************************
*
*   rcore_offscreen - Functions to manage window, graphics device and inputs
*
*   PLATFORM: OFFSCREEN
*       - EGL surfaceless platform for headless/CI rendering without a display server
*       - Uses EGL_MESA_platform_surfaceless + Mesa llvmpipe for software OpenGL
*       - No X11, Wayland, DRM, or GPU required
*
*   DEPENDENCIES:
*       - EGL (libEGL)
*       - OpenGL (libGL) or OpenGL ES (libGLESv2)
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5) and contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include <string.h>
#include <time.h>

// GLAD (included by rlgl.h) embeds its own khrplatform.h which defines the
// __khrplatform_h_ include guard but uses KHRONOS_GLAD_API_PTR instead of
// KHRONOS_APIENTRY. When the system EGL headers later try to use
// KHRONOS_APIENTRY (via eglplatform.h -> khrplatform.h, which is now skipped
// due to the guard), the macro is undefined and all EGL function declarations
// fail. Fix by defining the missing macros before including EGL headers.
#ifndef KHRONOS_APIENTRY
    #ifdef _WIN32
        #define KHRONOS_APIENTRY __stdcall
    #else
        #define KHRONOS_APIENTRY
    #endif
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local helpers
//----------------------------------------------------------------------------------

#define CASE_STR(value) case value: return #value;
static const char *eglGetErrorString(EGLint error) {
    switch (error) {
        CASE_STR(EGL_SUCCESS)
        CASE_STR(EGL_NOT_INITIALIZED)
        CASE_STR(EGL_BAD_ACCESS)
        CASE_STR(EGL_BAD_ALLOC)
        CASE_STR(EGL_BAD_ATTRIBUTE)
        CASE_STR(EGL_BAD_CONTEXT)
        CASE_STR(EGL_BAD_CONFIG)
        CASE_STR(EGL_BAD_CURRENT_SURFACE)
        CASE_STR(EGL_BAD_DISPLAY)
        CASE_STR(EGL_BAD_SURFACE)
        CASE_STR(EGL_BAD_MATCH)
        CASE_STR(EGL_BAD_PARAMETER)
        CASE_STR(EGL_BAD_NATIVE_PIXMAP)
        CASE_STR(EGL_BAD_NATIVE_WINDOW)
        CASE_STR(EGL_CONTEXT_LOST)
        default: return "Unknown";
    }
}
#undef CASE_STR

static int hasExtension(const char *extensions, const char *ext) {
    if (!extensions || !ext) return 0;
    size_t len = strlen(ext);
    const char *p = extensions;
    while ((p = strstr(p, ext)) != NULL) {
        if ((p == extensions || p[-1] == ' ') && (p[len] == '\0' || p[len] == ' ')) return 1;
        p += len;
    }
    return 0;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);
bool InitGraphicsDevice(void);

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

bool WindowShouldClose(void) {
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

void ToggleFullscreen(void) {
    TRACELOG(LOG_WARNING, "ToggleFullscreen() not available on target platform");
}

void ToggleBorderlessWindowed(void) {
    TRACELOG(LOG_WARNING, "ToggleBorderlessWindowed() not available on target platform");
}

void MaximizeWindow(void) {
    TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

void MinimizeWindow(void) {
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

void RestoreWindow(void) {
    TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

void SetWindowState(unsigned int flags) {
    TRACELOG(LOG_WARNING, "SetWindowState() not available on target platform");
}

void ClearWindowState(unsigned int flags) {
    TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

void SetWindowIcon(Image image) {
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

void SetWindowIcons(Image *images, int count) {
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

void SetWindowTitle(const char *title) {
    CORE.Window.title = title;
}

void SetWindowPosition(int x, int y) {
    TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

void SetWindowMonitor(int monitor) {
    TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

void SetWindowMinSize(int width, int height) {
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

void SetWindowMaxSize(int width, int height) {
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

void SetWindowSize(int width, int height) {
    TRACELOG(LOG_WARNING, "SetWindowSize() not available on target platform");
}

void SetWindowOpacity(float opacity) {
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

void SetWindowFocused(void) {
    TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

void *GetWindowHandle(void) {
    return NULL;
}

int GetMonitorCount(void) {
    return 1;
}

int GetCurrentMonitor(void) {
    return 0;
}

Vector2 GetMonitorPosition(int monitor) {
    return (Vector2){ 0, 0 };
}

int GetMonitorWidth(int monitor) {
    return CORE.Window.screen.width;
}

int GetMonitorHeight(int monitor) {
    return CORE.Window.screen.height;
}

int GetMonitorPhysicalWidth(int monitor) {
    return 0;
}

int GetMonitorPhysicalHeight(int monitor) {
    return 0;
}

int GetMonitorRefreshRate(int monitor) {
    return 60;
}

const char *GetMonitorName(int monitor) {
    return "Offscreen (EGL surfaceless)";
}

Vector2 GetWindowPosition(void) {
    return (Vector2){ 0, 0 };
}

Vector2 GetWindowScaleDPI(void) {
    return (Vector2){ 1.0f, 1.0f };
}

void SetClipboardText(const char *text) {
    TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

const char *GetClipboardText(void) {
    return NULL;
}

Image GetClipboardImage(void) {
    Image image = { 0 };
    return image;
}

void ShowCursor(void) {
    CORE.Input.Mouse.cursorHidden = false;
}

void HideCursor(void) {
    CORE.Input.Mouse.cursorHidden = true;
}

void EnableCursor(void) {
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    CORE.Input.Mouse.cursorHidden = false;
}

void DisableCursor(void) {
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    CORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void) {
    if (platform.surface != EGL_NO_SURFACE) {
        eglSwapBuffers(platform.display, platform.surface);
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

double GetTime(void) {
    double time = 0.0;
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;
    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;
    return time;
}

void OpenURL(const char *url) {
    TRACELOG(LOG_WARNING, "OpenURL() not implemented on target platform");
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

int SetGamepadMappings(const char *mappings) {
    return 0;
}

void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration) {
}

void SetMousePosition(int x, int y) {
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

void SetMouseCursor(int cursor) {
}

const char *GetKeyName(int key) {
    return "";
}

void PollInputEvents(void) {
#if defined(SUPPORT_GESTURES_SYSTEM)
    UpdateGestures();
#endif

    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    CORE.Input.Gamepad.lastButtonPressed = 0;
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];
    for (int i = 0; i < 260; i++) {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

int InitPlatform(void) {
    TRACELOG(LOG_INFO, "OFFSCREEN: Initializing EGL surfaceless platform");

    // Check for surfaceless platform support in EGL client extensions
    const char *clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!clientExts) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: Cannot query EGL client extensions (EGL too old?)");
        return -1;
    }

    if (!hasExtension(clientExts, "EGL_MESA_platform_surfaceless")) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: EGL_MESA_platform_surfaceless not supported");
        return -1;
    }

    // Get platform display function
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (!eglGetPlatformDisplayEXT) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: Cannot resolve eglGetPlatformDisplayEXT");
        return -1;
    }

    // Get EGL display via surfaceless platform (no X11/Wayland needed)
    platform.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
    if (platform.display == EGL_NO_DISPLAY) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: eglGetPlatformDisplayEXT failed: %s", eglGetErrorString(eglGetError()));
        return -1;
    }

    EGLint major, minor;
    if (!eglInitialize(platform.display, &major, &minor)) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: eglInitialize failed: %s", eglGetErrorString(eglGetError()));
        return -1;
    }
    TRACELOG(LOG_INFO, "OFFSCREEN: EGL %d.%d initialized", major, minor);

    // Choose EGL config
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_ES3)
    EGLint renderableType = EGL_OPENGL_ES2_BIT;
#else
    EGLint renderableType = EGL_OPENGL_BIT;
#endif

    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, renderableType,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(platform.display, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: eglChooseConfig failed: %s", eglGetErrorString(eglGetError()));
        eglTerminate(platform.display);
        return -1;
    }

    // Bind API
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_ES3)
    eglBindAPI(EGL_OPENGL_ES_API);
#else
    eglBindAPI(EGL_OPENGL_API);
#endif

    // Create context
#if defined(GRAPHICS_API_OPENGL_ES2)
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
#elif defined(GRAPHICS_API_OPENGL_ES3)
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
#elif defined(GRAPHICS_API_OPENGL_43)
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };
#elif defined(GRAPHICS_API_OPENGL_33)
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };
#elif defined(GRAPHICS_API_OPENGL_21)
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 2,
        EGL_CONTEXT_MINOR_VERSION, 1,
        EGL_NONE
    };
#else
    const EGLint contextAttribs[] = { EGL_NONE };
#endif

    platform.context = eglCreateContext(platform.display, config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.context == EGL_NO_CONTEXT) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: eglCreateContext failed: %s", eglGetErrorString(eglGetError()));
        eglTerminate(platform.display);
        return -1;
    }

    // Create a 1x1 pbuffer surface so FBO 0 is valid
    const EGLint pbufferAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    platform.surface = eglCreatePbufferSurface(platform.display, config, pbufferAttribs);
    if (platform.surface == EGL_NO_SURFACE) {
        TRACELOG(LOG_WARNING, "OFFSCREEN: pbuffer creation failed, trying surfaceless context");
        platform.surface = EGL_NO_SURFACE;
    }

    if (!eglMakeCurrent(platform.display, platform.surface, platform.surface, platform.context)) {
        TRACELOG(LOG_FATAL, "OFFSCREEN: eglMakeCurrent failed: %s", eglGetErrorString(eglGetError()));
        eglDestroyContext(platform.display, platform.context);
        eglTerminate(platform.display);
        return -1;
    }

    eglSwapInterval(platform.display, 0);

    // Set up window dimensions from InitWindow() params (already set in CORE.Window.screen by rcore.c)
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.currentFbo.width = CORE.Window.screen.width;
    CORE.Window.currentFbo.height = CORE.Window.screen.height;

    SetupFramebuffer(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);
    rlLoadExtensions(eglGetProcAddress);
    InitTimer();
    CORE.Storage.basePath = GetWorkingDirectory();

    CORE.Window.ready = true;

    TRACELOG(LOG_INFO, "OFFSCREEN: Platform initialized successfully (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height);
    return 0;
}

void ClosePlatform(void) {
    CORE.Window.ready = false;

    if (platform.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(platform.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (platform.surface != EGL_NO_SURFACE) {
            eglDestroySurface(platform.display, platform.surface);
            platform.surface = EGL_NO_SURFACE;
        }
        if (platform.context != EGL_NO_CONTEXT) {
            eglDestroyContext(platform.display, platform.context);
            platform.context = EGL_NO_CONTEXT;
        }

        eglTerminate(platform.display);
        platform.display = EGL_NO_DISPLAY;
    }
}
