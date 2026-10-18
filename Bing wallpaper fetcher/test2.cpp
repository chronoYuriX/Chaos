#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

int main() {
	std::wcout << L"sizeof(wchar_t) = " << sizeof(wchar_t) << std::endl;
    // 1. Session
    HINTERNET hSession = WinHttpOpen(
        L"BingWallpaper/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession) {
        std::wcerr << L"WinHttpOpen failed: " << GetLastError() << std::endl;
        return 1;
    }

    // 2. Connect
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"cn.bing.com",
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );
    if (!hConnect) {
        std::wcerr << L"WinHttpConnect failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 3. Open request
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        L"/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=zh-CN",
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );
    if (!hRequest) {
        std::wcerr << L"WinHttpOpenRequest failed: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    std::string jsonBody;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    if (!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    )) {
        std::wcerr << L"WinHttpSendRequest failed: " << GetLastError() << std::endl;
        goto cleanup;
    }

    // 5. Receive response
    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        std::wcerr << L"WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
        goto cleanup;
    }

    do {
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
            std::wcerr << L"WinHttpQueryDataAvailable failed" << std::endl;
            break;
        }
        if (bytesAvailable == 0) break;

        std::vector<char> buffer(bytesAvailable + 1, 0);
        if (!WinHttpReadData(
            hRequest,
            buffer.data(),
            bytesAvailable,
            &bytesRead
        )) {
            std::wcerr << L"WinHttpReadData failed" << std::endl;
            break;
        }

        jsonBody.append(buffer.data(), bytesRead);
    } while (bytesAvailable > 0);

    // 7. Output result
    std::cout << "JSON length: " << jsonBody.size() << std::endl;
    std::cout << jsonBody << std::endl;

	cleanup:
    	if (hRequest) WinHttpCloseHandle(hRequest);
    	if (hConnect) WinHttpCloseHandle(hConnect);
    	if (hSession) WinHttpCloseHandle(hSession);

    return 0;
}
