template <typename TYPE_1, typename TYPE_2>
struct PAIR {
	TYPE_1 data_1; TYPE_2 data_2;
	constexpr PAIR(TYPE_1 _data_1, TYPE_2 _data_2): data_1(_data_1), data_2(_data_2) { }
	PAIR() = default;
};

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


struct STRING_POOL_REG {
	size_t max_main_storage,  max_indexer;
	float  indexer_expansion, indexer_load_factor;
	HANDLE storage_hheap,     indexer_hheap;
	void set_default() {
		max_main_storage       = max_indexer         = 0;
		main_storage_expansion = indexer_expansion   = indexer_load_factor = 0.f;
		main_storage_hheap     = indexer_hheap       = GetProcessHeap();
	}
};
template <typename TYPE_STR>
struct STRING_POOL {
	DICT<size_t, TYPE_STR*> indexer;
	TYPE_STR*               main_storage;
	float                   main_storage_expansion;
	size_t                  max_main_storage, main_storage_counter;
	HANDLE                  main_storage_hheap;
	STRING_POOL(STRING_POOL_REG* _preg):
			indexer(_preg->max_indexer, _preg->indexer_expansion, _preg->indexer_load_factor, _preg->indexer_hheap),
			max_main_storage      ((_preg->max_main_storage == 0)         ? LIST<TYPE_STR>::DEFAULT_STORAGE   : _preg->max_main_storage),
			main_storage_expansion((_preg->main_storage_expansion <= 1.f) ? LIST<TYPE_STR>::DEFAULT_EXPANSION : _preg->main_storage_expansion),
			main_storage_hheap    ((_preg->main_storage_hheap == NULL)    ? GetProcessHeap()                  : _preg->main_storage_hheap),
			main_storage_counter(0) {
		main_storage = (TYPE_STR*)HeapAlloc(main_storage_hheap, 0, max_main_storage * sizeof(TYPE_STR));
	}
	~STRING_POOL() {
		if (main_storage != nullptr) HeapFree(main_storage_hheap, 0, main_storage);
		main_storage = nullptr;
	}
	static bool storage_GC_enum(DICT_ENTRY<size_t, TYPE_STR*>* string_entry, void* root) {
		TYPE_STR* external_main_storage = ((PAIR<STRING_POOL*, PAIR<size_t*, size_t*>>*)root)->data_1->main_storage;
		PAIR<size_t*, size_t*> external_copy_params = ((PAIR<STRING_POOL*, PAIR<size_t*, size_t*>>*)root)->data_2;
		size_t *external_copy_dest = external_copy_params.data_1, *external_copy_source = external_copy_params.data_2;
		for (size_t i = 0; string_entry->val[i] != 0 /* '\0' or L'\0' */ ; i++) {
			external_main_storage[(*external_copy_dest)++] = string_entry->val[i];
		}
	}
	void resize(size_t max_main_storage_new) {
		size_t copy_dest = 0, copy_source = 0;
		PAIR<size_t*, size_t*> storage_GC_enum_copy_params(&copy_dest, &copy_dest);
		PAIR<STRING_POOL*, size_t*> storage_GC_enum_params(this, storage_GC_enum_copy_params);
		enum_entries(storage_GC_enum, &storage_GC_enum_params);
	}
	size_t alloc_string(const TYPE_STR* source, size_t length = 0) {
		if (length == 0) {
			while (source[length] != 0 /* '\0' or L'\0' */ ) length++;
			if (length == 0) return;
		}
		if (main_storage_counter + length >= max_main_storage) {
			
		}
	}
};



