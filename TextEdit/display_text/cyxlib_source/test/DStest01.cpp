#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdint.h>
#include <stdio.h>


uint64_t FNVhash(const uint8_t* key_bytes, size_t key_length) {
	uint64_t result = 14695981039346656037ULL;
	for (size_t i = 0; i < key_length; i++) result = (result ^ key_bytes[i]) * 1099511628211ULL;
	return result;
}
template <typename TYPE_KEY>
inline uint64_t FNVhash_tp(const TYPE_KEY* p_key) { return FNVhash((uint8_t*)p_key, sizeof(TYPE_KEY)); }
template <typename TYPE_KEY>
inline uint64_t FNVhash_t(const TYPE_KEY key) { return FNVhash_tp<TYPE_KEY>(&key); }

typedef struct STANDARD_CONTAINER_CONSTRUCTOR {
	size_t initial_size;
	float  expansion;
	HANDLE h_heap;
	STANDARD_CONTAINER_CONSTRUCTOR(): h_heap(NULL) { }
} SCC;

template <typename TYPE_ELEMENT>
struct LIST {
	TYPE_ELEMENT* elements;
	size_t        storage_counter, max_storage;
	float         expansion;
	HANDLE        h_heap;
	static constexpr float  DEFAULT_EXPANSION = 1.5f;
	static constexpr size_t DEFAULT_STORAGE = 64;
	LIST(SCC* p_constructor) {
		if (p_constructor == nullptr) {
			max_storage = DEFAULT_STORAGE;
			expansion   = DEFAULT_EXPANSION;
			h_heap      = GetProcessHeap();
		} else {
			max_storage = (p_constructor->initial_size == 0) ?   DEFAULT_STORAGE : p_constructor->initial_size;
			expansion   = (p_constructor->expansion  <= 1.f) ? DEFAULT_EXPANSION : p_constructor->expansion;
			h_heap      = (p_constructor->h_heap == nullptr) ?  GetProcessHeap() : p_constructor->h_heap;
		}
		elements = (TYPE_ELEMENT*)HeapAlloc(h_heap, 0, max_storage * sizeof(TYPE_ELEMENT));
		cleanup();
	}
	~LIST() {
		if (elements != nullptr) HeapFree(h_heap, 0, elements);
		elements = nullptr;
	}
	void append_ptr(TYPE_ELEMENT* p_element) {
		if (storage_counter == max_storage) expand();
		memcpy(elements + storage_counter, p_element, sizeof(TYPE_ELEMENT));
		storage_counter++;
	}
	inline void append(TYPE_ELEMENT element) { append_ptr(&element); }
	void resize(size_t max_storage_new) {
		if (max_storage_new != max_storage) {
			max_storage = max_storage_new;
			elements = (TYPE_ELEMENT*)HeapReAlloc(h_heap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
		}
	}
	void expand() {
		size_t max_storage_new = max_storage * expansion;
		if (max_storage_new == max_storage) max_storage_new++;
		printf("Expand: %u -> %u\n", max_storage, max_storage_new);
		resize(max_storage_new);
	}
	void expand_to(size_t min_storage) {
		while (max_storage < min_storage) {
			size_t max_storage_temp = max_storage * expansion;
			if (max_storage_temp == max_storage) max_storage_temp++;
			max_storage = max_storage_temp;
		}
		elements = (TYPE_ELEMENT*)HeapReAlloc(h_heap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
	}
	inline void cleanup() { storage_counter = 0; }
	inline void shrink() { resize(storage_counter); }
};

template <typename TYPE_ELEMENT>
struct STACK {
	LIST<TYPE_ELEMENT> elements;
	STACK(SCC* p_constructor): elements(p_constructor) { }
	inline void push_ptr(TYPE_ELEMENT* p_element) { elements.append_ptr(p_element); }
	inline void push(TYPE_ELEMENT element) { elements.append(element); }
	TYPE_ELEMENT pop() {
		if (elements.storage_counter > 0) {
			elements.storage_counter--;
			return elements.elements[elements.storage_counter];
		}
		return TYPE_ELEMENT{ 0 };
	}
	inline void cleanup() { elements.cleanup(); }
	inline size_t getsize() { return elements.storage_counter; }
	inline void shrink() { elements.shrink(); }
};

template <typename TYPE_KEY, typename TYPE_VAL>
struct DICT {
	struct ENTRY {
		TYPE_KEY key;
		TYPE_VAL val;
		uint8_t  state;
		ENTRY* previous;
		static constexpr uint8_t FREE = 1, OCCUPIED = 2, DELETED = 3;
	};
	using ENUM_FUNC = bool (*)(ENTRY*, void*);
	size_t  storage_counter,  max_storage;
	ENTRY  *entries,         *last_entry;
	float   expansion,        load_factor;
	HANDLE  h_heap;
	static constexpr float DEFAULT_LOAD_FACTOR = .5f;
	static constexpr bool  ENUM_CONTINUE = false, ENUM_FINISH = true;
	DICT(SCC* p_constructor, float _load_factor = 0.f):
			load_factor((_load_factor > 1.f || _load_factor <= 0.f) ? DEFAULT_LOAD_FACTOR : _load_factor) {
		if (p_constructor == nullptr) {
			max_storage = LIST<ENTRY>::DEFAULT_STORAGE;
			expansion   = LIST<ENTRY>::DEFAULT_EXPANSION;
			h_heap      = GetProcessHeap();
		} else {
			max_storage = (p_constructor->initial_size == 0) ?   LIST<ENTRY>::DEFAULT_STORAGE : p_constructor->initial_size;
			expansion   = (p_constructor->expansion <= 1.f)  ? LIST<ENTRY>::DEFAULT_EXPANSION : p_constructor->expansion;
			h_heap      = (p_constructor->h_heap == nullptr) ?               GetProcessHeap() : p_constructor->h_heap;
		}
		entries = (ENTRY*)HeapAlloc(h_heap, 0, max_storage * sizeof(ENTRY));
		cleanup();
	}
	~DICT() {
		if (entries != nullptr) HeapFree(h_heap, 0, entries);
		entries = nullptr;
	}
	void set(const TYPE_KEY key, const TYPE_VAL val) {
		TYPE_VAL* target = get(key);
		if (target == nullptr) {
			if (storage_counter >= max_storage * load_factor) expand();
			uint64_t dest = FNVhash_t(key) % max_storage;
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
		uint64_t dest = FNVhash_t(key) % max_storage; uint64_t start = dest;
		while (entries[dest].state != ENTRY::FREE) {
			if (entries[dest].key == key) {
				if (entries[dest].state == ENTRY::OCCUPIED) return entries + dest;
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
	void rebuild(size_t original_size) {
		ENTRY *entries_new = (ENTRY*)HeapAlloc(h_heap, 0, max_storage * sizeof(ENTRY)), *entries_copy = entries;
		entries = entries_new;
		cleanup();
		for (size_t i = 0; i < original_size; i++)
			if (entries_copy[i].state == ENTRY::OCCUPIED) set(entries_copy[i].key, entries_copy[i].val);
		HeapFree(h_heap, 0, entries_copy);
	}
	void expand() {
		size_t original_size = max_storage;
		while (storage_counter >= max_storage * load_factor) {
			size_t max_storage_new = max_storage * expansion;
			if (max_storage_new == max_storage) max_storage_new++;
			max_storage = max_storage_new;
		}
		rebuild(original_size);
	}
	void shrink() {
		max_storage = storage_counter + 1;
		rebuild();
	}
	void enum_entries(ENUM_FUNC enum_func, void* param) {
		ENTRY* current_entry = last_entry;
		while (current_entry != nullptr) {
			if (current_entry->state == ENTRY::OCCUPIED)
				if (enum_func(current_entry, param) == ENUM_FINISH) return;
			current_entry = current_entry->previous;
		}
	}
};

struct STRING_POOL {
	typedef uint64_t string_handle;
	struct STRING_HEADER {
		size_t   length;
		uint64_t hash_val;
		uint8_t  state;
		static constexpr uint8_t OCCUPIED = 1, DELETED = 2;
	};
	LIST<uint8_t>               main_storage;
	DICT<string_handle, size_t> strings;
	uint64_t counter = 0;
	static constexpr uint8_t FIT_FIRST = 1, FIT_BEST = 2;
	STRING_POOL(SCC* p_constructor, float _load_factor = 0.f): main_storage(p_constructor), strings(p_constructor, _load_factor) { }
	string_handle alloc(const wchar_t* string, uint8_t fit_mode = FIT_FIRST, size_t string_length = 0) {
		if (string_length == 0) {
			string_length = wcslen(string);
			if (string_length == 0) return string_handle(~0);
		}
		struct ENUM_GLOBLES {
			uint8_t fit_mode,         *main_storage_elements;
			size_t  dest_offset = ~0,  best_size = ~0,        string_length;
		} globles;
		globles.fit_mode              = fit_mode;
		globles.main_storage_elements = main_storage.elements;
		globles.string_length         = string_length;
		strings.enum_entries([](DICT<string_handle, size_t>::ENTRY* entry, void* p_void) -> bool {
				ENUM_GLOBLES*  p_globles = (ENUM_GLOBLES*)p_void;
				STRING_HEADER* header    = (STRING_HEADER*)(p_globles->main_storage_elements + entry->val - sizeof(STRING_HEADER));
        		if (header->state == STRING_HEADER::DELETED) {
        			if (header->length >= p_globles->string_length) {
        				p_globles->dest_offset = entry->val;
        				if (p_globles->fit_mode == FIT_FIRST) return DICT<string_handle, size_t>::ENUM_FINISH;
						else if (header->length < p_globles->best_size) p_globles->best_size = header->length;
					}
				} else wprintf(L"ENUM:ALIVE:%u\n", header->length);
        		return DICT<string_handle, size_t>::ENUM_CONTINUE;
    		}, &globles);
		STRING_HEADER current_header;
		current_header.length   = string_length;
		current_header.hash_val = FNVhash((uint8_t*)string, string_length * sizeof(wchar_t));
		current_header.state    = STRING_HEADER::OCCUPIED;
		if (globles.dest_offset == ~0) {
			wprintf(L"alloc_at_tail: %u\n", main_storage.storage_counter);
			globles.dest_offset = main_storage.storage_counter;
			main_storage.expand_to(globles.dest_offset + sizeof(STRING_HEADER) + (string_length + 1) * sizeof(wchar_t));
		}
		uint8_t* copy_destination = main_storage.elements + globles.dest_offset;
		memcpy(copy_destination, &current_header, sizeof(STRING_HEADER));
		wcscpy((wchar_t*)(copy_destination + sizeof(STRING_HEADER)), string);
		main_storage.storage_counter += sizeof(STRING_HEADER) + (string_length + 1) * sizeof(wchar_t);
		strings.set(counter, globles.dest_offset + sizeof(STRING_HEADER));
		wprintf(L"Done: %ls, %u\n", (wchar_t*)(copy_destination + sizeof(STRING_HEADER)), counter);
		return counter++;
	}
	void get(string_handle h_string, wchar_t* dest) {
		size_t* offset = strings.get(h_string);
		if (offset == nullptr) dest[0] = L'\0';
		else wcscpy(dest, (wchar_t*)(main_storage.elements + *offset));
	}
	void remove(string_handle h_string) {
		size_t* offset = strings.get(h_string);
		if (offset != nullptr) {
			STRING_HEADER* header = (STRING_HEADER*)(main_storage.elements + *offset - sizeof(STRING_HEADER));
			header->state = STRING_HEADER::DELETED;
			strings.remove(h_string);
		}
	}
	void rebuild() {
		struct ENUM_GLOBLES {
			uint8_t* main_storage_elements;
			size_t   new_counter = 0;
		} globles;
		globles.main_storage_elements = main_storage.elements;
		strings.enum_entries([](DICT<string_handle, size_t>::ENTRY* entry, void* p_void) -> bool {
				ENUM_GLOBLES*  p_globles = (ENUM_GLOBLES*)p_void;
				STRING_HEADER* header = (STRING_HEADER*)(p_globles->main_storage_elements + entry->val - sizeof(STRING_HEADER));
        		if (header->state == STRING_HEADER::OCCUPIED) {
        			size_t move_size = sizeof(STRING_HEADER) + (header->length + 1) * sizeof(wchar_t);
        			wprintf(L"Move: %u -> %u\n", entry->val - sizeof(STRING_HEADER), p_globles->new_counter);
					memmove(p_globles->main_storage_elements + p_globles->new_counter, header, move_size);
					entry->val = p_globles->new_counter + sizeof(STRING_HEADER);
					p_globles->new_counter += move_size;
				}
        		return DICT<string_handle, size_t>::ENUM_CONTINUE;
    		}, &globles);
		main_storage.storage_counter = globles.new_counter;
	}
};


int main() {
	wprintf(L"LIST:\n");
	LIST<wchar_t> ls(nullptr);
	for (wchar_t c = L'A'; c <= L'Z'; c++) ls.append(c);
	for (wchar_t c = L'a'; c <= L'z'; c++) ls.append(c);
	for (size_t i = 0; i < ls.storage_counter; i++) wprintf(L"%lc", ls.elements[i]);
	wprintf(L"\nSTACK:\n");
	STACK<wchar_t> st(nullptr);
	for (wchar_t c = L'A'; c <= L'Z'; c++) st.push(c);
	for (wchar_t c = L'a'; c <= L'z'; c++) st.push(c);
	while (st.getsize() > 0) wprintf(L"%lc", st.pop());
	
	wprintf(L"\nSTRPOOL:\n");
	STRING_POOL po(nullptr);
	STRING_POOL::string_handle hstr_1 = po.alloc(L"1234567");
	STRING_POOL::string_handle hstr_X = po.alloc(L"To be deleted...");
	STRING_POOL::string_handle hstr_2 = po.alloc(L"abcde");
	STRING_POOL::string_handle hstr_3 = po.alloc(L"qwertyuiop");
	wchar_t buffer[64];
	po.get(hstr_X, buffer);
	wprintf(L"hstr_X: %ls\n", buffer);
	po.remove(hstr_X);
	po.get(hstr_X, buffer);
	wprintf(L"hstr_X: %ls\n", buffer);
	wprintf(L"Size: %u\n", po.main_storage.storage_counter);
	po.rebuild();
	wprintf(L"Size: %u\n", po.main_storage.storage_counter);
	po.get(hstr_3, buffer);
	wprintf(L"hstr_3: %ls\n", buffer);
	wprintf(L"\nOVER\n");
	return 0;
}

