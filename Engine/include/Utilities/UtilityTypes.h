#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace pn {

// ------------ TYPEDEFS ---------------

using string	= std::string;
using wstring	= std::wstring;

template<typename T>
using vector	= std::vector<T>;

template<typename K, typename V>
using map		= std::unordered_map<K, V>;

using bytes		= vector<char>;

// ------------ FUNCTIONS -------------

// --------- CONTAINER FUNCTIONS -----

template<typename Container>
auto	Size(const Container& vec) -> decltype(vec.size()) {
	return vec.size();
}

// ------- VECTOR FUNCTIONS -----------

template<typename Vec, typename T>
void	Insert(Vec& vec, T&& value) {
	vec.push_back(std::forward<T>(value));
}

template<typename Vec, typename T, typename... Ts>
void	Insert(Vec& vec, T&& value, Ts&&... args) {
	vec.push_back(std::forward<T>(value));
	Insert(vec, args...);
}

template<typename Vec>
auto	Pop(Vec& vec) -> decltype(vec.pop_back()) {
	return vec.pop_back();
}

template<typename Vec, typename SizeType>
void	Reserve(Vec& vec, const SizeType s) {
	vec.reserve(s);
}

template<typename Vec, typename SizeType>
void	Resize(Vec& vec, const SizeType s) {
	vec.resize(s);
}

template<typename T>
auto	Get(const vector<T>& v, size_t i) -> decltype(v[i]) {
	return v[i];
}

// -------- MAP FUNCTIONS ------------

template<typename K, typename V>
void	Contains(const map<K,V>& m, const K& key) {
	return m.find(key) != m.end();
}

template<typename K, typename V>
void	Insert(map<K,V>& m, K&& key, V&& value) {
	if (Contains(m, key)) return;
	m.insert(std::make_pair(std::forward<K>(key), std::forward<V>(value)));
}

template<typename K, typename V>
auto	Get(const map<K,V>& m, const K& key) -> decltype(m.at(key)) {
	return m.at(key);
}


} // namespace pn