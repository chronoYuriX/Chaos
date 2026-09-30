// main.cpp
// 编译：cl /EHsc main.cpp user32.lib gdi32.lib dwmapi.lib
// 或 VS 直接建 Win32 项目替换 main

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <dwmapi.h>
#include <algorithm>
#include <vector>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 500;
constexpr int PANEL_W = 260;      // 毛玻璃面板宽度

// 简单 3x3 高斯核（演示用；生产建议分离 + 降采样）
static const float GAUSS3[3][3] = {
    {0.0625f, 0.125f, 0.0625f},
    {0.125f,  0.25f,  0.125f},
    {0.0625f, 0.125f, 0.0625f}
};

bool g_aero = false;
HBITMAP g_bgBitmap = nullptr;  // 截屏缓存

// ---------- 工具函数 ----------

bool IsAeroEnabled()
{
    BOOL comp = FALSE;
    if (SUCCEEDED(DwmIsCompositionEnabled(&comp)) && comp)
        return true;
    return false;
}

// 截取屏幕某区域到 HBITMAP
HBITMAP CaptureScreen(int x, int y, int w, int h)
{
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmp = CreateCompatibleBitmap(hdcScreen, w, h);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hbmp);
    BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);
    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    return hbmp;
}

// 对 HBITMAP 做 3x3 高斯模糊（原地）
void GaussianBlurBitmap(HBITMAP hbmp, int w, int h)
{
    if (!hbmp) return;

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;  // 自上而下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<COLORREF> src(w * h);
    std::vector<COLORREF> dst(w * h);

    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hbmp, 0, h, src.data(), &bmi, DIB_RGB_COLORS);

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            float r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    COLORREF c = src[(y + dy) * w + (x + dx)];
                    float k = GAUSS3[dy + 1][dx + 1];
                    r += GetRValue(c) * k;
                    g += GetGValue(c) * k;
                    b += GetBValue(c) * k;
                }
            }
            dst[y * w + x] = RGB((BYTE)r, (BYTE)g, (BYTE)b);
        }
    }
    // 边缘直接复制
    for (int x = 0; x < w; ++x) {
        dst[x] = src[x];
        dst[(h - 1) * w + x] = src[(h - 1) * w + x];
    }
    for (int y = 0; y < h; ++y) {
        dst[y * w] = src[y * w];
        dst[y * w + w - 1] = src[y * w + w - 1];
    }

    SetDIBits(hdc, hbmp, 0, h, dst.data(), &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);
}

// ---------- 窗口过程 ----------

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_aero = IsAeroEnabled();

        if (g_aero) {
            // 方案 A：DWM 系统毛玻璃
            DWM_BLURBEHIND bb = {0};
            bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
            bb.fEnable = TRUE;
            HRGN hRgn = CreateRectRgn(0, 0, PANEL_W, WINDOW_H);
            bb.hRgnBlur = hRgn;
            DwmEnableBlurBehindWindow(hwnd, &bb);
            DeleteObject(hRgn);
        } else {
            // 方案 B：分层窗口 + 截屏模糊
            SetWindowLong(hwnd, GWL_EXSTYLE,
                GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            g_bgBitmap = CaptureScreen(0, 0, PANEL_W, WINDOW_H);
            GaussianBlurBitmap(g_bgBitmap, PANEL_W, WINDOW_H);
        }
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;  // 不擦背景，防止盖住 DWM 玻璃

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 主区域（右侧）画成深色不透明
        RECT mainRect = { PANEL_W, 0, WINDOW_W, WINDOW_H };
        HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdc, &mainRect, hBrush);
        DeleteObject(hBrush);

        // 画一些文字
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(200, 200, 200));
        DrawText(hdc, L"main zone (solid)", -1, &mainRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        if (!g_aero && g_bgBitmap) {
            // 非 Aero：把模糊后的截图画到左侧面板
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, g_bgBitmap);
            BitBlt(hdc, 0, 0, PANEL_W, WINDOW_H, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hOld);
            DeleteDC(hdcMem);

            // 半透明遮罩（模拟毛玻璃效果）
            BLENDFUNCTION bf = {0};
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = 120;  // 半透明
            AlphaBlend(hdc, 0, 0, PANEL_W, WINDOW_H,
                       hdc, 0, 0, PANEL_W, WINDOW_H, bf);
        }

        // 面板文字
        RECT panelRect = { 0, 0, PANEL_W, WINDOW_H };
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawText(hdc, L"glass", -1, &panelRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DWMCOMPOSITIONCHANGED:
    {
        // 用户切换 Aero/Basic 时
        bool nowAero = IsAeroEnabled();
        if (nowAero != g_aero) {
            g_aero = nowAero;
            if (g_aero) {
                SetWindowLong(hwnd, GWL_EXSTYLE,
                    GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
                DWM_BLURBEHIND bb = {0};
                bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
                bb.fEnable = TRUE;
                HRGN hRgn = CreateRectRgn(0, 0, PANEL_W, WINDOW_H);
                bb.hRgnBlur = hRgn;
                DwmEnableBlurBehindWindow(hwnd, &bb);
                DeleteObject(hRgn);
            } else {
                SetWindowLong(hwnd, GWL_EXSTYLE,
                    GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                if (g_bgBitmap) DeleteObject(g_bgBitmap);
                g_bgBitmap = CaptureScreen(0, 0, PANEL_W, WINDOW_H);
                GaussianBlurBitmap(g_bgBitmap, PANEL_W, WINDOW_H);
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_DESTROY:
        if (g_bgBitmap) DeleteObject(g_bgBitmap);
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_NCHITTEST:
        // 无边框窗口可拖动
        return HTCAPTION;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// ---------- 入口 ----------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"BlurDemo";
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED, wc.lpszClassName, L"Blur Demo",
        WS_POPUP,
        100, 100, WINDOW_W, WINDOW_H,
        NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
