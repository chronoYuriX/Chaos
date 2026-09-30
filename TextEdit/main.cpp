#ifndef _WINDOWS_
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
    #define UNICODE
	#include <windows.h>
#endif
#include <mmsystem.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdint.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")


template <typename TYPE_ELEMENT>
struct LIST {
	TYPE_ELEMENT* elements;
	size_t        storage_counter, max_storage;
	float         expansion;
	HANDLE        hheap;
	static constexpr float  DEFAULT_EXPANSION = 1.5f;
	static constexpr size_t DEFAULT_STORAGE = 64;
	LIST(size_t _max_storage = 0, float _expansion = 0.f, HANDLE _hheap = NULL):
			max_storage((_max_storage == 0) ? DEFAULT_STORAGE : _max_storage),
			expansion  ((_expansion <= 1.f) ? DEFAULT_EXPANSION : _expansion),
			hheap      ((_hheap == nullptr) ? GetProcessHeap() : _hheap) {
		elements = (TYPE_ELEMENT*)HeapAlloc(hheap, 0, max_storage * sizeof(TYPE_ELEMENT));
		cleanup();
	}
	~LIST() {
		if (elements != nullptr) HeapFree(hheap, 0, elements);
		elements = nullptr;
	}
	void append_p(TYPE_ELEMENT* pelement) {
		memcpy(elements + storage_counter, pelement, sizeof(TYPE_ELEMENT));
		storage_counter++;
	}
	inline void append(TYPE_ELEMENT element) { append_p(&element); }
	void resize(size_t max_storage_new) {
		if (max_storage_new != max_storage) {
			max_storage = max_storage_new;
			elements = (TYPE_ELEMENT*)HeapReAlloc(hheap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
		}
	}
	void expand() {
		size_t max_storage_new = max_storage * expansion;
		if (max_storage_new == max_storage) max_storage_new++;
		resize(max_storage_new);
	}
	void expand_to(size_t min_storage) {
		while (max_storage < min_storage) {
			size_t max_storage_temp = max_storage * expansion;
			if (max_storage_temp == max_storage) max_storage_temp++;
			max_storage = max_storage_temp;
		}
		elements = (TYPE_ELEMENT*)HeapReAlloc(hheap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
	}
	inline void cleanup() { storage_counter = 0; }
};

template <typename TYPE_KEY, typename TYPE_VAL>
struct DICT_ENTRY {
	TYPE_KEY key;
	TYPE_VAL val;
	uint8_t  state;
	DICT_ENTRY<TYPE_KEY, TYPE_VAL>* previous;
	static constexpr uint8_t FREE = 1, OCCUPIED = 2, DELETED = 3;
};
template <typename TYPE_KEY, typename TYPE_VAL>
using DICT_ENUM_FUNC = bool (*)(DICT_ENTRY<TYPE_KEY, TYPE_VAL>*, void*);
template <typename TYPE_KEY, typename TYPE_VAL>
struct DICT {
	#define ENTRY DICT_ENTRY<TYPE_KEY, TYPE_VAL>
	size_t    storage_counter,  max_storage;
	ENTRY    *entries,         *last_entry;
	float     expansion,        load_factor;
	HANDLE    hheap;
	static constexpr float DEFAULT_LOAD_FACTOR = .5f;
	static constexpr bool  ENUM_CONTINUE = false, ENUM_FINISH = true;
	DICT(size_t _max_storage = 0, float _expansion = 0.f, float _load_factor = 0.f, HANDLE _hheap = NULL):
			max_storage((_max_storage == 0) ? LIST<ENTRY>::DEFAULT_STORAGE : _max_storage),
			expansion  ((_expansion <= 1.f) ? LIST<ENTRY>::DEFAULT_EXPANSION : _expansion),
			load_factor((_load_factor > 1.f || _load_factor <= 0.f) ? DEFAULT_LOAD_FACTOR : _load_factor),
			hheap      ((_hheap == nullptr) ? GetProcessHeap() : _hheap) {
		entries = (ENTRY*)HeapAlloc(hheap, 0, max_storage * sizeof(ENTRY));
		cleanup();
	}
	~DICT() {
		if (entries != nullptr) HeapFree(hheap, 0, entries);
		entries = nullptr;
	}
	uint64_t _hash(const TYPE_KEY key) {
		uint64_t result = 14695981039346656037ULL;
		const uint8_t* key_bytes = (uint8_t*)&key;
		for (size_t i = 0; i < sizeof(key); i++) result = (result ^ key_bytes[i]) * 1099511628211ULL;
		return result % max_storage;
	}
	void set(const TYPE_KEY key, const TYPE_VAL val) {
		TYPE_VAL* target = get(key);
		if (target == nullptr) {
			if (storage_counter >= max_storage * load_factor) rebuild();
			uint64_t dest = _hash(key);
			while (entries[dest].state == ENTRY::OCCUPIED) dest = (dest + 1) % max_storage;
			ENTRY* current_entry = entries + dest;
			current_entry->previous = last_entry;
			current_entry->key = key;
			current_entry->val = val;
			current_entry->state = ENTRY::OCCUPIED;
			last_entry = current_entry;
			storage_counter++;
		} else *target = val;
	}
	ENTRY* get_entry(const TYPE_KEY key) {
		uint64_t dest = _hash(key); uint64_t start = dest;
		while (entries[dest].state != ENTRY::FREE) {
			if (entries[dest].key == key) {
				if (entries[dest].state == DICT_ENTRY<TYPE_KEY, TYPE_VAL>::OCCUPIED) return entries + dest;
				break;
			}
			dest = (dest + 1) % max_storage;
			if (dest == start) break;
		}
		return nullptr;
	}
	inline TYPE_VAL* get(const TYPE_KEY key) {
		ENTRY* target = get_entry(key);
		return (target == nullptr) ? nullptr : &(target->val);
	}
	void remove(const TYPE_KEY key) {
		ENTRY* target = get_entry(key);
		if (target != nullptr) target->state = ENTRY::DELETED;
	}
	void cleanup() {
		last_entry = nullptr;
		storage_counter = 0;
		for (size_t i = 0; i < max_storage; i++) entries[i].state = ENTRY::FREE;
	}
	void rebuild() {
		size_t max_storage_copy = max_storage;
		while (storage_counter >= max_storage * load_factor) {
			size_t max_storage_new = max_storage * expansion;
			if (max_storage_new == max_storage) max_storage_new++;
			max_storage = max_storage_new;
		}
		ENTRY *entries_new = (ENTRY*)HeapAlloc(hheap, 0, max_storage * sizeof(ENTRY)), *entries_copy = entries;
		entries = entries_new;
		cleanup();
		for (size_t i = 0; i < max_storage_copy; i++)
			if (entries_copy[i].state == ENTRY::OCCUPIED) set(entries_copy[i].key, entries_copy[i].val);
		HeapFree(hheap, 0, entries_copy);
	}
	void enum_entries(DICT_ENUM_FUNC<TYPE_KEY, TYPE_VAL> enum_func, void* param) {
		ENTRY* current_entry = last_entry;
		while (current_entry != nullptr) {
			if (current_entry->state == ENTRY::OCCUPIED)
				if (enum_func(current_entry, param) == ENUM_FINISH) return;
			current_entry = current_entry->previous;
		}
	}
	#undef ENTRY
};


typedef struct float2D {
	float x, y;
	constexpr float2D(float _x, float _y): x(_x), y(_y) { }
	constexpr float2D(POINT p): x(float(p.x)), y(float(p.y)) { }
	float2D() = default;
	float2D operator+(const float2D& other) const { return float2D(x + other.x, y + other.y); }
	float2D operator-(const float2D& other) const { return float2D(x - other.x, y - other.y); }
	float2D operator*(float multiplier) const { return float2D(x * multiplier, y * multiplier); }
	float2D operator/(float divisor) const { return float2D(x / divisor, y / divisor); }
} float_pair;

GLuint create_empty_texture(SIZE size, GLenum VRAMformat, GLenum RAMformat = GL_BGRA_EXT) {
	GLuint textureID;
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, VRAMformat, size.cx, size.cy, 0, RAMformat, GL_UNSIGNED_BYTE, nullptr);
	return textureID;
}

struct FONT_CONTEXT {
	HDC     htextDC;
	HBITMAP htextBMP;
	BYTE*   BMPdata;
	SIZE    BMPsize, last_text_size;
	static BITMAPINFO create_default_BMI(SIZE BMPsize) {
		BITMAPINFO BMI = { 0 };
    	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    	BMI.bmiHeader.biWidth = BMPsize.cx;   BMI.bmiHeader.biHeight = -BMPsize.cy;
   		BMI.bmiHeader.biPlanes = 1;           BMI.bmiHeader.biBitCount = 32;
    	BMI.bmiHeader.biCompression = BI_RGB; BMI.bmiHeader.biSizeImage = BMPsize.cx * BMPsize.cy * 4;
    	return BMI;
	}
	void create_BMP() {
		BITMAPINFO BMI = create_default_BMI(BMPsize);
		htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&BMPdata, NULL, 0);
		SelectObject(htextDC, BMPdata);
	}
	FONT_CONTEXT(): htextDC(CreateCompatibleDC(NULL)), BMPsize(SIZE{ 64, 64 }) {
		create_BMP();
		SetTextColor(htextDC, RGB(255, 255, 255));
		SetBkColor(htextDC, (COLORREF)0);
		SetBkMode(htextDC, OPAQUE);
	}
	~FONT_CONTEXT() {
		DeleteObject(htextBMP);
		DeleteDC(htextDC);
	}
	void setfont(HFONT hfont) { SelectObject(htextDC, hfont); }
	void render(const wchar_t* text) {
		int32_t textlen = (int32_t)wcslen(text);
		GetTextExtentPoint32W(htextDC, text, textlen, &last_text_size);
		if (last_text_size.cx > BMPsize.cx || last_text_size.cy > BMPsize.cy) {
			BMPsize = last_text_size;
			DeleteObject(htextBMP);
			create_BMP();
		}
		RECT text_zone = RECT{ 0, 0, last_text_size.cx, last_text_size.cy };
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &text_zone, text, textlen, NULL);
	}
};

