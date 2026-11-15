#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "basedefs.cpp"



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
	void append_ptr(TYPE_ELEMENT* pelement) {
		if (storage_counter == max_storage) expand();
		memcpy(elements + storage_counter, pelement, sizeof(TYPE_ELEMENT));
		storage_counter++;
	}
	inline void append(TYPE_ELEMENT element) { append_ptr(&element); }
	void resize(size_t max_storage_new) {
		if (max_storage_new != max_storage) {
			max_storage = max_storage_new;
			elements = (TYPE_ELEMENT*)HeapReAlloc(hheap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
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
		elements = (TYPE_ELEMENT*)HeapReAlloc(hheap, 0, elements, max_storage * sizeof(TYPE_ELEMENT));
	}
	inline void cleanup() { storage_counter = 0; }
	inline void shrink() { resize(storage_counter); }
};

template <typename TYPE_ELEMENT>
struct STACK {
	LIST<TYPE_ELEMENT> elements;
	STACK(size_t _max_storage = 0, float _expansion = 0.f, HANDLE _hheap = NULL): elements(_max_storage, _expansion, _hheap) { }
	inline void push_ptr(TYPE_ELEMENT* pelement) { elements.append_ptr(pelement); }
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
	HANDLE  hheap;
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
	struct HASHED_STRING {
		wchar_t* string;
		uint64_t hash_val;
		uint8_t  state;
		static constexpr uint8_t OCCUPIED = 1, DELETED = 2;
	};
	struct GAP {
		size_t offset, size;
		GAP* next;
		static constexpr uint8_t FIT_FIRST = 1, FIT_BEST = 2;
		static void remove_next(HANDLE hheap) {
			if (next != nullptr) next = next->next;
			HeapFree(hheap, 0, next);
		}
		GAP* fit(size_t goal, uint8_t mode) {
			GAP* current_gap = this, target = nullptr;
			if (mode == FIT_BEST) {
				bool found = false;
				size_t min_size;
				do {
					if (found) {
						if (current_gap->size < min_size) {
							min_size = current_gap->size;
							target = current_gap;
						}
					} else if (current_gap->size >= goal) {
						found = true;
						min_size = current_gap->size;
						target = current_gap;
					}
					current_gap = current_gap->next;
				} while (current_gap != nullptr);
			} else {
				do {
					if (current_gap->size >= goal) {
						target = current_gap;
						break;
					}
					current_gap = current_gap->next;
				} while (current_gap != nullptr);
			}
			return target;
		}
		void makegap(GAP* temp_gap) {
			GAP* current_gap = this;
			do {
				if (temp_gap->offset < current_gap->offset) {
					if (temp_gap->offset + temp_gap->size >= current_gap->offset) {
						current_gap
					}
				}
				current_gap = current_gap->next;
			} while (current_gap != nullptr);
		}
	};
	LIST<wchar_t>          main_storage;
	DICT<uint64_t, size_t> strings;
	GAP*                   first_gap;
	float                  expansion;
	HANDLE                 hheap;
	STRING_POOL():
};


int main() {
	LIST<wchar_t> ls(10);
	for (wchar_t c = L'A'; c <= L'Z'; c++) ls.append(c);
	for (wchar_t c = L'a'; c <= L'z'; c++) ls.append(c);
	for (size_t i = 0; i < ls.storage_counter; i++) wprintf(L"%lc", ls.elements[i]);
	wprintf(L"\nOVER\n");
	return 0;
}

