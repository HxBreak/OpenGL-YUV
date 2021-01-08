package com.android.gl2jni

import android.content.Context
import android.graphics.BitmapFactory
import android.util.AttributeSet
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class GLView @JvmOverloads constructor(context: Context?, attrs: AttributeSet? = null, defStyle: Int = 0) :
        SurfaceView(context, attrs, defStyle), SurfaceHolder.Callback {

    init {
        holder.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder?) {}

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        NativeUvcCamera.openCamera(holder!!.surface, context!!.assets, 1920, 1200)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }

}