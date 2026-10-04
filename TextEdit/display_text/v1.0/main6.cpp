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

bool operator==(const POINT& p1, const POINT& p2) { return (p1.x == p2.x) && (p1.y == p2.y); }
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
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &text_zone, text, textlen, nullptr);
	}
};

struct CHARPAGE {
	struct WRITE_INFO {
		wchar_t*                text;
		FONT_CONTEXT*           context;
		SIZE                    page_size;
		POINT                   base_dest_offset;
		HANDLE                  hheap;
		HFONT                   hfont;
		DICT<POINT, CHARPAGE*>* pages;
		WRITE_INFO(): pages(nullptr) { }
		~WRITE_INFO() {
			if (pages != nullptr) delete pages;
			pages = nullptr;
		}
		void create_index(HANDLE hheap = NULL) { pages = new DICT<POINT, CHARPAGE*>(0, 0.f, 0.f, hheap); }
		void copy(const WRITE_INFO* another) {
			text      = another->text;
			context   = another->context;
			page_size = another->page_size;
			hheap     = another->hheap;
			hfont     = another->hfont;
		}
		void add_page(POINT page_location) {
			CHARPAGE* root = new CHARPAGE(page_size);
			CHARPAGE** neighbor;
			if (page_location.x > 0) {
				neighbor = pages->get(POINT{ page_location.x - 1, page_location.y });
				if (neighbor != nullptr) (*neighbor)->neighbor_x = root;
			} if (page_location.y > 0) {
				neighbor = pages->get(POINT{ page_location.x, page_location.y - 1 });
				if (neighbor != nullptr) (*neighbor)->neighbor_y = root;
			}
			neighbor = pages->get(POINT{ page_location.x + 1, page_location.y });
			if (neighbor != nullptr) root->neighbor_x = *neighbor;
			neighbor = pages->get(POINT{ page_location.x, page_location.y + 1 });
			if (neighbor != nullptr) root->neighbor_y = *neighbor;
		}
		void remove_page(POINT page_location) {
			CHARPAGE** neighbor;
			if (page_location.x > 0) {
				neighbor = pages->get(POINT{ page_location.x - 1, page_location.y });
				if (neighbor != nullptr) (*neighbor)->neighbor_x = nullptr;
			} if (page_location.y > 0) {
				neighbor = pages->get(POINT{ page_location.x, page_location.y - 1 });
				if (neighbor != nullptr) (*neighbor)->neighbor_y = nullptr;
			}
			CHARPAGE**& root = neighbor;
			root = pages->get(page_location);
			if (root != nullptr) glDeleteTextures(1, &((*root)->textureID));
		}
	};
	GLuint    textureID;
	CHARPAGE *neighbor_x, *neighbor_y;
	static constexpr size_t HEAP_TIPPING_POINT = 16384ULL;
	CHARPAGE(SIZE _size): neighbor_x(nullptr), neighbor_y(nullptr) { textureID = create_empty_texture(_size, GL_LUMINANCE_ALPHA); }
	void writeline(const WRITE_INFO* write_info, POINT page_offset = POINT{ 0, 0 }) {
		POINT source_from, source_displacement, dest_offset;
		if (page_offset.x == 0) {
			dest_offset.x = write_info->base_dest_offset.x;
			source_from.x = 0;
			if (page_offset.y == 0) { // First call!
				if (write_info->hfont != NULL) write_info->context->setfont(write_info->hfont);
				write_info->context->render(write_info->text);
				if (write_info->base_dest_offset.x >= write_info->page_size.cx || write_info->base_dest_offset.y >= write_info->page_size.cy) {
					CHARPAGE** actual_root = write_info->pages->get(
						POINT{ write_info->base_dest_offset.x / write_info->page_size.cx, write_info->base_dest_offset.y / write_info->page_size.cy });
					if (actual_root != nullptr) {
						WRITE_INFO actual_write_info;
						actual_write_info.copy(write_info);
						actual_write_info.base_dest_offset = POINT{
							write_info->base_dest_offset.x % write_info->page_size.cx, write_info->base_dest_offset.y % write_info->page_size.cy };
						(*actual_root)->writeline(&actual_write_info, POINT{ 0, 0 });
					}
					return;
				} else {
					dest_offset.y = write_info->base_dest_offset.y;
					source_from.y = 0;
				}
			} else {
				dest_offset.y = 0;
				source_from.y = page_offset.y - write_info->base_dest_offset.y;
			}
		} else {
			dest_offset.x = 0;
			dest_offset.y = (page_offset.y == 0) ? write_info->base_dest_offset.y : 0;
			source_from.x = page_offset.x - write_info->base_dest_offset.x;
		}
		if (page_offset.x == 0) {
			if (write_info->base_dest_offset.x + write_info->context->last_text_size.cx < write_info->page_size.cx)
				source_displacement.x = write_info->context->last_text_size.cx;
			else {
				source_displacement.x = write_info->page_size.cx - write_info->base_dest_offset.x;
				if (neighbor_x != nullptr) neighbor_x->writeline(write_info, POINT{ page_offset.x + write_info->page_size.cx, page_offset.y });
			}
		} else {
			if (page_offset.x - write_info->base_dest_offset.x + write_info->page_size.cx > write_info->context->last_text_size.cx)
				source_displacement.x = write_info->context->last_text_size.cx + write_info->base_dest_offset.x - page_offset.x;
			else {
				source_displacement.x = write_info->page_size.cx;
				if (neighbor_x != nullptr) neighbor_x->writeline(write_info, POINT{ page_offset.x + write_info->page_size.cx, page_offset.y });
			}
		} if (page_offset.y == 0) {
			if (write_info->base_dest_offset.y + write_info->context->last_text_size.cy < write_info->page_size.cy)
				source_displacement.y = write_info->context->last_text_size.cy;
			else {
				source_displacement.y = write_info->page_size.cy - write_info->base_dest_offset.y;
				if (page_offset.x == 0 && neighbor_y != nullptr)
					neighbor_y->writeline(write_info, POINT{ page_offset.x, page_offset.y + write_info->page_size.cy });
			}
		} else {
			if (page_offset.y - write_info->base_dest_offset.y + write_info->page_size.cy > write_info->context->last_text_size.cy)
				source_displacement.y = write_info->context->last_text_size.cy + write_info->base_dest_offset.y - page_offset.y;
			else {
				source_displacement.y = write_info->page_size.cy;
				if (page_offset.x == 0 && neighbor_y != nullptr)
					neighbor_y->writeline(write_info, POINT{ page_offset.x, page_offset.y + write_info->page_size.cy });
			}
		}
    	bool use_heap = size_t(write_info->context->last_text_size.cx) * write_info->context->last_text_size.cy > HEAP_TIPPING_POINT;
    	size_t R8size = size_t(source_displacement.y) * source_displacement.x * 2;
    	BYTE* R8data;
    	if (use_heap) R8data = (BYTE*)HeapAlloc((write_info->hheap == NULL) ? GetProcessHeap() : write_info->hheap, 0, R8size);
    	else R8data = (BYTE*)__builtin_alloca(R8size);
	    size_t R8data_counter = 0;
	    POINT source_to = POINT{ source_from.x + source_displacement.x, source_from.y + source_displacement.y };
	    for (int32_t y = source_from.y; y < source_to.y; y++) {
	        size_t line_offset = y * write_info->context->BMPsize.cx * 4;
	        for (int32_t x = source_from.x; x < source_to.x; x++) {
        		R8data[R8data_counter++] = 255; // L
        		R8data[R8data_counter++] = write_info->context->BMPdata[line_offset + x * 4]; // A
			}
	    }
	    glBindTexture(GL_TEXTURE_2D, textureID);
	    glTexSubImage2D(
			GL_TEXTURE_2D, 0, dest_offset.x, dest_offset.y, source_displacement.x, source_displacement.y,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, R8data);
	    if (use_heap) HeapFree((write_info->hheap == NULL) ? GetProcessHeap() : write_info->hheap, 0, R8data);
	}
	void render(POINT destination, SIZE page_size, SIZE window_size, bool VTcall = true) {
		if (destination.x < window_size.cx && destination.y < window_size.cy) {
			POINT next_destination = POINT{ destination.x + page_size.cx, destination.y + page_size.cy };
			glBindTexture(GL_TEXTURE_2D, textureID);
			/*
			COLORREF randcolor = gradientRGB((uint16_t)rand());
			glColor4ub(GetRValue(randcolor), GetGValue(randcolor), GetBValue(randcolor), 255);
			*/
			glColor4ub(255, 255, 255, 255); // Text color
			glBegin(GL_QUADS);
				glTexCoord2f(0.f, 0.f); glVertex2i(     destination.x,      destination.y);
        		glTexCoord2f(1.f, 0.f); glVertex2i(next_destination.x,      destination.y);
        		glTexCoord2f(1.f, 1.f); glVertex2i(next_destination.x, next_destination.y);
        		glTexCoord2f(0.f, 1.f); glVertex2i(     destination.x, next_destination.y);
        	glEnd();
        	if (neighbor_x != nullptr) neighbor_x->render(POINT{ next_destination.x, destination.y }, page_size, window_size, false);
        	if (VTcall && neighbor_y != nullptr) neighbor_y->render(POINT{ destination.x, next_destination.y }, page_size, window_size, true);
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
    CHARPAGE page0(page_size), page1(page_size), page2(page_size), page3(page_size), page4(page_size), page5(page_size);
    /* [0]--[1]--[2]--[\]
	 *  |    |    |
	 * [3]--[4]--[5]--[\]
	 *  |    |    |
	 * [\]  [\]  [\]
    */
    page0.neighbor_x = &page1; page0.neighbor_y = &page3;
    page1.neighbor_x = &page2; page1.neighbor_y = &page4;
	page2.neighbor_y = &page5;
    page3.neighbor_x = &page4;
    page4.neighbor_x = &page5;

    CHARPAGE::WRITE_INFO write_info;
	wchar_t text0[] = L"Very long string......";
    write_info.text = text0;
    write_info.context = &font_context;
    write_info.page_size = page_size;
    write_info.base_dest_offset = POINT{ 10, 5 };
    write_info.hheap = NULL;
    write_info.hfont = hfont;
    write_info.create_index();
    write_info.pages->set(POINT{ 0, 0 }, &page0);
    write_info.pages->set(POINT{ 1, 0 }, &page1);
    write_info.pages->set(POINT{ 2, 0 }, &page2);
    write_info.pages->set(POINT{ 0, 1 }, &page3);
	write_info.pages->set(POINT{ 1, 1 }, &page4);
	write_info.pages->set(POINT{ 2, 1 }, &page5);

    page0.writeline(&write_info);

	wchar_t text1[] = L"......String very long";
	write_info.text = text1;
	write_info.base_dest_offset = POINT{ 10, 55 };
    page0.writeline(&write_info);

    wchar_t text2[] = L"QWERTYUIOPASDFGHJKLZXCVBNM";
    write_info.text = text2;
    write_info.base_dest_offset = POINT{ 10, 75 };
    page0.writeline(&write_info);

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
