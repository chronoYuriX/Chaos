#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define _WIN32_WINNT 0x0601

#include <windows.h>
#if _WIN32_WINNT >= 0x0600
	#include <dwmapi.h>
#endif
#include <mmsystem.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdint.h>
#include <stdio.h>

#include "cyxlib_source/data-structs.cpp"
#include "cyxlib_source/non-crt-math.cpp"
#include "cyxlib_source/GL-GDI/root.cpp"
#include "cyxlib_source/GL-GDI/EXtextpage.cpp"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")

int32_t g_blur_height = 50;
HWND g_hgraphic = NULL;

struct TEST_PROC_PARAMS {
	CHARPAGE::WRITE_INFO* write_info;
	POINT                 offset = POINT{ 0, 0 };
};
bool test_proc(GRAPHIC_WINDOW* root, UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result) {
	switch (message) {
        case WM_CREATE: break;
        case WM_CLOSE: DestroyWindow(root->hwnd); break;
        case WM_DESTROY: PostQuitMessage(0); break;
        case WM_ERASEBKGND: *result = TRUE; break;
        case WM_SIZE: {
        	if (wparam != SIZE_MINIMIZED) root->resize(SIZE{ (LONG)LOWORD(lparam), (LONG)HIWORD(lparam) }, false);
			break;
		} case GRAPHIC_WINDOW::MESSAGE_RENDER: {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			root->switch_dimension(2);
			glEnable(GL_TEXTURE_2D); glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			{
				TEST_PROC_PARAMS& params_ref = *(TEST_PROC_PARAMS*)lparam;
    			CHARPAGE::render(params_ref.offset, params_ref.write_info, root->window_size);
			}
    		glDisable(GL_TEXTURE_2D);
    		glEnable(GL_BLEND);
    		glFlush();
    		SwapBuffers(root->hDC);
			break;
		}
        default: return GRAPHIC_WINDOW::USE_DEFAULT_PROCESS;
    }
    return GRAPHIC_WINDOW::PASS_DEFAULT_PROCESS;
}

LRESULT CALLBACK shell_window_process(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    	case WM_CREATE: {
    		#if _WIN32_WINNT >= 0x0600
        		MARGINS margins = { 0, 0, g_blur_height, 0 };
        		DwmExtendFrameIntoClientArea(hwnd, &margins);
			#endif
        	break;
    	} case WM_ERASEBKGND: return TRUE;
    	case WM_SIZE: {
    		RECT client_rect;
    		GetClientRect(hwnd, &client_rect);
    		if (g_hgraphic != NULL) MoveWindow(g_hgraphic, 0, g_blur_height, client_rect.right, client_rect.bottom - g_blur_height, TRUE);
			break;
		} case WM_GETMINMAXINFO: {
    		MINMAXINFO& MMI = *(MINMAXINFO*)lParam;
    		RECT min_rect = { 0, 0, 500, 350 };
    		AdjustWindowRectEx(&min_rect, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
    		MMI.ptMinTrackSize.x = min_rect.right  - min_rect.left;
    		MMI.ptMinTrackSize.y = min_rect.bottom - min_rect.top;
    		break;
		} case WM_PAINT: {
    	    PAINTSTRUCT ps;
    	    HDC hdc = BeginPaint(hwnd, &ps);
    	    RECT rc;
    	    GetClientRect(hwnd, &rc);
    	    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
			rc.bottom = g_blur_height;
			FillRect(hdc, &rc, hBrush);
			DeleteObject(hBrush);
			EndPaint(hwnd, &ps);
			break;
    	} case WM_DESTROY: PostQuitMessage(0); break;
    	default: return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR, int ) {
	#if _WIN32_WINNT >= 0x0600
		SetProcessDPIAware();
	#endif
	DWORD PID = GetCurrentProcessId(), tick = GetTickCount();
    srand(PID ^ tick);
    // @shell window
    WNDCLASSW wndclass = { 0 };
    wndclass.lpfnWndProc   = shell_window_process;
    wndclass.hInstance     = hinstance;
    wndclass.lpszClassName = L"CYX_SHELL_12";
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wndclass);
    HWND hwnd = CreateWindowExW(
		0, L"CYX_SHELL_12", L"Shell Window",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hinstance, NULL);
	// @subwindow
	GRAPHIC_WINDOW::WINDOW_ATTRIBUTES window_attributes;
    window_attributes.set_default();
    window_attributes.window_size = SIZE{ 500, 300 };
    window_attributes.window_classname_header = L"GRAPHICS_DEMO_12_";
    window_attributes.window_title = L"Demo";
    window_attributes.client_process = test_proc;
    window_attributes.window_style_basic = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
    window_attributes.parent_window = hwnd;
    window_attributes.auto_locate = false;
    window_attributes.window_location = POINT{ 0, g_blur_height };
    window_attributes.maxFPS = 10.f;
	// @font
    LOGFONT LF = create_default_logfont();
    HFONT hfont = CreateFontIndirectW(&LF);
    FONT_CONTEXT font_context;
    font_context.setfont(hfont);
    SIZE page_size = SIZE{ 64, 64 };
    CHARPAGE::WRITE_INFO write_info;
    TEST_PROC_PARAMS params;
	params.write_info = &write_info;
	window_attributes.additional_info = (void*)&params;
    GRAPHIC_WINDOW graphic_window(&window_attributes, nullptr);
    g_hgraphic = graphic_window.hwnd;
    // @text 0
	wchar_t text0[] = L"Very long string......";
    write_info.text = text0;
    write_info.context = &font_context;
    write_info.page_size = page_size;
    write_info.base_dest_offset = POINT{ 10, 5 };
    write_info.hheap = NULL;
    write_info.hfont = hfont;
    write_info.color = RGB(128, 255, 127);
    write_info.create_index();
    for (int32_t x = 0; x < 4; x++) for (int32_t y = 0; y < 3; y++) write_info.add_page(POINT{ x, y });
	CHARPAGE* root_page = *(write_info.pages->get(POINT{ 0, 0 }));
    root_page->writeline(&write_info);
	// @text 1
	wchar_t text1[] = L"......String very long";
	write_info.text = text1;
	write_info.base_dest_offset = POINT{ 10, 55 };
    root_page->writeline(&write_info);
	// @text 2
    wchar_t text2[] = L"QWERTYUIOPASDFGHJKLZXCVBNM";
    write_info.text = text2;
    write_info.base_dest_offset = POINT{ 70, 75 };
    root_page->writeline(&write_info);
	// @main
    ShowWindow(hwnd, SW_SHOW);
    MSG message;
    DWORD last_render = GetTickCount(), current_tick;
    while (true) {
        if (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) break;
            else {
            	TranslateMessage(&message);
        	    DispatchMessageW(&message);
        	}
        } else if ((current_tick = GetTickCount()) >= last_render + 33) {
        	graphic_window.update();
        	last_render = current_tick;
		} else Sleep(10);
    }
	DeleteObject(hfont);
    return message.wParam;
}
