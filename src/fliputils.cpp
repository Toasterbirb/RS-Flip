#include "FlipUtils.hpp"

#include <doctest/doctest.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace flip_utils
{
	std::string clean_decimals(const f64 value)
	{
		std::string result = std::to_string(value);

		/* If the last char is not a zero, return early */
		if (result.at(result.size() - 1) != '0')
			return result;

		size_t first_non_zero_pos = result.size() - 1;
		for (i32 i = result.size() - 2; i > 0; --i)
		{
			if (result[i] != '0')
			{
				first_non_zero_pos = i;
				break;
			}
		}

		if (first_non_zero_pos != std::string::npos)
			result.erase(first_non_zero_pos + 1, result.size() - first_non_zero_pos);

		/* Check if the last char is a dot */
		if (result[result.size() - 1] == '.')
			result.erase(result.size() - 1, 1);

		return result;
	}

	std::string round_big_numbers(const long number)
	{
		if (number > 1'000'000 || number < -1'000'000)
			return clean_decimals((double)number / 1'000'000) + "m";
		else if (number > 1000 || number < -1000)
			return clean_decimals((double)number / 1000) + "k";

		/* Small enough number to not need rounding */
		return std::to_string(number);
	}

	TEST_CASE("Rounding big numbers into text format")
	{
		/* Positive numbers */
		CHECK(round_big_numbers(20'000) == "20k");
		CHECK(round_big_numbers(9500) == "9.5k");
		CHECK(round_big_numbers(150) == "150");
		CHECK(round_big_numbers(1'250'000) == "1.25m");
		CHECK(round_big_numbers(3'000'000) == "3m");

		/* Negative numbers */
		CHECK(round_big_numbers(-20'000) == "-20k");
		CHECK(round_big_numbers(-9500) == "-9.5k");
		CHECK(round_big_numbers(-150) == "-150");
		CHECK(round_big_numbers(-1'250'000) == "-1.25m");
		CHECK(round_big_numbers(-3'000'000) == "-3m");
	}

	std::string round(const f64 value, const i32 decimals)
	{
		return clean_decimals(std::round(value * std::pow(10, decimals)) / std::pow(10, decimals));
	}

	TEST_CASE("Round to accuracy")
	{
		CHECK(round(0.0001, 1) == "0");
		CHECK(round(0.5, 0) == "1");
		CHECK(round(0.1234, 2) == "0.12");
		CHECK(round(-5.05, 1) == "-5.1");
	}

	void print_title(const std::string& text)
	{
		std::cout << "\033[1m\033[32m#####| " << text << " |#####\033[0m\n";
	}

	std::string read_file(const std::string& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			std::cerr << "File " << filepath << " couldn't be opened!\n";
			return "";
		}

		file.seekg(0, std::ios::end);
		size_t file_size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::string contents;
		contents.resize(file_size);

		file.read(&contents[0], file_size);

		return contents;
	}

	std::unordered_set<std::string> read_file_items(const std::string& filepath)
	{
		std::unordered_set<std::string> contents;

		std::ifstream file(filepath);

		/* Check if the file exists / can be opened */
		if (!file.is_open())
			return contents;

		std::string line;
		while (std::getline(file, line))
			contents.insert(line);

		return contents;
	}

	void write_file(const std::string& filepath, const std::string& text)
	{
		std::ofstream file(filepath);
		if (!file.is_open())
		{
			std::cerr << "Can't open the data file!\n";
			return;
		}

		file << text;
	}

	void write_json_file(const nlohmann::json& json_data, const std::string& file_path)
	{
		std::ofstream file(file_path);
		file << std::setw(4) << json_data << std::endl;
	}

	f64 limes(const f64 approach_value, const f64 diminishing_returns, const f64 slope, const f64 value)
	{
		return value < 0.001 ? -300 : approach_value - diminishing_returns / value * slope;
	}

	TEST_CASE("Limes")
	{
		CHECK(limes(2, 1, 1, 1) == 1.0);
		CHECK(limes(2, 2, 1, 1) == 0);
		CHECK(limes(2, 2, 1, 2) == 1);
	}

	std::string color_format_string(const u8 color_code, const std::string& text)
	{
		return "\033[" + std::to_string(color_code) + 'm' + text + "\033[0m";
	}
}
