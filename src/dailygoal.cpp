#include "Dailygoal.hpp"
#include "Flips.hpp"
#include "FlipUtils.hpp"

date::date()
{
	time_t now = time(0);

	tm *ltm = localtime(&now);
	this->day 	= ltm->tm_mday;
	this->month = ltm->tm_mon;
	this->year 	= ltm->tm_year;
}

date::date(const nlohmann::json& json_object)
{
	this->day 	= json_object["day"];
	this->month = json_object["month"];
	this->year 	= json_object["year"];
}

void date::set_date(nlohmann::json& json_object)
{
	json_object["day"] 		= this->day;
	json_object["month"] 	= this->month;
	json_object["year"] 	= this->year;
}

daily_progress::daily_progress()
{
	this->valid_data = false;

	/* Get the date */
	today = date();

	/* Initialize the json data or create new data
	 * if there isn't any existing data */
	this->file_path = flips::data_path + "/daily_goal.json";

	/* File doesn't exist */
	if (!std::filesystem::exists(file_path))
	{
		/* Create new data and store that into the file
		 * for the next time to be read */
		today.set_date(json_data);
		json_data["goal"] 		= default_goal;
		json_data["progress"] 	= 0;

		flip_utils::write_json_file(json_data, file_path);
	}
	else if (std::filesystem::is_regular_file(file_path))
	{
		const std::string json_string = flip_utils::read_file(file_path);
		this->json_data = nlohmann::json::parse(json_string);

		/* Check if the date is different.
		 * If so, reset the progress */
		const date old_date = date(this->json_data);

		if (old_date != today)
		{
			/* Reset the progress */
			json_data["progress"] = 0;

			/* Update the date */
			today.set_date(json_data);

			/* Update the file */
			flip_utils::write_json_file(json_data, file_path);
		}
	}
	else
	{
		std::cerr << "Can't read the daily progress file for some reason ¯\\_(ツ)_/¯ (its probably a directory)" << std::endl;
		return;
	}

	valid_data = true;
}

void daily_progress::add_progress(const int amount)
{
	int progress = json_data["progress"];
	progress += amount;
	json_data["progress"] = progress;

	flip_utils::write_json_file(this->json_data, this->file_path);
}

int daily_progress::current_progress() const
{
	return json_data["progress"];
}

int daily_progress::goal() const
{
	return json_data["goal"];
}

void daily_progress::print_progress() const
{
	const int progress 	= current_progress();
	const int goal 		= daily_progress::goal();
	const float progress_in_percent = ((float)progress / goal) * 100;

	std::cout << "\033[1mDaily progress: " << flip_utils::round_big_numbers(progress) << " / " << flip_utils::round_big_numbers(goal) << " (" << std::round(progress_in_percent) << "%)\033[0m" << std::endl;
}
