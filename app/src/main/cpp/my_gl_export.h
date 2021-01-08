#ifndef MY_GL_EXPORT_H_
#define MY_GL_EXPORT_H_
#include <jni.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/log.h>
#include <android/surface_texture.h>
#include <android/window.h>
#include <android/native_window_jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "android/asset_manager_jni.h"
#include "android/asset_manager.h"
#include <mutex>

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct VBO {
    GLuint vertexVboId[3];
};

struct NV21Texture {
    GLuint width;
    GLuint height;
    void *data;
    const char *frag_shader;
};

bool SetupGraphics(JNIEnv *env, GLint w, GLint h, NV21Texture *data);

enum NV12ScriptLayout {
    POSITION,
    COLOR,
    TEX_COORD,
};

void esLogMessage ( const char *formatStr, ... );

extern "C" void drawFrame(NV21Texture * nv21Texture, std::mutex& mem_access);

void setSimpleTexParameter();

bool checkGlError(const char * tag);

extern "C" void initContext(JNIEnv *env, ANativeWindow * windowFromSurface, AAssetManager* native_am, NV21Texture * nv21Texture);

#endif