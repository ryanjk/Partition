#pragma once

#include <json11\json11.hpp>
#include <Utilities\Logging.h>

namespace pn {

#define as_type(name, type, Check, GetValue)\
	inline type as_##name(const json11::Json& val, type default_value = type()){\
		return val.Check() ? val.GetValue() : default_value;\
	}

	as_type(int, int, is_number, int_value);				// as_int
	as_type(string, std::string, is_string, string_value);	// as_string
	as_type(bool, bool, is_bool, bool_value);				// as_bool

#undef as_type

}