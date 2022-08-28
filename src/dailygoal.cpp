#include "Dailygoal.hpp"
#include "Flips.hpp"
#include "Utils.hpp"

Date::Date()
{
	time_t now = time(0);

	tm *ltm = localtime(&now);
	this->day 	= ltm->tm_mday;
	this->month = ltm->tm_mon;
	this->year 	= ltm->tm_year;
}

Date::Date(const nlohmann::json& json_object)
{
	this->day 	= json_object["day"];
	this->month = json_object["month"];
	this->year 	= json_object["year"];
}

void Date::SetDate(nlohmann::json& json_object)
{
	json_object["day"] 		= this->day;
	json_object["month"] 	= this->month;
	json_object["year"] 	= this->year;
}

DailyProgress::DailyProgress()
{
	this->valid_data = false;

	/* Get the date */
	today = Date();

	/* Initialize the json data or create new data
	 * if there isn't any existing data */
	this->file_path = Flips::data_path + "/daily_goal.json";

	/* File doesn't exist */
	if (!std::filesystem::exists(file_path))
	{
		/* Create new data and store that into the file
		 * for the next time to be read */
		today.SetDate(json_data);
		json_data["goal"] 		= default_goal;
		json_data["progress"] 	= 0;

		Utils::WriteJsonFile(json_data, file_path);
	}
	else if (std::filesystem::is_regular_file(file_path))
	{
		std::string json_string = Utils::ReadFile(file_path);
		this->json_data = nlohmann::json::parse(json_string);

		/* Check if the date is different.
		 * If so, reset the progress */
		Date old_date = Date(this->json_data);

		if (old_date != today)
		{
			/* Reset the progress */
			json_data["progress"] = 0;

			/* Update the date */
			today.SetDate(json_data);

			/* Update the file */
			Utils::WriteJsonFile(json_data, file_path);
		}
	}
	else
	{
		std::cerr << "Can't read the daily progress file for some reason ¯\\_(ツ)_/¯ (its probably a directory)" << std::endl;
		return;
	}

	valid_data = true;
}

void DailyProgress::AddProgress(const int& amount)
{
	int progress = json_data["progress"];
	progress += amount;
	json_data["progress"] = progress;

	Utils::WriteJsonFile(this->json_data, this->file_path);
}

int DailyProgress::CurrentProgress()
{
	return json_data["progress"];
}

int DailyProgress::Goal()
{
	return json_data["goal"];
}

void DailyProgress::PrintProgress()
{
	int progress 	= CurrentProgress();
	int goal 		= Goal();
	float progress_in_percent = ((float)progress / goal) * 100;

	std::cout << "\e[1mDaily progress: " << Utils::RoundBigNumbers(progress) << " / " << Utils::RoundBigNumbers(goal) << " (" << std::round(progress_in_percent) << "%)\e[0m" << std::endl;
}
