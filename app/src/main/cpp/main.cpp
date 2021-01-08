
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <linux/videodev2.h>
#include <cstring>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include "my_gl_export.h"
#include <mutex>
#include <condition_variable>

using namespace std;
mutex mtx, mem_access;
condition_variable readyToRender;

jobject on_frame_listener = nullptr;
jobject surface = nullptr;
JavaVM* g_vm = nullptr;

void* share_buffer = NULL;

struct CaptureInfo{
    int width;
    int height;
};

ANativeWindow * g_native_window;
AAssetManager * g_assetmanager;
CaptureInfo * g_capture_info = nullptr;

void* render_thread(void* arg){
    JNIEnv* env;
    if(g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK){
        return nullptr;
    }
    auto _texture = reinterpret_cast<NV21Texture*>(malloc(sizeof(NV21Texture)));

    _texture->width = g_capture_info->width;
    _texture->height = g_capture_info->height;
    initContext(env, g_native_window, g_assetmanager, _texture);
    g_vm->DetachCurrentThread();

    unique_lock<mutex> lck(mtx);
    while (1){
        readyToRender.wait(lck);
        _texture->data = share_buffer;
        drawFrame(_texture, mem_access);
    }
    return nullptr;
}

void* test_thread(void* arg){

    auto asset = AAssetManager_open(g_assetmanager, "sample.yuv", AASSET_MODE_UNKNOWN);
    auto * pdata = malloc(AAsset_getLength(asset));
    AAsset_read(asset, pdata, AAsset_getLength(asset));
    AAsset_close(asset);
    while (1){
        mem_access.lock();
        /**
         * Mem Cpy
         */
        share_buffer = pdata;
        mem_access.unlock();
        readyToRender.notify_all();
    }
    return nullptr;
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    g_vm = vm;
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    return JNI_VERSION_1_4;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_android_gl2jni_NativeUvcCamera_openCamera(JNIEnv *env, jobject thiz, jobject surface,
                                                   jobject am, jint width, jint height) {
    g_assetmanager = AAssetManager_fromJava(env, am);
    g_native_window = ANativeWindow_fromSurface(env, surface);
    g_capture_info = reinterpret_cast<CaptureInfo*>(malloc(sizeof(CaptureInfo)));
    g_capture_info->width = width;
    g_capture_info->height = height;
    pthread_t tid, renderThreadId;
    pthread_create(&renderThreadId, nullptr, &render_thread, nullptr);
    pthread_create(&tid, nullptr, &test_thread, nullptr);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_android_gl2jni_NativeUvcCamera_release(JNIEnv *env, jobject thiz) {
    free(g_capture_info);
    g_capture_info = nullptr;
}