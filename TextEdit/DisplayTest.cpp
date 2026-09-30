#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    	case WM_CREATE: {
        	MARGINS margins = { 0, 0, 50, 0 };
        	DwmExtendFrameIntoClientArea(hwnd, &margins);
        	return 0;
    	} case WM_ERASEBKGND: return TRUE;
    	case WM_PAINT: {
    	    PAINTSTRUCT ps;
    	    HDC hdc = BeginPaint(hwnd, &ps);
    	    RECT rc;
    	    GetClientRect(hwnd, &rc);
    	    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
			rc.bottom = 50;
			FillRect(hdc, &rc, hBrush);
			DeleteObject(hBrush);
			EndPaint(hwnd, &ps);return 0;
    	} case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"BlurWindow";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, L"BlurWindow", L"DWM Blur Demo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInst, NULL);
    ShowWindow(hwnd, nCmdShow);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return msg.wParam;
}
