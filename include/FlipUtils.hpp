#pragma once

#include "Types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_set>

namespace flip_utils
{
	__attribute__((hot))
	std::string clean_decimals(const f64 value);

	/* 1000 -> 1k, 1000000 -> 1m etc. */
	__attribute__((hot))
	std::string round_big_numbers(const long number);

	__attribute__((hot))
	std::string round(const f64 value, const i32 decimals); /* Round a value with given accuracy */
	void print_title(const std::string& text); /* #### Prints like this #### */
	std::string read_file(const std::string& filepath);
	std::unordered_set<std::string> read_file_items(const std::string& filepath); /* Read unique item lines from a file */
	void write_file(const std::string& filepath, const std::string& text);
	void write_json_file(const nlohmann::json& json_data, const std::string& file_path);
	std::string str_to_lower(const std::string& str);

	// Function that approaches a given value but never really reaches it
	// After the point of diminishing_returns, the value starts incresing slower
	// By lowering the slope value, you can make the value increase faster
	f64 limes(const f64 approach_value, const f64 diminishing_returns, const f64 slope, const f64 value);

	// Create a string with ANSI escape code colors or other formatting
	std::string color_format_string(const u8 color_code, const std::string& text);
}
