#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <cstdint>
#include <cstdio>

// ========== 手动定义 OpenGL 类型 ==========
typedef uint32_t GLenum;
typedef uint32_t GLuint;
typedef int32_t  GLint;
typedef int32_t  GLsizei;
typedef uint8_t  GLboolean;
typedef float    GLfloat;
typedef char     GLchar;

// ========== 手动定义 OpenGL 常量 ==========
#define GL_VERTEX_SHADER          0x8B31
#define GL_FRAGMENT_SHADER        0x8B30
#define GL_COMPILE_STATUS         0x8B81
#define GL_LINK_STATUS            0x8B82
#define GL_INFO_LOG_LENGTH        0x8B84
#define GL_TRIANGLES              0x0004

// ========== 函数指针类型 ==========
typedef void   (WINAPI *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLuint (WINAPI *PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (WINAPI *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar** source, const GLint* length);
typedef void   (WINAPI *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (WINAPI *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void   (WINAPI *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint (WINAPI *PFNGLCREATEPROGRAMPROC)(void);
typedef void   (WINAPI *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (WINAPI *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (WINAPI *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void   (WINAPI *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void   (WINAPI *PFNGLDELETESHADERPROC)(GLuint shader);
typedef void   (WINAPI *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef GLint  (WINAPI *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void   (WINAPI *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);

// ========== 函数指针变量 ==========
static PFNGLUSEPROGRAMPROC       glUseProgram;
static PFNGLCREATESHADERPROC     glCreateShader;
static PFNGLSHADERSOURCEPROC     glShaderSource;
static PFNGLCOMPILESHADERPROC    glCompileShader;
static PFNGLGETSHADERIVPROC      glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC    glCreateProgram;
static PFNGLATTACHSHADERPROC     glAttachShader;
static PFNGLLINKPROGRAMPROC      glLinkProgram;
static PFNGLGETPROGRAMIVPROC     glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLDELETESHADERPROC     glDeleteShader;
static PFNGLDELETEPROGRAMPROC    glDeleteProgram;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLUNIFORM3FPROC        glUniform3f;

// ========== 窗口相关 ==========
static HWND  hwnd;
static HDC   hdc;
static HGLRC hrc;

// ========== 着色器源码（GLSL 1.30） ==========
const char* vertexShaderSrc =
    "#version 130\n"
	"varying vec3 vColor;\n"  // 传递给片段着色器的颜色
	"void main() {\n"
    "    gl_Position = ftransform();\n"
    "    vColor = gl_Color.rgb;\n"  // 获取固定管线的颜色
	"}\n";
/*
const char* fragmentShaderSrc =
    "#version 130\n"
    "out vec4 FragColor;\n"
    "uniform vec3 colorWhite;\n"
    "uniform vec3 colorBlack;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(colorWhite, 1.0);\n"
    //"    float gray = gl_Color.r;\n"
    //"    vec3 result = mix(colorBlack, colorWhite, gray);\n"
    //"    FragColor = vec4(result, 1.0);\n"
    "}\n";
*/
// 片段着色器
const char* fragmentShaderSrc =
	"#version 130\n"
	"out vec4 FragColor;\n"
	"uniform vec3 colorWhite;\n"
	"uniform vec3 colorBlack;\n"
	"varying vec3 vColor;\n"  // 接收顶点着色器传来的颜色
	"void main() {\n"
    "    float gray = dot(vColor, vec3(0.299, 0.587, 0.114));\n" // 用顶点颜色的亮度作为插值因子
    "    vec3 result = mix(colorBlack, colorWhite, gray);\n"
    "    FragColor = vec4(result, 1.0);\n"
	"}\n";
// ========== 加载所有函数 ==========
bool loadOpenGLFunctions() {
    // 尝试创建 3.0 上下文
    typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (wglCreateContextAttribsARB) {
		/*
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 0,
            0
        };*/
        int attribs[] = {
    		0x2091, 3, // WGL_CONTEXT_MAJOR_VERSION_ARB
    		0x2092, 0, // WGL_CONTEXT_MINOR_VERSION_ARB
    		// 0x9126,    // WGL_CONTEXT_PROFILE_MASK_ARB
    		// 0x0002,    // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
    		0
		};
        hrc = wglCreateContextAttribsARB(hdc, 0, attribs);
        if (hrc) {
            wglMakeCurrent(hdc, hrc);
        }
    }

    // 如果失败，用临时上下文
    if (!hrc) {
        printf("Falling back to legacy context\n");
        hrc = wglCreateContext(hdc);
        wglMakeCurrent(hdc, hrc);
    }

    // 打印版本
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    GLenum GL_SHADING_LANGUAGE_VERSION = 0x8B8C;
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // 加载函数
    glUseProgram       = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glCreateShader     = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource     = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader    = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv      = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram    = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader     = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram      = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv     = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDeleteShader     = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glDeleteProgram    = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform3f        = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");

    // 检查
    if (!glCreateShader || !glUseProgram) {
        printf("Critical functions failed to load!\n");
        printf("glCreateShader: %p\n", (void*)glCreateShader);
        printf("glUseProgram: %p\n", (void*)glUseProgram);
        return false;
    }

    return true;
}

// ========== 编译着色器 ==========
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1) {
            char* infoLog = new char[logLength];
            GLsizei written = 0;
            glGetShaderInfoLog(shader, logLength, &written, infoLog);
            printf("Shader compilation error:\n%s\n", infoLog);
            delete[] infoLog;
        }
    }
    return shader;
}

// ========== 创建着色器程序 ==========
GLuint createProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 1) {
            char* infoLog = new char[logLength];
            GLsizei written = 0;
            glGetProgramInfoLog(program, logLength, &written, infoLog);
            printf("Program linking error:\n%s\n", infoLog);
            delete[] infoLog;
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// ========== 窗口过程 ==========
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_PAINT: {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(-100, -100);
            glColor3f(0.0f, 0.0f, 0.0f);
            glVertex2i(100, -100);
            glColor3f(0.5f, 0.5f, 0.5f);
            glVertex2i(0, 100);
            glEnd();

            SwapBuffers(hdc);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ========== 入口 ==========
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OpenGLWindow";
    RegisterClass(&wc);

    hwnd = CreateWindow("OpenGLWindow", "Grayscale Color Mapping",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 600, NULL, NULL, hInstance, NULL);

    hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    if (!loadOpenGLFunctions()) {
        printf("Failed to initialize OpenGL\n");
        return -1;
    }

    GLuint program = createProgram();
    glUseProgram(program);

    GLint colorWhiteLoc = glGetUniformLocation(program, "colorWhite");
    GLint colorBlackLoc = glGetUniformLocation(program, "colorBlack");
    printf("Uniform locations: white=%d black=%d\n", colorWhiteLoc, colorBlackLoc);

    glUniform3f(colorWhiteLoc, 0x12/255.0f, 0x34/255.0f, 1.0f);
    glUniform3f(colorBlackLoc, 0x0A/255.0f, 0x0A/255.0f, 0x0A/255.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-200, 200, -200, 200);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        InvalidateRect(hwnd, NULL, FALSE);
    }

    glDeleteProgram(program);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return 0;
}
