#include <iostream>
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
		std::cout << "####| " << text << " |####\n";
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
}
