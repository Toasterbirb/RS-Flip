#pragma once
#include "pch.hpp"

struct date
{
	date();
	explicit date(const nlohmann::json& json_object);
	void set_date(nlohmann::json& json_object); /* Changes the date of the json_object */
	int day, month, year;

	bool operator==(const date& other) const
	{
		return (day == other.day
				&& month == other.month
				&& year == other.year);
	}

	bool operator!=(const date& other) const
	{
		return (day != other.day
				|| month != other.month
				|| year != other.year);
	}
};

class daily_progress
{
public:
	daily_progress();
	void add_progress(const int amount);
	int current_progress() const;
	int goal() const;
	void print_progress() const;

private:
	std::string file_path;
	int default_goal = 15000000;
	nlohmann::json json_data;
	bool valid_data;
	date today;
};
