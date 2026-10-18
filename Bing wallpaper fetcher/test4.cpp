#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "winhttp.lib")

struct HTTPS_DOWNLOADER {
	HINTERNET  hsession, hconnect, hrequest;
	DWORD      error_code;
	wchar_t   *user_agent, *source_site, *object_name;
	static constexpr uint8_t STR_USER_AGENT = 1, STR_SOURCE_SITE = 2, STR_OBJECT_NAME = 3;
	HTTPS_DOWNLOADER(): hsession(NULL), hconnect(NULL), hrequest(NULL), user_agent(nullptr), source_site(nullptr), object_name(nullptr) { }
	~HTTPS_DOWNLOADER() {
		if (hrequest != NULL) WinHttpCloseHandle(hrequest);
		if (hconnect != NULL) WinHttpCloseHandle(hconnect);
		if (hsession != NULL) WinHttpCloseHandle(hsession);
		if (user_agent != nullptr)  free(user_agent);
		if (source_site != nullptr) free(source_site);
		if (object_name != nullptr) free(object_name);
	}
	void set_string_object(uint8_t object_type, const wchar_t* new_object) {
		if (new_object == nullptr) return;
		size_t new_object_length = wcslen(new_object);
		wchar_t* local_object = (wchar_t*)malloc((new_object_length + 1) * sizeof(wchar_t));
		local_object[new_object_length] = L'\0';
		wcscpy(local_object, new_object);
		wchar_t** dest_object;
		switch (object_type) {
			case STR_USER_AGENT:  dest_object = &user_agent;  break;
			case STR_SOURCE_SITE: dest_object = &source_site; break;
			case STR_OBJECT_NAME: dest_object = &object_name;
		}
		if (*dest_object != nullptr) free(dest_object);
		*dest_object = local_object;
	}
	void open_session() {
		if (hsession != NULL || user_agent == nullptr) return;
		hsession = WinHttpOpen(user_agent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    	if (hsession == NULL) {
    		error_code = GetLastError();
    		return;
		}
    	DWORD prot = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
		WinHttpSetOption(hsession, WINHTTP_OPTION_SECURE_PROTOCOLS, &prot, sizeof(DWORD));
	}
	void connect() {
		if (hsession == NULL || source_site == NULL) return;
		hconnect = WinHttpConnect(hsession, source_site, INTERNET_DEFAULT_HTTPS_PORT, 0);
    	if (hconnect == NULL) {
        	error_code = GetLastError();
    		return;
    	}
	}
	void request() {
		hrequest = WinHttpOpenRequest(hconnect, L"GET", object_name, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    	if (hrequest == NULL) goto handle_error;
    	if (WinHttpSendRequest(hrequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) == FALSE) goto handle_error;
    	if (WinHttpReceiveResponse(hrequest, nullptr) == FALSE) goto handle_error;
    	return;
    handle_error:
    	error_code = GetLastError();
	}
	DWORD get_resonse_length() {
		if (hrequest == NULL) return 0;
		DWORD resonse_length;
		if (WinHttpQueryDataAvailable(hrequest, &resonse_length) == FALSE) return 0;
		return resonse_length;
	}
	void read_response(char* dest) {
		if (hrequest == NULL) return;
		DWORD total_size = get_resonse_length(), dest_counter = 0;
    	if (total_size == 0) goto handle_error;
    	while (total_size > 0) {
    	    char* output = (char*)malloc(total_size + 1);
    	    ZeroMemory(output, total_size + 1);
    	    DWORD current_size = 0;
			if (WinHttpReadData(hrequest, output, total_size, &current_size) == FALSE) {
            	free(output);
				goto handle_error;
        	}
			strncpy(dest + dest_counter, output, current_size);
			dest_counter += current_size;
        	free(output);
        	if (WinHttpQueryDataAvailable(hrequest, &total_size) == FALSE) goto handle_error;
    	}
    	return;
	handle_error:
    	error_code = GetLastError();
	}
};

int main() {
    HINTERNET hSession = WinHttpOpen(
        L"WinHttpExample/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if (!hSession) {
        printf("WinHttpOpen Ê§°Ü: %lu\n", GetLastError());
        return 1;
    }
    DWORD prot = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
	WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &prot, sizeof(prot));
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"cn.bing.com",
        INTERNET_DEFAULT_HTTPS_PORT, // 443
        0
    );
    if (!hConnect) {
        printf("WinHttpConnect Ê§°Ü: %lu\n", GetLastError());
        WinHttpCloseHandle(hSession);
        return 1;
    }
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        L"/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=zh-CN",
        nullptr, // HTTP/1.1
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE // HTTPS
    );
    if (!hRequest) {
        printf("WinHttpOpenRequest Ê§°Ü: %lu\n", GetLastError());
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }
    if (!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    )) {
        printf("WinHttpSendRequest Ê§°Ü: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        printf("WinHttpReceiveResponse Ê§°Ü: %lu\n", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

	char JSON_data[1024 * 4] = "\0";
    read_HTTP_response(hRequest, JSON_data);
    if (JSON_data[0]) {
        printf("=== ÏìÓ¦ÄÚÈÝ ===\n%s\n\n", JSON_data);
    }
    char* URL_start = strstr(JSON_data, "url");
    if (URL_start) {
    	URL_start += strlen("url\":\"");
    	char* URL_end = URL_start;
    	while (*URL_end != '\0' && *URL_end != '\"') URL_end++;
		*URL_end = '\0';
		printf("URL:\n%s\n", URL_start);

		char fullURL[512] = "https://cn.bing.com";
		strcat(fullURL, URL_start);
	}

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return 0;
}
