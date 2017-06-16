#pragma once

#include <string>
#include <vector>

namespace pn {

// ------------ TYPEDEFS ---------------

using string	= std::string;
using wstring	= std::wstring;

template<typename T>
using vector	= std::vector<T>;

using bytes = vector<char>;


} // namespace pn