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
	struct STATIC_WRITE_INFO {
		wchar_t*      text;
		FONT_CONTEXT* context;
		SIZE          page_size;
		POINT         base_dest_offset;
		HANDLE        hheap;
		HFONT         hfont;
	};
	struct DYNAMIC_WRITE_INFO {
		POINT source_offset, dest_offset;
		static DYNAMIC_WRITE_INFO get_header_info(STATIC_WRITE_INFO* basis) {
			DYNAMIC_WRITE_INFO header;
			header.source_offset = POINT{ 0, 0 };
			header.dest_offset = basis->base_dest_offset;
			return header;
		}
	};
	GLuint    textureID;
	int32_t   pageID;
	CHARPAGE *neighbor_x, *neighbor_y;
	static constexpr size_t HEAP_TIPPING_POINT = 16384ULL;
	CHARPAGE(SIZE _size, int32_t _pageID): pageID(_pageID), neighbor_x(nullptr), neighbor_y(nullptr) { textureID = create_empty_texture(_size, GL_LUMINANCE_ALPHA); }
	void writeline(const STATIC_WRITE_INFO* static_info, const DYNAMIC_WRITE_INFO* dynamic_info) {
		POINT source_from, source_to, dest_offset;
		if ((dynamic_info->source_offset.x | dynamic_info->source_offset.y) == 0) { // First call
			if (static_info->hfont != NULL) static_info->context->setfont(static_info->hfont);
    		static_info->context->render(static_info->text);
    		source_from = POINT{ 0, 0 };
    		dest_offset = static_info->base_dest_offset;
    		if (dynamic_info->dest_offset.x + static_info->context->last_text_size.cx < static_info->page_size.cx) {
				source_to.x = static_info->context->last_text_size.cx;
			} else {
				source_to.x = static_info->page_size.cx - static_info->base_dest_offset.x;
				if (neighbor_x != nullptr) {
					DYNAMIC_WRITE_INFO next_dynamic_info;
					next_dynamic_info.source_offset = POINT{ source_to.x, 0 };
					next_dynamic_info.dest_offset = POINT{ 0, static_info->base_dest_offset.y };
					printf("source_offset: (%d, %d)\ndest_offset: (%d, %d)\n",
						next_dynamic_info.source_offset.x, next_dynamic_info.source_offset.y,
						next_dynamic_info.dest_offset.x, next_dynamic_info.dest_offset.y);
					neighbor_x->writeline(static_info, &next_dynamic_info);
				}
			}
			if (dynamic_info->dest_offset.y + static_info->context->last_text_size.cy < static_info->page_size.cy) {
				source_to.y = static_info->context->last_text_size.cy;
			} else {
				source_to.y = static_info->page_size.cy - static_info->base_dest_offset.y;
				if (neighbor_y != nullptr) {
					DYNAMIC_WRITE_INFO next_dynamic_info;
					next_dynamic_info.source_offset = POINT{ 0, source_to.y };
					next_dynamic_info.dest_offset = POINT{ static_info->base_dest_offset.x, 0 };
					neighbor_y->writeline(static_info, &next_dynamic_info);
				}
			}
		} else { // Second call
			/*
			dest_offset.x = (dynamic_info->source_offset.x == 0) ? static_info->base_dest_offset.x : 0;
			dest_offset.y = (dynamic_info->source_offset.y == 0) ? static_info->base_dest_offset.y : 0;
			*/
			dest_offset = dynamic_info->dest_offset;
			source_from = dynamic_info->source_offset;
			if (static_info->context->last_text_size.cx - dynamic_info->source_offset.x + dest_offset.x < static_info->page_size.cx) {
				source_to.x = static_info->context->last_text_size.cx;
			} else {
				source_to.x = dynamic_info->source_offset.x + static_info->page_size.cx;
				if (neighbor_x != nullptr) {
					DYNAMIC_WRITE_INFO next_dynamic_info;
					next_dynamic_info.source_offset = POINT { source_to.x, dynamic_info->source_offset.y };
					next_dynamic_info.dest_offset = POINT{ 0, dynamic_info->dest_offset.y };
					neighbor_x->writeline(static_info, &next_dynamic_info);
				}
			}
			if (static_info->context->last_text_size.cy - dynamic_info->source_offset.y + dest_offset.y < static_info->page_size.cy) {
				source_to.y = static_info->context->last_text_size.cy;
			} else {
				source_to.y = dynamic_info->source_offset.y + static_info->page_size.cy;
				// source_to.y = dynamic_info->
				if (neighbor_y != nullptr) {
					DYNAMIC_WRITE_INFO next_dynamic_info;
					next_dynamic_info.source_offset = POINT{ dynamic_info->source_offset.x, source_to.y };
					next_dynamic_info.dest_offset = POINT{ dynamic_info->dest_offset.x, 0 };
					neighbor_y->writeline(static_info, &next_dynamic_info);
				}
			}
		}
		printf(
			"(page %d) source: (%d, %d)->(%d, %d) | dest: (%d, %d)->(%d, %d) | size: (%d, %d)\n", pageID,
			source_from.x, source_from.y, source_to.x, source_to.y,
			dest_offset.x, dest_offset.y, dest_offset.x + source_to.x - source_from.x, dest_offset.y + source_to.y - source_from.y,
			source_to.x - source_from.x, source_to.y - source_from.y
		);
		/*
    	bool use_heap = size_t(static_info->context->last_text_size.cx) * static_info->context->last_text_size.cy > HEAP_TIPPING_POINT;
    	size_t R8size = size_t(source_to.y - source_from.y) * (source_to.x - source_from.x) * 2;
    	BYTE* R8data;
    	if (use_heap) R8data = (BYTE*)HeapAlloc((static_info->hheap == NULL) ? GetProcessHeap() : static_info->hheap, 0, R8size);
    	else R8data = (BYTE*)__builtin_alloca(R8size);
	    size_t R8data_counter = 0;
	    for (int32_t y = source_from.y; y < source_to.y; y++) {
	        size_t line_offset = y * static_info->context->BMPsize.cx * 4;
	        for (int32_t x = source_from.x; x < source_to.x; x++) {
	            BYTE pixel = static_info->context->BMPdata[line_offset + x * 4];
        		R8data[R8data_counter++] = 255; // L
        		R8data[R8data_counter++] = pixel; // A
			}
	    }
	    glBindTexture(GL_TEXTURE_2D, textureID);
	    glTexSubImage2D(
			GL_TEXTURE_2D, 0, dest_offset.x, dest_offset.y, source_to.x - source_from.x, source_to.y - source_from.y,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, R8data);
	    if (use_heap) HeapFree((static_info->hheap == NULL) ? GetProcessHeap() : static_info->hheap, 0, R8data);
	    */
	}
	void render(POINT destination, SIZE page_size, SIZE window_size) {
		if (destination.x < window_size.cx && destination.y < window_size.cy) {
			POINT next_destination = POINT{ destination.x + page_size.cx, destination.y + page_size.cy };
			glBindTexture(GL_TEXTURE_2D, textureID);
			/*
			COLORREF randcolor = gradientRGB((uint16_t)rand());
			glColor4ub(GetRValue(randcolor), GetGValue(randcolor), GetBValue(randcolor), 255);
			*/
			glColor4ub(255, 255, 255, 255); // text color
			glBegin(GL_QUADS);
				glTexCoord2f(0.f, 0.f); glVertex2i(     destination.x,      destination.y);
        		glTexCoord2f(1.f, 0.f); glVertex2i(next_destination.x,      destination.y);
        		glTexCoord2f(1.f, 1.f); glVertex2i(next_destination.x, next_destination.y);
        		glTexCoord2f(0.f, 1.f); glVertex2i(     destination.x, next_destination.y);
        	glEnd();
        	if (neighbor_x != nullptr) neighbor_x->render(POINT{ next_destination.x, destination.y }, page_size, window_size);
        	if (neighbor_y != nullptr) neighbor_y->render(POINT{ destination.x, next_destination.y }, page_size, window_size);
		}
	}
};


