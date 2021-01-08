#ifdef GLOBALS
#define EXT
#else
#define EXT extern
#endif

EXT EGLDisplay eglDisplay;
EXT EGLSurface eglSurface;
EXT EGLint width, height;
EXT GLint obj = 0;

EXT GLint g_texture_layout[] = {-1, -1};
EXT GLuint * texture = nullptr;
EXT VBO *vbo;
