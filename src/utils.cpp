#include "Utils.hpp"
#include "doctest/doctest.h"

namespace Utils
{
	std::string CleanDecimals(const double& value)
	{
		std::string result = std::to_string(value);
		int size = (int)result.size();
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


	std::string RoundBigNumbers(const int& number)
	{
		if (number > 1000000 || number < -1000000)
			return CleanDecimals((double)number / 1000000) + "m";
		else if (number > 1000 || number < -1000)
			return CleanDecimals((double)number / 1000) + "k";

		/* Small enough number to not need rounding */
		return std::to_string(number);
	}

	TEST_CASE("Rounding big numbers into text format")
	{
		/* Positive numbers */
		CHECK(RoundBigNumbers(20000) == "20k");
		CHECK(RoundBigNumbers(9500) == "9.5k");
		CHECK(RoundBigNumbers(150) == "150");
		CHECK(RoundBigNumbers(1250000) == "1.25m");
		CHECK(RoundBigNumbers(3000000) == "3m");

		/* Negative numbers */
		CHECK(RoundBigNumbers(-20000) == "-20k");
		CHECK(RoundBigNumbers(-9500) == "-9.5k");
		CHECK(RoundBigNumbers(-150) == "-150");
		CHECK(RoundBigNumbers(-1250000) == "-1.25m");
		CHECK(RoundBigNumbers(-3000000) == "-3m");
	}

	void PrintTitle(const std::string& text)
	{
		std::cout << "\e[1m\e[32m#####| " << text << " |#####\e[0m\n";
	}

	int Clamp(const int& value, const int& min, const int& max)
	{
		if (value < min)
			return min;
		else if (value > max)
			return max;

		return value;
	}

	TEST_CASE("Clamp integer values")
	{
		CHECK(Clamp(15, 0, 10) 	== 10);
		CHECK(Clamp(3, -5, 5) 	== 3);
		CHECK(Clamp(-2, 5, 10) 	== 5);
	}

	std::string ReadFile(const std::string& filepath)
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

	void WriteFile(const std::string& filepath, const std::string text)
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

	void WriteJsonFile(nlohmann::json json_data, std::string file_path)
	{
		std::ofstream file(file_path);
		file << std::setw(4) << json_data << std::endl;
	}
}
