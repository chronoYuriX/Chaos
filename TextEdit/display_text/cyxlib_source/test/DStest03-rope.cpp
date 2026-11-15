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

template <typename TYPE_HANDLE>
struct HANDLE_ALLOCATOR_ALPHA {
	TYPE_HANDLE        counter;
	STACK<TYPE_HANDLE> free_handles;
	size_t             stack_initial_size;
	float              shrink_threshold;
	HANDLE_ALLOCATOR_ALPHA(SCC* p_constructor, float _shrink_threshold = 0.f):
			counter(0), free_handles(p_constructor), stack_initial_size(p_constructor->initial_size) {
		shrink_threshold = (0.f < _shrink_threshold && _shrink_threshold < 1.f) ? _shrink_threshold : DICT<TYPE_HANDLE, TYPE_HANDLE>::DEFAULT_LOAD_FACTOR;
	}
	TYPE_HANDLE alloc() {
		if (free_handles.getsize() == 0) return counter++;
		else return free_handles.pop();
	}
	void free(TYPE_HANDLE handle) {
		if (handle == counter - 1) counter--;
		else free_handles.push(handle);
	}
	void GC() {
		TYPE_HANDLE i = free_handles.getsize();
		while (--i > 0) {
			for (TYPE_HANDLE j = 0; j < i; j++) {
				if (free_handles.elements.elements[j] > free_handles.elements.elements[j + 1]) {
					TYPE_HANDLE swap_temp = free_handles.elements.elements[j];
					free_handles.elements.elements[j] = free_handles.elements.elements[j + 1];
					free_handles.elements.elements[j + 1] = swap_temp;
				}
			}
			if (free_handles.elements.elements[i] != counter - free_handles.getsize() + i) break;
		}
		TYPE_HANDLE free_counter = free_handles.getsize() - i - 1;
		counter -= free_counter;
		free_handles.elements.storage_counter -= free_counter;
		if (free_handles.getsize() < free_handles.elements.max_storage * shrink_threshold) free_handles.shrink();
	}
};
/*
struct HANDLE_ALLOCATOR_BETA {
	TYPE_HANDLE       counter;
	DICT<TYPE_HANDLE> free_handles;
};
*/

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
	
	wprintf(L"\nHANDLE_ALLOCATOR_ALPHA:\n");
	HANDLE_ALLOCATOR_ALPHA<int> ha(nullptr);
	srand(GetTickCount());
	wprintf(L"Alloc: ");
	for (int i = 0; i < 200; i++) wprintf(L"%d ", ha.alloc());
	wprintf(L"\nFree: ");
	for (int i = 0; i < 200; i++) if (rand() % 3 == 0) {
		ha.free(i);
		wprintf(L"%d ", i);
	}
	wprintf(L"\nAlloc: ");
	for (int i = 0; i < 20; i++) wprintf(L"%d ", ha.alloc());
	wprintf(L"\nFree: ");
	for (int i = 0; i < 100; i++) {
		if (rand() % 2) {
			bool found = false;
			for (int j = 0; j < ha.free_handles.getsize(); j++) {
				if (ha.free_handles.elements.elements[j] == i) {
					found = true;
					break;
				}
			}
			if (!found) {
				ha.free(i);
				wprintf(L"%d ", i);
			}
		}
	}
	for (int i = 100; i < ha.counter; i++) {
		bool found = false;
		for (int j = 0; j < ha.free_handles.getsize(); j++) {
			if (ha.free_handles.elements.elements[j] == i) {
				found = true;
				break;
				}
			}
		if (!found) {
			ha.free(i);
			wprintf(L"%d ", i);
		}
	}
	wprintf(L"\nNow free handles count: %d, counter: %d\n", ha.free_handles.getsize(), ha.counter);
	for (int i = 0; i < ha.free_handles.getsize(); i++) wprintf(L"%d ", ha.free_handles.elements.elements[i]);
	ha.GC();
	wprintf(L"\nNow free handles count: %d, counter: %d\n", ha.free_handles.getsize(), ha.counter);
	for (int i = 0; i < ha.free_handles.getsize(); i++) wprintf(L"%d ", ha.free_handles.elements.elements[i]);
	
	wprintf(L"\nOVER\n");
	return 0;
}

