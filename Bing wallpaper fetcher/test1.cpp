#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string>
#include <fstream>

#pragma comment(lib, "winhttp.lib")

static std::wstring Utf8ToWide(const std::string& s) {
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], len);
    w.pop_back(); // remove null
    return w;
}

static std::string WideToUtf8(const std::wstring& w) {
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], len, nullptr, nullptr);
    s.pop_back();
    return s;
}

// 从 "key":"value" 里取第一个 value
static std::string JsonExtract(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos = json.find('"', pos + needle.size());
    if (pos == std::string::npos) return {};
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return json.substr(pos + 1, end - pos - 1);
}

static bool HttpGet(
    const std::wstring& host,
    const std::wstring& path,
    std::string& outBody
) {
    outBody.clear();

    HINTERNET hSession = WinHttpOpen(
        L"bing-wallpaper/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession) return false;

    HINTERNET hConn = WinHttpConnect(
        hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0
    );
    if (!hConn) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    HINTERNET hReq = WinHttpOpenRequest(
        hConn,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );
    if (!hReq) {
        WinHttpCloseHandle(hConn);
        WinHttpCloseHandle(hSession);
        return false;
    }

    bool ok = WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 nullptr, 0, 0, 0)
              && WinHttpReceiveResponse(hReq, nullptr);

    if (ok) {
        DWORD size, downloaded;
        char buf[4096];
        while (WinHttpReadData(hReq, buf, sizeof(buf), &downloaded) && downloaded) {
            outBody.append(buf, downloaded);
        }
    }

    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hSession);
    return ok;
}

int main() {
    // 1) 获取 Bing 元数据
    std::string meta;
    if (!HttpGet(L"cn.bing.com",
                 L"/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=zh-CN",
                 meta)) {
        printf("Failed to request Bing API\n");
        return 1;
    }

    std::string relUrl = JsonExtract(meta, "url");
    if (relUrl.empty()) {
        printf("Cannot find image url in JSON\n");
        return 1;
    }

    // 尝试升级为 UHD（不一定都有）
    std::string imgPath = relUrl;
    auto p = imgPath.find("_1920x1080");
    if (p != std::string::npos)
        imgPath.replace(p + 1, 9, "UHD");

    std::wstring imgHost = L"cn.bing.com";
    std::wstring imgPathW = Utf8ToWide(L"/" + imgPath);
    if (imgPathW[0] != L'/')
        imgPathW = L"/" + imgPathW;

    // 2) 下载图片
    std::string jpg;
    if (!HttpGet(imgHost, imgPathW, jpg) || jpg.size() < 1024) {
        printf("Failed to download image\n");
        return 1;
    }

    // 3) 保存到当前目录
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t name[64];
    swprintf_s(name, L"bing_%04d-%02d-%02d.jpg",
               st.wYear, st.wMonth, st.wDay);

    std::ofstream f(WideToUtf8(name), std::ios::binary);
    f.write(jpg.data(), jpg.size());
    f.close();

    printf("Saved: %ws (%zu bytes)\n", name, jpg.size());

    // 4) 设为桌面壁纸
    if (SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)name,
            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE)) {
        printf("Wallpaper set.\n");
    } else {
        printf("Set wallpaper failed, error=%lu\n", GetLastError());
    }

    return 0;
}
