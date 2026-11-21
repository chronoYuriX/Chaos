#ifdef _DEBUG
	#define DS_POISON_PTR(p) (p = reinterpret_cast<decltype(p)>(0xCDCDCDCD))
#else
	#define DS_POISON_PTR(p)
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>


struct DS_CONTEXT {
	typedef uint16_t ERROR_CODE;
	static constexpr ERROR_CODE
		CONTEXT_NO_ERROR          = 0x0000, CONTEXT_REACHED_MAX_DEPTH = 0x0001, CONTEXT_OVERPOPPED = 0x0002, CONTEXT_UNDEFINED_ERROR = 0x0FFF,
		MEM_HEAP_ALLOCATION_ERROR = 0x1001;
	struct CHAIN_LINK {
		void*          structure;
		const wchar_t* location_description;
	};
	CHAIN_LINK* call_chain;
	ERROR_CODE  last_error_code;
	wchar_t*    last_error_description;
	size_t      max_depth, current_depth, max_error_description;
	HANDLE      h_heap;
	DS_CONTEXT(size_t _max_depth, size_t _max_error_description, HANDLE _h_heap = nullptr):
			max_depth(_max_depth), current_depth(0), max_error_description(_max_error_description) {
		h_heap = (_h_heap == nullptr) ? GetProcessHeap() : _h_heap;
		call_chain             = (CHAIN_LINK*)HeapAlloc(h_heap, 0, sizeof(CHAIN_LINK) * max_depth);
		last_error_description =    (wchar_t*)HeapAlloc(h_heap, 0, sizeof(wchar_t)    * max_error_description);
		if (call_chain == nullptr || last_error_description == nullptr) {
			try_HeapFree(h_heap, 0, call_chain);
			try_HeapFree(h_heap, 0, last_error_description);
			report_error(L"Failed to allocate heap memory!", MEM_HEAP_ALLOCATION_ERROR);
		}
	}
	~DS_CONTEXT() {
		try_HeapFree(h_heap, 0, call_chain);
		try_HeapFree(h_heap, 0, last_error_description);
	}
	static inline WINAPI WINBOOL try_HeapFree(HANDLE h_heap, DWORD flags, void* p_mem) {
		if (p_mem != nullptr) return HeapFree(h_heap, flags, p_mem);
		return (WINBOOL)0;
	}
	void push(void* structure, const wchar_t* location_description) {
		if (current_depth < max_depth) {
			CHAIN_LINK current_link;
			current_link.structure            = structure;
			current_link.location_description = location_description;
			call_chain[current_depth++] = current_link;
		} else report_error(L"Context reached max depth!", CONTEXT_REACHED_MAX_DEPTH);
	}
	void pop(size_t depth = 1) {
		if (current_depth >= depth) current_depth -= depth;
		else report_error(L"Context overpopped!", CONTEXT_OVERPOPPED);
	}
	void report_error(const wchar_t* error_description, ERROR_CODE error_code) {
		const wchar_t STRING_REPORT_OVERFLOW[] = L"\\:Error report overflow!";
		last_error_code = error_code;
		size_t length = 0;
		for (size_t i = 0; i < current_depth; i++) {
			size_t new_length = length + wcslen(call_chain[i].location_description) + 1;
			if (new_length >= max_error_description) {
				wcscpy(last_error_description, STRING_REPORT_OVERFLOW);
				break;
			}
			last_error_description[length] = L'\\';
			length += 1;
			wcscpy(last_error_description + length, call_chain[i].location_description);
			length = new_length;
		}
		size_t new_length = length + wcslen(error_description) + 2;
		if (new_length >= max_error_description) wcscpy(last_error_description, STRING_REPORT_OVERFLOW);
		else {
			wcscpy(last_error_description + length, L": ");
			wcscpy(last_error_description + length + 2, error_description);
			last_error_description[new_length] = L'\0';
		}
	}
};

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
	STANDARD_CONTAINER_CONSTRUCTOR(): h_heap(nullptr) { }
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
		DS_CONTEXT::try_HeapFree(h_heap, 0, elements);
		elements = nullptr;
	}
	void append_ptr(TYPE_ELEMENT* p_element) {
		if (storage_counter == max_storage) expand();
		memcpy(elements + storage_counter, p_element, sizeof(TYPE_ELEMENT));
		storage_counter++;
	}
	inline void append(TYPE_ELEMENT element) { append_ptr(&element); }
	void resize(size_t max_storage_new, DS_CONTEXT* context = nullptr) {
		if (max_storage_new != max_storage) {
			max_storage = max_storage_new;
			elements = (TYPE_ELEMENT*)HeapReAlloc(h_heap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
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
		ENTRY*   previous;
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

struct BUCKET_ALLOCATOR {
	size_t bucket_size;
	HANDLE h_heap;
	BUCKET_ALLOCATOR(size_t _bucket_size, HANDLE _h_heap = nullptr): bucket_size(_bucket_size),
			h_heap((_h_heap == nullptr) ? GetProcessHeap() : _h_heap) { }
	inline void* alloc() { return HeapAlloc(h_heap, 0, bucket_size); }
	inline void free(void* p_bucket) { HeapFree(h_heap, 0, p_bucket); }
};

template <typename TYPE_HANDLE, size_t BUCKET_STORAGE_SIZE>
struct HANDLE_ALLOCER {
	template <size_t STORAGE_SIZE>
	struct BUCKET {
		HANDLE_ALLOCER* root;
		TYPE_HANDLE     storage[STORAGE_SIZE];
		BUCKET*         previous;
		BUCKET(HANDLE_ALLOCER* _root): root(_root) { }
		void* operator new(size_t __reserved_size, HANDLE_ALLOCER* root) { return HeapAlloc(root->h_heap, 0, __reserved_size); }
		void operator delete(void* __reserved_ptr, HANDLE_ALLOCER* root) { HeapFree(root->h_heap, 0, __reserved_ptr); }
	};
	BUCKET<BUCKET_STORAGE_SIZE>* last_bucket;
	BUCKET_ALLOCATOR bucket_allocator;
	size_t           handles_per_bucket;
	HANDLE           h_heap;
	HANDLE_ALLOCER(size_t _handles_per_bucket = 1024, HANDLE _h_heap = nullptr) {
		last_bucket = new(this) BUCKET(this);
	}
};

int main() {
	DS_CONTEXT context(1024, 1024);
	context.push(nullptr, L"main");
	context.push(nullptr, L"class01");
	context.push(nullptr, L"func02");
	context.push(nullptr, L"struct03");
	context.report_error(L"Test error~", DS_CONTEXT::CONTEXT_NO_ERROR);
	wprintf(L"%ls #%u\n", context.last_error_description, context.last_error_code);
	context.pop();
	context.report_error(L"Error test~", DS_CONTEXT::CONTEXT_NO_ERROR);
	wprintf(L"%ls #%u\n", context.last_error_description, context.last_error_code);
	return 0;
}























