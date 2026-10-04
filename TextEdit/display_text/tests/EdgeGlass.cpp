#include <windows.h>
#include <dwmapi.h>
#include <bits/stdc++.h>

// 标签页数据
struct TabInfo {
    std::wstring title;
    RECT rect;
    bool active;
};

const int TAB_HEIGHT = 35;
const int TAB_WIDTH = 160;
const int TAB_GAP = 5;
const int WINDOW_BTN_WIDTH = 45;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static std::vector<TabInfo> tabs;
    static int activeTab = 0;

    switch (msg)
    {
    case WM_CREATE:
    {
        // 初始化标签页
        TabInfo tab;
        tab.title = L"New tab";
        tab.active = true;
        tabs.push_back(tab);

        // 扩展 DWM 玻璃效果
        MARGINS margins = { 0, 0, TAB_HEIGHT + 10, 0 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        // 设置窗口为可调整大小
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        break;
    }

    case WM_NCCALCSIZE:
    {
        if (wParam == TRUE)
        {
        	/*
            // 吃掉标题栏区域
            NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
            RECT original = params->rgrc[0];
            */
/*
            // 保留边框宽度用于调整大小
            RECT border = {0};
            AdjustWindowRectEx(&border, WS_THICKFRAME, FALSE, 0);

            params->rgrc[0] = original;
            params->rgrc[0].left += border.left;
            params->rgrc[0].top += border.top;
            params->rgrc[0].right += border.right;
            params->rgrc[0].bottom += border.bottom;
*/
            return 0;
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);

        // 背景
        //FillRect(hdc, &client, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // 标签栏背景

        RECT tabBar = { 0, 0, client.right, TAB_HEIGHT + 10 };
        HBRUSH tabBarBrush = CreateSolidBrush(RGB(45, 45, 48));
        FillRect(hdc, &tabBar, tabBarBrush);
        DeleteObject(tabBarBrush);


        // 画标签页
        for (size_t i = 0; i < tabs.size(); i++)
        {
            RECT tabRect = {
                10 + i * (TAB_WIDTH + TAB_GAP),
                5,
                10 + i * (TAB_WIDTH + TAB_GAP) + TAB_WIDTH,
                5 + TAB_HEIGHT
            };
            tabs[i].rect = tabRect;

            // 标签背景
            HBRUSH tabBrush = CreateSolidBrush(
                tabs[i].active ? RGB(60, 60, 64) : RGB(45, 45, 48)
            );
            FillRect(hdc, &tabRect, tabBrush);
            DeleteObject(tabBrush);

            // 标签文字
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextW(hdc, tabs[i].title.c_str(), -1, &tabRect,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // 关闭按钮
            RECT closeRect = { tabRect.right - 20, tabRect.top + 5,
                               tabRect.right - 5, tabRect.bottom - 5 };
            DrawTextW(hdc, L"X", -1, &closeRect,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_NCHITTEST:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hwnd, &pt);

        RECT client;
        GetClientRect(hwnd, &client);

        // 窗口控制按钮区域
        if (pt.y < TAB_HEIGHT + 10)
        {
            if (pt.x > client.right - WINDOW_BTN_WIDTH)
                return HTCLOSE;
            if (pt.x > client.right - WINDOW_BTN_WIDTH * 2)
                return HTMAXBUTTON;
            if (pt.x > client.right - WINDOW_BTN_WIDTH * 3)
                return HTMINBUTTON;

            // 标签区域
            for (auto& tab : tabs)
            {
                if (PtInRect(&tab.rect, pt))
                    return HTCLIENT;
            }

            // 空白区域 → 允许拖动
            return HTCAPTION;
        }

        return HTCLIENT;
    }

    case WM_LBUTTONDOWN:
    {
        // 处理标签点击、关闭按钮等
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };

        for (size_t i = 0; i < tabs.size(); i++)
        {
            if (PtInRect(&tabs[i].rect, pt))
            {
                // 检查是否点击了关闭按钮
                RECT closeRect = { tabs[i].rect.right - 20, tabs[i].rect.top + 5,
                                   tabs[i].rect.right - 5, tabs[i].rect.bottom - 5 };
                if (PtInRect(&closeRect, pt))
                {
                    // 关闭标签
                    tabs.erase(tabs.begin() + i);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else
                {
                    // 激活标签
                    for (auto& t : tabs) t.active = false;
                    tabs[i].active = true;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // 注册窗口类
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = L"CustomTitleBarWindow";
    RegisterClassExW(&wc);

    // 创建窗口（去掉 WS_CAPTION）
    HWND hwnd = CreateWindowExW(
        0,
        L"CustomTitleBarWindow",
        L"",
        WS_OVERLAPPEDWINDOW & ~WS_CAPTION | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return msg.wParam;
}
