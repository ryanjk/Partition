#pragma once

#include <spdlog\spdlog.h>

namespace pn {

// ------------------ TYPEDEFS -------------------------

template<typename T>
using log_ptr = std::shared_ptr<T>;

// ---------------- VARIABLES -------------

extern log_ptr<spdlog::logger> console;

// ---------------- FUNCTIONS --------------

std::string ErrMsg(const HRESULT hr);
std::string ErrMsg(const DWORD error);

void InitLogger();
void CloseLogger();

extern constexpr const char* file_name_from_path(const char* str);

template<typename ... Args>
void __Log(const spdlog::level::level_enum level, const std::string& filename, const std::string& fn_name, const int line_number, const char* fmt, const Args& ... args) {
	auto message = fmt::format(fmt, args...);
	auto final_message = fmt::format("{3} ({0}:{1}:{2})", filename, fn_name, line_number, message);
	console->log(level, final_message);
}

template<typename T, typename ... Args>
const T& __LogValue(const spdlog::level::level_enum level, const std::string& filename, const std::string& fn_name, const int line_number, const char* fmt, const T& value, const Args& ... args) {
	auto message = fmt::format(fmt, value, args...);
	auto final_message = fmt::format("{3} ({0}:{1}:{2})", filename, fn_name, line_number, message);
	console->log(level, final_message);
	return value;
}

} // namespace pn

  // ----------------- MACRO DEFINITIONS ------------------
#define TYPENAME(t) typeid(t).name()

#define INFO spdlog::level::level_enum::info
#define DEBUG spdlog::level::level_enum::debug
#define ERR   spdlog::level::level_enum::err
#define TRACE spdlog::level::level_enum::trace

#define BaseLog(level, ...) pn::__Log(level, pn::file_name_from_path(__FILE__), __FUNCTION__, __LINE__, __VA_ARGS__)
#define Log(...) BaseLog(TRACE, __VA_ARGS__)
#define LogDebug(...) BaseLog(DEBUG, __VA_ARGS__)
#define LogError(...) BaseLog(ERR, __VA_ARGS__)
#define LogInfo(...) BaseLog(INFO, __VA_ARGS__)

#define BaseLogValue(level, ...) pn::__LogValue(level, pn::file_name_from_path(__FILE__), __FUNCTION__, __LINE__, __VA_ARGS__)
#define LogValue(...) BaseLogValue(TRACE, __VA_ARGS__)
#define LogValueDebug(...) BaseLogValue(DEBUG, __VA_ARGS__)
#define LogValueError(...) BaseLogValue(ERR, __VA_ARGS__)
#define LogValueInfo(...) BaseLogValue(INFO, __VA_ARGS__)
