#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "winhttp.lib")

// 从响应中读取全部数据
std::string ReadResponse(HINTERNET hRequest) {
    std::string result;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;

    // 查询可用数据大小
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
        printf("WinHttpQueryDataAvailable 失败: %lu\n", GetLastError());
        return result;
    }

    while (dwSize > 0) {
        char* pszOutBuffer = new char[dwSize + 1];
        ZeroMemory(pszOutBuffer, dwSize + 1);

        if (!WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded)) {
            printf("WinHttpReadData 失败: %lu\n", GetLastError());
            delete[] pszOutBuffer;
            break;
        }
        result.append(pszOutBuffer, dwDownloaded);
        delete[] pszOutBuffer;

        // 继续读剩余数据
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            printf("WinHttpQueryDataAvailable 失败: %lu\n", GetLastError());
            break;
        }
    }
    return result;
}

int main() {
    // 1. 打开 WinHTTP 会话
    HINTERNET hSession = WinHttpOpen(
        L"WinHttpExample/1.0",           // User-Agent
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, // 使用系统代理
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession) {
        printf("WinHttpOpen 失败: %lu\n", GetLastError());
        return 1;
    }
    DWORD prot = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
	WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &prot, sizeof(prot));

    // 2. 连接到服务器
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"cn.bing.com",                   // 主机名
        INTERNET_DEFAULT_HTTPS_PORT,      // 443 端口
        0
    );
    if (!hConnect) {
        printf("WinHttpConnect 失败: %lu\n", GetLastError());
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 3. 创建请求
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        L"/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=zh-CN",  // 路径+查询参数
        NULL,                                 // HTTP/1.1
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE                  // HTTPS
    );
    if (!hRequest) {
        printf("WinHttpOpenRequest 失败: %lu\n", GetLastError());
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 4. 发送请求
    if (!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    )) {
        printf("WinHttpSendRequest 失败: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 5. 接收响应
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        printf("WinHttpReceiveResponse 失败: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 6. 读取 JSON 数据
    std::string json = ReadResponse(hRequest);
    if (!json.empty()) {
        printf("=== 响应内容 ===\n%s\n", json.c_str());
    }

    // 7. 清理句柄
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return 0;
}
