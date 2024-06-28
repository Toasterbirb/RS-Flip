#include "FlipUtils.hpp"

#include <algorithm>
#include <doctest/doctest.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace flip_utils
{
	std::string clean_decimals(const double value)
	{
		std::string result = std::to_string(value);
		const int size = (int)result.size();
		for (int i = size - 1; i > 0; i--)
		{
			if (result[i] != '0' && i < size - 1)
			{
				result.erase(i + 1, size);
				break;
			}
		}

		/* Check if the last char is a dot */
		if (result[result.size() - 1] == '.')
			result.erase(result.size() - 1, 1);

		return result;
	}


	double json_average(const std::vector<nlohmann::json>& data, const std::string& key)
	{
		double total = 0;
		for (const nlohmann::json& value : data)
			total = total + static_cast<double>(value[key]);

		return total / data.size();
	}

	void json_sort(std::vector<nlohmann::json>& data, const std::string& key)
	{
		std::sort(data.begin(), data.end(), [key](const nlohmann::json& a, const nlohmann::json& b) {
			return a[key] < b[key];
		});
	}

	int json_min_int(const std::vector<nlohmann::json>& data, const std::string& key)
	{
		int lowest_value = INT32_MAX;
		for (const nlohmann::json& j : data)
			if (j[key] < lowest_value)
				lowest_value = j[key];

		return lowest_value;
	}

	TEST_CASE("JsonMin")
	{
		std::vector<nlohmann::json> json_data;
		for (int i = -5; i < 5; ++i)
		{
			nlohmann::json j;
			j["test"] = i;
			json_data.push_back(j);
		}

		CHECK(json_min_int(json_data, "test") == -5);
	}

	int json_max_int(const std::vector<nlohmann::json>& data, const std::string& key)
	{
		int highest_value = INT32_MIN;
		for (const nlohmann::json& j : data)
			if (j[key] > highest_value)
				highest_value = j[key];

		return highest_value;
	}

	TEST_CASE("JsonMin")
	{
		std::vector<nlohmann::json> json_data;
		for (int i = -5; i < 5; ++i)
		{
			nlohmann::json j;
			j["test"] = i;
			json_data.push_back(j);
		}

		CHECK(json_max_int(json_data, "test") == 4);
	}


	std::string round_big_numbers(const long number)
	{
		if (number > 1000000 || number < -1000000)
			return clean_decimals((double)number / 1000000) + "m";
		else if (number > 1000 || number < -1000)
			return clean_decimals((double)number / 1000) + "k";

		/* Small enough number to not need rounding */
		return std::to_string(number);
	}

	TEST_CASE("Rounding big numbers into text format")
	{
		/* Positive numbers */
		CHECK(round_big_numbers(20000) == "20k");
		CHECK(round_big_numbers(9500) == "9.5k");
		CHECK(round_big_numbers(150) == "150");
		CHECK(round_big_numbers(1250000) == "1.25m");
		CHECK(round_big_numbers(3000000) == "3m");

		/* Negative numbers */
		CHECK(round_big_numbers(-20000) == "-20k");
		CHECK(round_big_numbers(-9500) == "-9.5k");
		CHECK(round_big_numbers(-150) == "-150");
		CHECK(round_big_numbers(-1250000) == "-1.25m");
		CHECK(round_big_numbers(-3000000) == "-3m");
	}

	std::string round(const double value, const int decimals)
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

		std::string contents( 	(std::istreambuf_iterator<char>(file)),
								(std::istreambuf_iterator<char>()));
		file.close();
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

		file.close();
		return contents;
	}

	void write_file(const std::string& filepath, const std::string& text)
	{
		std::ofstream file(filepath);
		if (!file.is_open())
		{
			std::cerr << "Can't open the data file!" << std::endl;
			return;
		}

		file << text;
		file.close();
	}

	void write_json_file(const nlohmann::json& json_data, const std::string& file_path)
	{
		std::ofstream file(file_path);
		file << std::setw(4) << json_data << std::endl;
	}

	double limes(const double approach_value, const double diminishing_returns, const double slope, const double value)
	{
		if (value < 0.001)
			return -300;
		return approach_value - diminishing_returns / value * slope;
	}

	TEST_CASE("Limes")
	{
		CHECK(limes(2, 1, 1, 1) == 1.0);
		CHECK(limes(2, 2, 1, 1) == 0);
		CHECK(limes(2, 2, 1, 2) == 1);
	}
}