struct TEST_PROC_PARAMS {
	CHARPAGE* page_root;
	SIZE      page_size;
};
void test_proc(GL_WINDOW* root, void* params) {
	GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
    	printf("GL error before clear: 0x%04X\n", err);
    	Sleep(1000);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	root->switch_dimension(2);
	glBegin(GL_QUADS);
		glColor4ub(064, 064, 000, 255);
		glVertex2i(00, 00);
        glVertex2i(64, 00);
        glVertex2i(64, 64);
        glVertex2i(00, 64);
        glColor3ub(000, 064, 064);
		glVertex2i(64, 64);
        glVertex2i(128, 64);
        glVertex2i(128, 128);
        glVertex2i(64, 128);
        glColor3ub(064, 000, 064);
		glVertex2i(64, 0);
        glVertex2i(128, 0);
        glVertex2i(128, 64);
        glVertex2i(64, 64);
    glEnd();

	glEnable(GL_TEXTURE_2D); glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ((TEST_PROC_PARAMS*)params)->page_root->render(POINT{ 0, 0 }, ((TEST_PROC_PARAMS*)params)->page_size, SIZE{ root->window_size_x, root->window_size_y });

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
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

    SIZE page_size = SIZE{ 64, 64 };
    CHARPAGE page0(page_size, 0), page1(page_size, 1), page2(page_size, 2), page3(page_size, 3), page4(page_size, 4), page5(page_size, 5);
    /* [0]--[1]--[2]--[\]
	 *  |
	 * [3]--[4]--[5]--[\]
	 *  |
	 * [\]
    */
    page0.neighbor_x = &page1;
	page0.neighbor_y = &page3;
    page1.neighbor_x = &page2;
    page3.neighbor_x = &page4;
    page4.neighbor_x = &page5;
    
    CHARPAGE::STATIC_WRITE_INFO static_info;
	// wchar_t text0[] = L"Very long string......xxxxxxxxxxxxxxxxxxx";
	// wchar_t text0[] = L"Not so long";
	wchar_t text0[] = L"......String very long";
    static_info.text = text0;
    static_info.context = &font_context;
    static_info.page_size = page_size;
    static_info.base_dest_offset = POINT{ 10, 5 };
    static_info.hheap = NULL;
    static_info.hfont = hfont;
    
    CHARPAGE::DYNAMIC_WRITE_INFO dynamic_info = CHARPAGE::DYNAMIC_WRITE_INFO::get_header_info(&static_info);
    page0.writeline(&static_info, &dynamic_info);

	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	wchar_t text1[] = L"......String very long";
	static_info.text = text1;
	static_info.base_dest_offset = POINT{ 10, 55 };
	dynamic_info = CHARPAGE::DYNAMIC_WRITE_INFO::get_header_info(&static_info);
    page0.writeline(&static_info, &dynamic_info);

	// top.set_render_proc(GL_WINDOW::render_frame_demo);
	top.set_render_proc(test_proc);
	TEST_PROC_PARAMS params;
	params.page_root = &page0;
	params.page_size = page_size;
	top.set_additional_render_info(&params);
	top.mainloop();

	DeleteObject(hfont);
	return 0;
}
