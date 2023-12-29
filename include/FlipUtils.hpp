#pragma once
#include "pch.hpp"

namespace FlipUtils
{
	std::string CleanDecimals(const double value);

	double JsonAverage(const std::vector<nlohmann::json>& data, const std::string& key);

	/* Sort the given json vector in-place based on the values gotten with the key */
	void JsonSort(std::vector<nlohmann::json>& data, const std::string& key);

	/* Find the lowest value from the json array with the given key */
	int JsonMinInt(const std::vector<nlohmann::json>& data, const std::string& key);

	/* Find the highest value from the json array with the given key */
	int JsonMaxInt(const std::vector<nlohmann::json>& data, const std::string& key);

	/* 1000 -> 1k, 1000000 -> 1m etc. */
	std::string RoundBigNumbers(const int number);
	std::string Round(const double value, const int decimals); /* Round a value with given accuracy */
	void PrintTitle(const std::string& text); /* #### Prints like this #### */
	int Clamp(const int value, const int min, const int max);
	std::string ReadFile(const std::string& filepath);
	std::unordered_set<std::string> ReadFileItems(const std::string& filepath); /* Read unique item lines from a file */
	void WriteFile(const std::string& filepath, const std::string& text);
	void WriteJsonFile(const nlohmann::json& json_data, const std::string& file_path);

	// Function that approaches a given value but never really reaches it
	// After the point of diminishing_returns, the value starts incresing slower
	// By lowering the slope value, you can make the value increase faster
	double Limes(const double approach_value, const double diminishing_returns, const double slope, const double value);
}
