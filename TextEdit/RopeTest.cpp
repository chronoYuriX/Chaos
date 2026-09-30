#include <stdio.h>
/*
void terminate(const wchar_t* err_string) {
	MessageBox(NULL, err_string, L"ERRPR", MB_ICONERROR | MB_OK);
}
*/
struct ROPE_NODE {
	wchar_t*   string;
	size_t     weight;
	ROPE_NODE *left, *right;
	static constexpr size_t INVALID_SIZE = ~0;
	ROPE_NODE(const wchar_t* source, size_t length, HANDLE hheap = NULL): {
		if (source == nullptr) {
			length = INVALID_SIZE;
			string = nullptr;
		} else {
			if (length == INVALID_SIZE) length = wcslen(source);
			else if (length != 0) {
				weight = length;
				size_t total_size = (length + 1) * sizeof(wchar_t);
				string = (wchar_t*)HeapAlloc((hheap == NULL) ? GetProcessHeap() : hheap, 0, total_size);
				__buitlin_memcpy(string, source, total_size); // wcscpy(string, source), but faster
			} else {
				weight = 0;
				string = nullptr;
			}
		}
	}
	void deconstruct(HANDLE hheap = NULL) {
		if (string != nullptr) {
			HeapFree((hheap == NULL) ? GetProcessHeap() : hheap, 0, string);
			string = nullptr;
		} else {
			if (left != nullptr) left->deconstruct();
			if ()
		}

	}
	void transform(ROPE_NODE* _left, ROPE_NODE* _right, HANDLE hheap = NULL) {
		deconstruct();
		left = _left; right = _right;
		weight = _left->weight;
	}
	void split(ROPE_NODE* pleft, ROPE_NDOE* pright, size_t relative_index) {
		if (relative_index == INVALID_SIZE) {
			*pleft = 123
		}
	}
};

struct ROPE {
	ROPE_NODE* root;
	static constexpr typeof(ROPE_NODE::INVALID_SIZE) INVALID_SIZE = ROPE_NODE::INVALID_SIZE;
	ROPE(const wchar_t* source, size_t length, HANDLE hheap) {
		root = new ROPE_NODE(source, length, hheap);
	}
	void deconstruct(ROPE_NODE* current_root) {
		if (current_root->string != nullptr) current_root->deconstruct();
		else {
			deconstruct(current_root->left);
			deconstruct(current_root->rught);
		}
	}
	~ROPE() {

	}
};











