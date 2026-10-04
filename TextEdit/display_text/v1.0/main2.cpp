#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
    #define UNICODE
	#include <windows.h>
#endif
#include <mmsystem.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdint.h>
#include "cyxlib_source/data-structs.cpp"
#include "cyxlib_source/non-crt-math.cpp"
#include "cyxlib_source/GL-GDI.cpp"
#include "cyxlib_source/IO.cpp"
#include <stdio.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")


GLuint create_empty_texture(SIZE size, GLenum format = GL_BGRA_EXT) {
	GLuint textureID;
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.cx, size.cy, 0, format, GL_UNSIGNED_BYTE, nullptr);
	return textureID;
}

struct FONT_CONTEXT {
	HDC     htextDC;
	HBITMAP htextBMP;
	BYTE*   BMPdata;
	SIZE    BMPsize, last_text_size;
	static BITMAPINFO create_default_BMI(SIZE BMPsize) {
		BITMAPINFO BMI = { 0 };
    	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    	BMI.bmiHeader.biWidth = BMPsize.cx;   BMI.bmiHeader.biHeight = -BMPsize.cy;
   		BMI.bmiHeader.biPlanes = 1;           BMI.bmiHeader.biBitCount = 32;
    	BMI.bmiHeader.biCompression = BI_RGB; BMI.bmiHeader.biSizeImage = BMPsize.cx * BMPsize.cy * 4;
    	return BMI;
	}
	void create_BMP() {
		BITMAPINFO BMI = create_default_BMI(BMPsize);
		htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&BMPdata, NULL, 0);
		SelectObject(htextDC, htextBMP);
	}
	FONT_CONTEXT(): htextDC(CreateCompatibleDC(NULL)), BMPsize(SIZE{ 64, 64 }) {
		create_BMP();
		SetTextColor(htextDC, RGB(255, 255, 255));
		SetBkColor(htextDC, (COLORREF)0);
		SetBkMode(htextDC, OPAQUE);
	}
	~FONT_CONTEXT() {
		DeleteObject(htextBMP);
		DeleteDC(htextDC);
	}
	void setfont(HFONT hfont) { SelectObject(htextDC, hfont); }
	void render(const wchar_t* text) {
		int32_t textlen = (int32_t)wcslen(text);
		GetTextExtentPoint32W(htextDC, text, textlen, &last_text_size);
		if (last_text_size.cx > BMPsize.cx || last_text_size.cy > BMPsize.cy) {
			BMPsize = last_text_size;
			DeleteObject(htextBMP);
			create_BMP();
		}
		RECT text_zone = RECT{ 0, 0, last_text_size.cx, last_text_size.cy };
		printf("len: %d | size: %d x %d\n", textlen, last_text_size.cx, last_text_size.cy);
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &text_zone, text, textlen, nullptr);
	}
};

struct CHARPAGE {
	GLuint textureID;
	CHARPAGE(SIZE _size = SIZE{ 64, 64 }) { textureID = create_empty_texture(_size, GL_LUMINANCE); }
	void write(const wchar_t* text, POINT pos, FONT_CONTEXT* context, HANDLE hheap = NULL, HFONT hfont = NULL) {
		if (hfont != NULL) context->setfont(hfont);
		context->render(text);
		POINT source_from = POINT{ 0, 0 }, source_to = POINT{ context->last_text_size.cx, context->last_text_size.cy };
		size_t R8size = size_t(source_to.y - source_from.y) * (source_to.x - source_from.x);
		BYTE* R8data;
		if (R8size > 1024) R8data = (BYTE*)HeapAlloc((hheap == NULL) ? GetProcessHeap() : hheap, 0, R8size);
		else R8data = (BYTE*)__builtin_alloca(R8size);
		size_t R8data_counter = 0;
		for (; source_from.y < source_to.y; source_from.y++) {
			printf("ln%02d:", source_from.y);
			size_t line_offset = size_t(source_to.x - source_from.x) * source_from.y;
			for (int32_t current_x = source_from.x; current_x < source_to.x; current_x++) {
				R8data[R8data_counter++] = context->BMPdata[(line_offset + current_x) * 4];
				if (R8data[R8data_counter - 1]) printf(" #");
				else printf(" .");
			}
			printf("\n");
		}
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, context->last_text_size.cx, context->last_text_size.cy, GL_LUMINANCE, GL_UNSIGNED_BYTE, R8data);
		/*
		BYTE* the_void = new BYTE[64 * 64 * 4];
		memset(the_void, 128, 64 * 64 * 4);
		printf("is texture? %d\n", (int)glIsTexture(textureID));
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, context->last_text_size.cx, context->last_text_size.cy, GL_LUMINANCE, GL_UNSIGNED_BYTE, the_void);
		delete the_void;
		*/
		if (R8size > 1024) HeapFree((hheap == NULL) ? GetProcessHeap() : hheap, 0, R8data);
	}
};


void test_proc(GL_WINDOW* root, void* image_textureID) {
	//printf("%d", (int)glIsTexture(*(GLuint*)image_textureID));
	GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("GL error before clear: 0x%04X\n", err);
    }
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	root->switch_dimension(2);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, *(GLuint*)image_textureID);
	glEnable(GL_TEXTURE_2D);
	glColor4ub(255, 255, 255, 255);

	if (glIsTexture(*(GLuint*)image_textureID)) {
		glBegin(GL_QUADS);
        	glTexCoord2f(0.f, 0.f); glVertex2i(00, 00);
        	glTexCoord2f(1.f, 0.f); glVertex2i(64, 00);
        	glTexCoord2f(1.f, 1.f); glVertex2i(64, 64);
        	glTexCoord2f(0.f, 1.f); glVertex2i(00, 64);
    	glEnd();
	}
	

    glDisable(GL_TEXTURE_2D);
    glFlush();
    SwapBuffers(root->hDC);
}

int main() {
	DWORD PID = GetCurrentProcessId(), tick = GetTickCount();
    srand(PID ^ tick);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_WINDOW top(400, 300, true, true);
    
    LOGFONT LF = GL_WINDOW::_create_default_logfont();
    HFONT hfont = CreateFontIndirectW(&LF);
    FONT_CONTEXT font_context;
    font_context.setfont(hfont);
    CHARPAGE page0;
    page0.write(L"PAGE 0", POINT{ 0, 0 }, &font_context);
    
	// top.set_render_proc(GL_WINDOW::render_frame_demo);
	top.set_render_proc(test_proc);
	top.set_additional_render_info(&(page0.textureID));
	top.mainloop();
	
	DeleteObject(hfont);
	return 0;
}
