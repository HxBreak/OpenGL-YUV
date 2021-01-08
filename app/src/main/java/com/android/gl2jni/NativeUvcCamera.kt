package com.android.gl2jni

import android.content.res.AssetManager
import android.graphics.Bitmap
import android.os.SharedMemory
import android.view.Surface
import java.lang.Exception
import java.nio.ByteBuffer

object NativeUvcCamera {

    init {
        try {
            System.loadLibrary("gl2jni")
        }catch (e: Exception){
            e.printStackTrace()
        }
    }

    external fun openCamera(surface: Surface, am: AssetManager, width: Int, height: Int)

    external fun release()

}