#pragma once
#include "pch.hpp"

struct Date
{
	Date();
	Date(const nlohmann::json& json_object);
	void SetDate(nlohmann::json& json_object); /* Changes the date of the json_object */
	int day, month, year;

	bool operator==(const Date& other) const
	{
		return (day == other.day
				&& month == other.month
				&& year == other.year);
	}

	bool operator!=(const Date& other) const
	{
		return (day != other.day
				|| month != other.month
				|| year != other.year);
	}
};

class DailyProgress
{
public:
	DailyProgress();
	void AddProgress(const int amount);
	int CurrentProgress() const;
	int Goal() const;
	void PrintProgress() const;

private:
	std::string file_path;
	int default_goal = 15000000;
	nlohmann::json json_data;
	bool valid_data;
	Date today;
};
