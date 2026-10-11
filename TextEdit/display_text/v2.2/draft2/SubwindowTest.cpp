#include <windows.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    /* 注册主窗口类 */
    WNDCLASS wc = {};
    wc.lpfnWndProc   = MainWndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "MainWndClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);   // 白色背景
    RegisterClass(&wc);

    /* 注册子窗口类 */
    WNDCLASS wcChild = {};
    wcChild.lpfnWndProc   = ChildWndProc;
    wcChild.hInstance     = hInst;
    wcChild.lpszClassName = "ChildWndClass";
    wcChild.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // 灰色背景
    RegisterClass(&wcChild);

    /* 创建主窗口 */
    HWND hwndMain = CreateWindowEx(
        0, "MainWndClass", "主窗口",
        WS_OVERLAPPEDWINDOW,
        100, 100, 640, 480,
        NULL, NULL, hInst, NULL
    );
    if (!hwndMain) return 0;

    /* 创建子窗口，坐标相对于主窗口客户区 */
    HWND hwndChild = CreateWindowEx(
        0, "ChildWndClass", "子窗口",
        WS_CHILD | WS_VISIBLE,
        20, 20, 300, 200,
        hwndMain, NULL, hInst, NULL
    );

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    /* 同一个消息循环即可，主窗口和子窗口都会收到消息 */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        MessageBox(GetParent(hwnd), "子窗口被点击了", "提示", MB_OK);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
