#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

#include <string.h>

#include <Utilities\Memory.h>

namespace pn {

// ------------ TYPEDEFS ---------------

using string	= std::string;
using wstring	= std::wstring;

template<typename T>
using vector	= std::vector<T>;

template<typename K, typename V>
using map		= std::unordered_map<K, V>;

using bytes		= vector<char>;

// ------------ STRUCT/CLASS DEFINITIONS --------

template<typename T>
struct basic_frame_string {
	static linear_allocator*	frame_alloc;
	T*							data;
	size_t						length;

	basic_frame_string() : data(nullptr), length(0) {}
	basic_frame_string(const basic_frame_string& f) {
		length = f.length;
		data = Allocate(length);
		memcpy(data, f.data, sizeof(T)*length);
	}
	basic_frame_string(basic_frame_string&& f) {
		length = f.length;
		data = f.data;
		f.data = nullptr;
		f.length = 0;
	}
	basic_frame_string(const char* str) {
		length = strlen(str);
		data = Allocate(length);
		memcpy(data, str, sizeof(T)*length);
	}

	T& operator[](size_t index) {
		return data[index];
	}
	T operator[](size_t index) const {
		return data[index];
	}

	static void SetFrameAllocator(linear_allocator* fa) {
		frame_alloc = fa;
	}

private:
	T* Allocate(size_t n) {
		assert(frame_alloc != nullptr);
		return (T*) frame_alloc->Allocate(n);
	}

};

template<typename T>
linear_allocator* basic_frame_string<T>::frame_alloc = nullptr;

using frame_string = basic_frame_string<char>;

// ------------ FUNCTIONS -------------

// --------- CONTAINER FUNCTIONS -----

template<typename Container>
auto	Size(const Container& container) -> decltype(container.size()) {
	return container.size();
}

template<typename Container>
void	Clear(Container& container) {
	container.clear();
}

// ------- STRING FUNCTIONS ----------

inline vector<string>	Split(const string& s, char delim) {
	std::stringstream ss(s);
	vector<string> elements;
	string element;
	while (std::getline(ss, element, delim)) {
		elements.emplace_back(element);
	}
	return elements;
}

inline void				Insert(string& s, char c) {
	s.push_back(c);
}

inline const char*		CString(const string& s) {
	return s.c_str();
}

// ------- VECTOR FUNCTIONS -----------

template<typename T, typename U>
void	Insert(vector<T>& vec, U&& value) {
	vec.push_back(std::forward<U>(value));
}

template<typename T, typename U, typename... Us>
void	Insert(vector<T>& vec, U&& value, Us&&... args) {
	vec.push_back(std::forward<U>(value));
	Insert(vec, args...);
}

template<typename T>
auto	Pop(vector<T>& vec) -> decltype(vec.pop_back()) {
	return vec.pop_back();
}

template<typename T, typename SizeType>
void	Reserve(vector<T>& vec, const SizeType s) {
	vec.reserve(s);
}

template<typename T, typename SizeType>
void	Resize(vector<T>& vec, const SizeType s) {
	vec.resize(s);
}

template<typename T>
auto	Get(const vector<T>& v, size_t i) -> decltype(v[i]) {
	return v[i];
}

template<typename T>
auto	Get(vector<T>& v, size_t i) -> decltype(v[i]) {
	return v[i];
}

// -------- MAP FUNCTIONS ------------

template<typename K, typename V>
bool	Contains(const map<K,V>& m, const K& key) {
	return m.find(key) != m.end();
}

template<typename K, typename V>
void	Insert(map<K,V>& m, K&& key, V&& value) {
	if (Contains(m, key)) return;
	m.insert(std::make_pair(std::forward<K>(key), std::forward<V>(value)));
}

template<typename K, typename V>
auto	Get(const map<K,V>& m, const K& key) {
	return m.at(key);
}


} // namespace pn