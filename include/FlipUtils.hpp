#pragma once
#include "pch.hpp"

namespace FlipUtils
{
	std::string CleanDecimals(const double& value);

	/* 1000 -> 1k, 1000000 -> 1m etc. */
	std::string RoundBigNumbers(const int& number);
	void PrintTitle(const std::string& text); /* #### Prints like this #### */
	int Clamp(const int& value, const int& min, const int& max);
	std::string ReadFile(const std::string& filepath);
	std::unordered_set<std::string> ReadFileItems(const std::string& filepath); /* Read unique item lines from a file */
	void WriteFile(const std::string& filepath, const std::string& text);
	void WriteJsonFile(nlohmann::json json_data, std::string file_path);
}
