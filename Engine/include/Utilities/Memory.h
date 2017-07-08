#pragma once

#include <bitset>
#include <type_traits>
#include <cassert>

namespace pn {

template<typename T, unsigned int N>
class pool_allocator {
	struct alignas(sizeof(T)) free_memory_node {
		free_memory_node* next;
	};
	free_memory_node* head;
	free_memory_node memory[N];

	T* Allocate() {
		if (!HasFree()) return nullptr;
		T* new_object = (T*) head;
		head = head->next;
		return new_object;
	}

public:
	pool_allocator() {
		head = &memory[0];
		free_memory_node* current = head;
		for (int i = 0; i < N - 1; ++i) {
			current->next = current + 1;
			current = current->next;
		}
		current->next = nullptr;
	}

	bool HasFree() const {
		return head != nullptr;
	}

	template<typename... Args>
	T* Create(Args&&... args) {
		T* new_object_address = Allocate();
		if (new_object_address == nullptr) return nullptr;
		return new (new_object_address) T(std::forward<Args>(args)...);
	}

	void Release(T* obj) {
		assert(obj != nullptr);
		free_memory_node* new_head = (free_memory_node*) obj;
		assert(0 <= (new_head - memory) && (new_head - memory) < N);
		assert(((char*) new_head - (char*) memory) % sizeof(T) == 0);

		obj->~T();
		new_head->next = head;
		head = new_head;
	}
};

template<unsigned int N>
class linear_allocator {
	char	memory[N];
	char*	offset;

public:
	linear_allocator() {
		offset = memory;
	}

	bool HasFree() const {
		return offset < memory + N;
	}

	bool HasFree(unsigned int space_for) const {
		return (offset + space_for) <= (memory + N);
	}

	char* Allocate(unsigned int n) {
		assert(HasFree(n));
		char* allocation = offset;
		offset += n;
		return allocation;
	}

	template<typename T, typename... Args>
	T* Create(Args&&... args) {
		auto* allocation = Allocate(sizeof(T));
		return new (allocation) T(std::forward<Args>(args)...);
	}

	void Release() {
		offset = memory;
	}

};

}