struct WRITE_INFO {
	uint8_t state;
	POINT   divide;
	static constexpr uint8_t X_POSITIVE = 0x0, X_NEGATIVE = 0x1, Y_POSITIVE = 0x0, Y_NEGATIVE = 0x2, X_DRAW_HALF = 0x4, Y_DRAW_HALF = 0x8;
	constexpr WRITE_INFO(uint8_t _state, POINT _divide): state(_state), divide(_divide) { }
	constexpr WRITE_INFO(): state(0) { }
};
struct CHARPAGE {
	GLuint textureID;
	CHARPAGE(SIZE size) { textureID = create_empty_texture(SIZE{ 64, 64 }, GL_LUMINANCE, GL_LUMINANCE); }
	void write(const wchar_t* text, POINT pos, FONT_CONTEXT* context, WRITE_INFO direction, HANDLE hheap = NULL, HFONT hfont = NULL) {
		if (direction.state & (WRITE_INFO::X_DRAW_HALF | WRITE_INFO::Y_DRAW_HALF) == 0) {
			if (hfont != NULL) context->setfont(hfont);
			context->render(text);
			POINT edge;
			if (context->last_text_size + pos.x > )
		} else {
			
		}
		POINT source_from, source_to;
		if (direction.state & WRITE_INFO::X_DRAW_HALF) { // X-axis divided
			if (direction.state & WRITE_INFO::X_NEGATIVE) // @ left part?
			     { source_from.x = 0;                  source_to.x = direction.divide.x; }
			else { source_from.x = direction.divide.x; source_to.x = context->last_text_size.cx; }
		} else   { source_from.x = 0;                  source_to.x = context->last_text_size.cx; }
		if (direction.state & WRITE_INFO::Y_DRAW_HALF) { // Likewise...
			if (direction.state & WRITE_INFO::Y_NEGATIVE)
			     { source_from.y = 0;                  source_to.y = direction.divide.y; }
			else { source_from.y = direction.divide.y; source_to.y = context->last_text_size.cy; }
		} else   { source_from.y = 0;                  source_to.y = context->last_text_size.cy; }
		size_t R8size = size_t(source_to.y - source_from.y) * (source_to.x - source_from.x);
		BYTE* R8data;
		if (R8size > 1024) R8data = (BYTE*)HeapAlloc((hheap == NULL) ? GetProcessHeap() : hheap, 0, R8size);
		else R8data = (BYTE)__builtin_alloca(R8size);
		size_t R8data_counter = 0;
		for (; source_from.y < source_to.y; source_from.y++) {
			size_t line_offset = size_t(source_to.x - source_from.x) * source_from.y;
			for (int32_t current_x = source_from.x, current_x < source_to.x; current_x++)
				R8data[R8data_counter++] = context->BMPdata[(line_offset + current_x) * 4];
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, ); /////////////////
		if (R8size > 1024) HeapFree((hheap == NULL) ? GetProcessHeap() : hheap, 0, R8data);
	}
};

struct VRAM_CHARZONE {
	SIZE     texture_size;
	GLuint   textureID;
	LOGFONTW LF;
	HFONT    hfont;
	BYTE*    R8buffer;
	VRAM_CHARZONE(LOGFONTW* pLF): texture_size(SIZE{ 64, 64 }), R8buffer(nullptr) {
		
	}
	~VRAM_CHARZONE() { DeleteObject(hfont); }
	
};

// glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

