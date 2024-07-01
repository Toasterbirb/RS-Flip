#pragma once

#include "Types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_set>

namespace flip_utils
{
	std::string clean_decimals(const double value);

	/* 1000 -> 1k, 1000000 -> 1m etc. */
	std::string round_big_numbers(const long number);
	std::string round(const double value, const int decimals); /* Round a value with given accuracy */
	void print_title(const std::string& text); /* #### Prints like this #### */
	std::string read_file(const std::string& filepath);
	std::unordered_set<std::string> read_file_items(const std::string& filepath); /* Read unique item lines from a file */
	void write_file(const std::string& filepath, const std::string& text);
	void write_json_file(const nlohmann::json& json_data, const std::string& file_path);

	// Function that approaches a given value but never really reaches it
	// After the point of diminishing_returns, the value starts incresing slower
	// By lowering the slope value, you can make the value increase faster
	double limes(const double approach_value, const double diminishing_returns, const double slope, const double value);

	// Create a string with ANSI escape code colors or other formatting
	std::string color_format_string(const u8 color_code, const std::string& text);
}
