#include <windows.h>
#include <stdint.h>
#include <stdio.h>

template <typename TYPE>
inline TYPE min(TYPE A, TYPE B) { return (A < B) ? A : B; }
template <typename TYPE>
inline TYPE max(TYPE A, TYPE B) { return (A > B) ? A : B; }

namespace RECTOPS {
	RECT ENPTY_RECT = RECT{ 0 };
	inline int64_t surface(RECT rect) { return int64_t(rect.right - rect.left) * (rect.bottom - rect.top); }
	#define MXREDGE(SIDE, OP) MXR.SIDE = OP<LONG>(rect_A.SIDE, rect_B.SIDE)
	#define GETMXR(TYPE, OP_1, OP_2) RECT get##TYPE(RECT rect_A, RECT rect_B) { \
		RECT MXR; \
		MXREDGE(left, OP_1); MXREDGE(right, OP_2); MXREDGE(top, OP_1); MXREDGE(bottom, OP_2); \
		return MXR; \
	}
	GETMXR(MBR, min, max) // Minimum bounding rectangle
	GETMXR(MCR, max, min) // Maximum common rectangle
	#undef GETMXR
	#undef MXREDGE
	inline RECT getMCRex(RECT rect_A, RECT rect_B) {
		RECT MCR = getMCR(rect_A, rect_B);
		if (MCR.left >= MCR.right || MCR.top >= MCR.bottom) return RECT{ 0 };
		return MCR;
	}
	inline int64_t enlargement(RECT rect_A, RECT rect_B) {
		return surface(getMBR(rect_A, rect_B)) + surface(getMCRex(rect_A, rect_B)) - surface(rect_A) - surface(rect_B);
	}
	inline bool intersect(RECT rect_A, RECT rect_B) {
		RECT MCR = getMCR(rect_A, rect_B);
		return MCR.left < MCR.right && MCR.top < MCR.bottom;
	}
	inline bool pointin(RECT rect, POINT point) {
		return rect.left <= point.x && point.x < rect.right && rect.top <= point.y && point.y < rect.bottom;
	}
}


template <uint8_t CAPACITY>
struct R_TREE {
	static constexpr uint8_t LEAF_CAPACITY = uint8_t((uint16_t)CAPACITY * sizeof(NODE*) / sizeof(RECT));
	struct NODE {
		typedef uint8_t (*ENUM_FUNC)(NODE*, void*);
		struct CREATE_EMPTY_TAG { };
		RECT    MBR;
		uint8_t tag;
		union {
			NODE* sub_nodes[CAPACITY];
			RECT  decision_boxes[LEAF_CAPACITY];
		};
		static constexpr uint8_t LEAF = 1, BRANCH = 2, ENUM_CONTINUE = 1, ENUM_PASSBRANCH = 2, ENUM_FINISH = 3;
		static constexpr CREATE_EMPTY_TAG CREATE_EMPTY;
		inline void cleanup() { ZeroMemory(sub_nodes, CAPACITY * sizeof(NODE*)); }
		NODE(RECT decision_box): MBR(decision_box), tag(LEAF) { cleanup(); }
		NODE(CREATE_EMPTY_TAG): MBR(RECTOPS::ENPTY_RECT), tag(LEAF) { cleanup(); }
		NODE() = default;
		void* operator    new(size_t reserved_size, HANDLE h_heap) { return HeapAlloc(h_heap, 0, sizeof(NODE)); }
		void  operator delete(void*   reserved_ptr, HANDLE h_heap) { HeapFree(h_heap, 0, reserved_ptr); }
		void split_leaf() {
			constexpr size_t HALF_LEAF_CAPACITY = LEAF_CAPACITY / 2;
			NODE *new_leaf_A = new NODE(CREATE_EMPTY), *new_leaf_B = new NODE(CREATE_EMPTY);
			uint8_t i = 0;
			while (i < HALF_LEAF_CAPACITY) new_leaf_A->add(decision_boxes[i++]);
			while (i < LEAF_CAPACITY)      new_leaf_B->add(decision_boxes[i++]);
			tag = BRANCH;
			cleanup();
			sub_nodes[0] = new_leaf_A; sub_nodes[1] = new_leaf_B;
		}
		void add(RECT decision_box) {
			if (tag == LEAF) {
				for (uint8_t i = 0; i < LEAF_CAPACITY; i++) {
					if (memcmp(decision_boxes + i, &RECTOPS::ENPTY_RECT) == 0) {
						decision_boxes[i] = decision_box;
						MBR = RECTOPS::getMBR(MBR, decision_box);
						return;
					}
				}
				split_leaf();
				add(decision_box);
			} else { // tag == BRANCH
				uint8_t current_index = 0, best_index = 0;
				int64_t best_metric = INT64_MAX;
				RECT    new_MBR = MBR;
				while (current_index < CAPACITY && sub_nodes[current_index] != nullptr) {
					int64_t current_metric = RECTOPS::enlargement(sub_nodes[current_index]->MBR, decision_box);
					if (current_metric < best_metric) {
						best_metric = current_metric;
						best_index = current_index;
					}
					current_index++;
				}
				sub_nodes[current_index] = new NODE(decision_box);
				MBR = RECTOPS::getMBR(MBR, decision_box);
			}
		}
		void enum_tree(NODE* root, ENUM_FUNC enum_func, void* param) {
			NODE* current_node =
			for
		}
		bool intersect(RECT scope) {
			if (RECTOPS::intersect(MBR, scope)) {
				if (tag == LEAF) {
					for (uint8_t i = 0; i < LEAF_CAPACITY; i++)
						if (RECTOPS::intersect(decision_boxes[i], scope)) return true;
					return false;
				} else { // tag == BRANCH
					for (uint8_t)
				}
				
			}
		}
	} root;
	R_TREE(): root(RECT{ 0 }) { }
	void* operator    new(size_t reserved_size, HANDLE h_heap) { return HeapAlloc(h_heap, 0, sizeof(NODE)); }
	void  operator delete(void*   reserved_ptr, HANDLE h_heap) { HeapFree(h_heap, 0, reserved_ptr); }
};

int main() {
	R_TREE<4> rt;
	return 0;
}







