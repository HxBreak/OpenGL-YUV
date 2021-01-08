/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code
#include "my_gl_export.h"
#include "render_const.h"
#define GLOBALS
#include "shared.h"
#undef GLOBALS

void setSimpleTexParameter(){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

bool checkGlError(const char * tag){
    auto err = glGetError();
    if (err != GL_NO_ERROR){
        LOGE("%s %x", tag, err);
    }
    return true;
}

void esLogMessage ( const char *formatStr, ... )
{
    va_list params;
    char buf[BUFSIZ];

    va_start ( params, formatStr );
    vsprintf ( buf, formatStr, params );

    __android_log_print ( ANDROID_LOG_INFO, "esUtil" , "%s", buf );

    va_end ( params );
}

GLuint LoadShader(GLenum type, const char *shaderSrc) {
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0) {
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = reinterpret_cast<char *>(malloc(sizeof(char) * infoLen));

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            esLogMessage("Error compiling shader:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;

}

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

void initContext(JNIEnv *env, ANativeWindow * windowFromSurface, AAssetManager* native_am, NV21Texture * pNv21Texture) {
//    auto windowFromSurface = ANativeWindow_fromSurface(env, surface);
    if ((eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGE("Get Display Error");
        return;
    }
    if (eglInitialize(eglDisplay, nullptr, nullptr) == EGL_FALSE) {
        LOGE("Initialize Failed");
        return;
    }
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT,  // Request opengl ES2.0
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              24,
                              EGL_BUFFER_SIZE,
                              32,
                              EGL_NONE};
    EGLConfig config;
    EGLint config_num;
    if (eglChooseConfig(eglDisplay, attribs, &config, 1, &config_num) == EGL_FALSE) {
        LOGE("SetAttributes Error %x", eglGetError());
        return;
    }
    if (!config_num) {
        LOGE("Error retrieve Config");
        return;
    }
    eglSurface = eglCreateWindowSurface(eglDisplay, config, windowFromSurface, nullptr);
    eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
    eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

    const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                                      3,  // Request opengl ES2.0
                                      EGL_NONE};
    auto context = eglCreateContext(eglDisplay, config, nullptr, context_attribs);
    if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context) == EGL_FALSE) {
        LOGE("Error Make Current");
        return;
    }
//    auto native_am = AAssetManager_fromJava(env, am);
//    auto asset_sample = AAssetManager_open(native_am, "sample.yuv", AASSET_MODE_UNKNOWN);
    auto asset_shader = AAssetManager_open(native_am, "sample.frag", AASSET_MODE_UNKNOWN);
    using namespace std;
//    auto new_buf = malloc(AAsset_getLength(asset_sample));
    auto shader_buf = malloc(AAsset_getLength(asset_shader) + 1);
//    AAsset_read(asset_sample, new_buf, AAsset_getLength(asset_sample));
    auto len = AAsset_read(asset_shader, shader_buf, AAsset_getLength(asset_shader));
    reinterpret_cast<char*>(shader_buf)[len] = '\0';
//    pNv21Texture->data = new_buf;
    pNv21Texture->frag_shader = static_cast<const char *>(shader_buf);
    if (!SetupGraphics(env, width, height, pNv21Texture)) {
        exit(0);
    }
    pNv21Texture->frag_shader = nullptr;
    free(shader_buf);
    AAsset_close(asset_shader);
//    AAsset_close(asset_sample);
}

GLuint * pbo = nullptr;

bool makeTexture(NV21Texture *data){
    if(data->data == NULL) {
        LOGE("error NV21Texture is empty!!!====\n");
        return false;
    }
    auto ySize = data->width * data->height;
    auto uvSize = ySize / 2;
    auto uvData = reinterpret_cast<const unsigned char *>(data->data) + data->width * data->height;
    if (texture == nullptr){
        /**
         * Gen Buffers And Textures
         */
        pbo = static_cast<GLuint *>(malloc(sizeof(GLuint) * 2));
        glGenBuffers(2, pbo);
        texture = static_cast<GLuint *>(malloc(sizeof(GLuint) * 2));
        glGenTextures(2, texture);
        /**
         * Bind Y Texture
         */
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[0]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, ySize, data->data, GL_DYNAMIC_DRAW);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, data->width, data->height,
                     0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        setSimpleTexParameter();
        /**
         * Bind UV Texture
         */
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[1]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, uvSize, uvData, GL_DYNAMIC_DRAW);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, data->width / 2, data->height / 2,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,NULL);
        setSimpleTexParameter();
    }else{
        /**
         * Y Area
         */
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[0]);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, ySize, data->data);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data->width , data->height, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        /**
         * UV Area
         */
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo[1]);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, uvSize, uvData);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data->width / 2, data->height / 2,
                        GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,NULL);
    }
    return true;
}

bool SetupGraphics(JNIEnv *env, GLint w, GLint h, NV21Texture *data) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    LOGE("setupGraphics(%d, %d)", w, h);
    glViewport(0, 0, w, h);

    char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec3 a_Color;   \n"
            "layout(location = 2) in vec2 a_texCoord;   \n"
            "out vec4 v_Color;                       \n"
            "out vec2 v_texCoord;                       \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "   v_Color = vec4(a_Color, 1.0f);                \n"
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, data->frag_shader);

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0) {
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = reinterpret_cast<char *>(malloc(sizeof(char) * infoLen));

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            LOGE ("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return false;
    }

    // Store the program object
    obj = programObject;
    g_texture_layout[0] = glGetUniformLocation(obj, "s_textureY");
    g_texture_layout[1] = glGetUniformLocation(obj, "s_textureUV");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return true;
}

void drawFrame(NV21Texture * nv21Texture, std::mutex& mem_access) {
    auto start = clock();
    glClear(GL_COLOR_BUFFER_BIT);
    mem_access.lock();
    makeTexture(nv21Texture);
    mem_access.unlock();
    auto uploadTex = clock();
    glUseProgram(obj);
    if (vbo == nullptr) {
        vbo = static_cast<VBO *>(malloc(sizeof(VBO)));
        glGenBuffers(3, vbo->vertexVboId);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexVboId[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, vBufVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexVboId[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, vTexCoord, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vertexVboId[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    //Position
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexVboId[0]);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    //TexCoord
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vertexVboId[1]);
    glEnableVertexAttribArray(TEX_COORD);
    glVertexAttribPointer(TEX_COORD, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

    /**
     * Bind Texture
     */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(g_texture_layout[0], 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glUniform1i(g_texture_layout[1], 1);

    /**
     * Special Triangle Render Index
     */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vertexVboId[2]);

    /**
     * Render Rect
     */
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    /**
     * Unbind Render Buffer
     */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    eglSwapBuffers(eglDisplay, eglSurface);
//    LOGE("FRAME COST: %f, %f", (float)(clock() - start) / (float)CLOCKS_PER_SEC, (float)(uploadTex - start) / (float)CLOCKS_PER_SEC);
}