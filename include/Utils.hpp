#pragma ocne
#include "pch.hpp"

namespace Utils
{
	std::string CleanDecimals(const double& value);

	/* 1000 -> 1k, 1000000 -> 1m etc. */
	std::string RoundBigNumbers(const int& number);
	void PrintTitle(const std::string& text); /* #### Prints like this #### */
	int Clamp(const int& value, const int& min, const int& max);
}
