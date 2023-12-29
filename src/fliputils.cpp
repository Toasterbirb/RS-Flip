#include "FlipUtils.hpp"

namespace FlipUtils
{
	std::string CleanDecimals(const double value)
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


	double JsonAverage(const std::vector<nlohmann::json>& data, const std::string& key)
	{
		double total = 0;
		for (const nlohmann::json& value : data)
			total = total + static_cast<double>(value[key]);

		return total / data.size();
	}

	void JsonSort(std::vector<nlohmann::json>& data, const std::string& key)
	{
		std::sort(data.begin(), data.end(), [key](const nlohmann::json& a, const nlohmann::json& b) {
			return a[key] < b[key];
		});
	}

	int JsonMinInt(const std::vector<nlohmann::json>& data, const std::string& key)
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

		CHECK(JsonMinInt(json_data, "test") == -5);
	}

	int JsonMaxInt(const std::vector<nlohmann::json>& data, const std::string& key)
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

		CHECK(JsonMaxInt(json_data, "test") == 4);
	}


	std::string RoundBigNumbers(const int number)
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

	std::string Round(const double value, const int decimals)
	{
		return CleanDecimals(std::round(value * std::pow(10, decimals)) / std::pow(10, decimals));
	}

	TEST_CASE("Round to accuracy")
	{
		CHECK(Round(0.0001, 1) == "0");
		CHECK(Round(0.5, 0) == "1");
		CHECK(Round(0.1234, 2) == "0.12");
		CHECK(Round(-5.05, 1) == "-5.1");
	}

	void PrintTitle(const std::string& text)
	{
		std::cout << "\033[1m\033[32m#####| " << text << " |#####\033[0m\n";
	}

	int Clamp(const int value, const int min, const int max)
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

	std::unordered_set<std::string> ReadFileItems(const std::string& filepath)
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

	void WriteFile(const std::string& filepath, const std::string& text)
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

	void WriteJsonFile(const nlohmann::json& json_data, const std::string& file_path)
	{
		std::ofstream file(file_path);
		file << std::setw(4) << json_data << std::endl;
	}

	double Limes(const double approach_value, const double diminishing_returns, const double slope, const double value)
	{
		if (value < 0.001)
			return -300;
		return approach_value - diminishing_returns / value * slope;
	}

	TEST_CASE("Limes")
	{
		CHECK(Limes(2, 1, 1, 1) == 1.0);
		CHECK(Limes(2, 2, 1, 1) == 0);
		CHECK(Limes(2, 2, 1, 2) == 1);
	}
}
