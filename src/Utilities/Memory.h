#pragma once

#include <bitset>
#include <type_traits>
#include <cassert>

namespace pn {

template<typename T>
class pool_allocator {
	struct alignas(sizeof(T)) free_memory_node {
		free_memory_node* next;
	};
	free_memory_node*	head;
	free_memory_node*	memory;
	size_t				N;

	T* Allocate() {
		if (!HasFree()) return nullptr;
		T* new_object = (T*) head;
		head = head->next;
		return new_object;
	}

public:
	pool_allocator(size_t N) {
		memory = new free_memory_node[N];
		head = memory;
		free_memory_node* current = head;
		for (int i = 0; i < N - 1; ++i) {
			current->next = current + 1;
			current = current->next;
		}
		current->next = nullptr;
	}
	
	~pool_allocator() {
		delete memory;
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

class linear_allocator {
	char*	memory;
	char*	offset;
	size_t	N;

public:
	linear_allocator(size_t N) : N(N) {
		memory = new char[N];
		offset = memory;
	}
	~linear_allocator() {
		delete memory;
	}

	linear_allocator(const linear_allocator&)				= delete;
	linear_allocator(linear_allocator&&)					= delete;
	linear_allocator& operator=(const linear_allocator&)	= delete;
	linear_allocator& operator=(linear_allocator&&)			= delete;

	bool HasFree() const {
		return offset < memory + N;
	}

	bool HasFree(unsigned int space_for) const {
		return (offset + space_for) <= (memory + N);
	}

	void* Allocate(unsigned int n) {
		assert(HasFree(n));
		void* allocation = offset;
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