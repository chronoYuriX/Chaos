#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <stdio.h>

template <typename TYPE_ELEMENT>
struct RING_BUFFER {
	size_t        buffer_size, pointer;
	TYPE_ELEMENT* buffer;
	HANDLE        h_heap;
	RING_BUFFER(size_t initial_size, bool reserve_0 = true, HANDLE storage_location = nullptr): buffer_size(initial_size), pointer(0) {
		if (reserve_0) initial_size++;
		h_heap = (storage_location == nullptr) ? GetProcessHeap() : storage_location;
		buffer = (TYPE_ELEMENT*)HeapAlloc(h_heap, 0, initial_size * sizeof(TYPE_ELEMENT));
		if (buffer != nullptr && reserve_0) buffer[initial_size - 1] = (TYPE_ELEMENT)0;
	}
	~RING_BUFFER() {
		if (buffer != nullptr) {
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

int main() {
	printf("Write test:\n");
	RING_BUFFER<char> rb(16);
	ZeroMemory(rb.buffer, rb.buffer_size);
	for (char c = 'A'; c <= 'Z'; c++) {
		rb.write(c);
		printf("%s\n", rb.buffer);
	}
	
	printf("Read test:\n");
	char buffer[64];
	rb.readback(buffer, 3);
	printf("Readback 3: %s\n", buffer);
	rb.readback(buffer, 48);
	printf("Readback 48: %s\n", buffer);
	
	printf("Check test:\n");
	printf("Checkback 3: %s\n", rb.checkback("XYZ", 3) ? "True" : "False");
	printf("Checkback 48: %s\n", rb.checkback("KLMNOPQRSTUVWXYZKLMNOPQRSTUVWXYZKLMNOPQRSTUVWXYZ", 48) ? "True" : "False");
	return 0;
}












