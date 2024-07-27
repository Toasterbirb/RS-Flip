#include "DB.hpp"
#include "FilePaths.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"

#include <assert.h>
#include <doctest/doctest.h>
#include <filesystem>
#include <iostream>
#include <sys/types.h>

constexpr char DEFAULT_DATA_FILE[] = "{\"stats\":{\"profit\":0,\"flips_done\":0},\"flips\":[]}\n";

db::db()
{
	assert(!file_paths::data_path.empty());
	assert(!file_paths::data_file.empty());

	if (!std::filesystem::exists(file_paths::data_path))
		std::filesystem::create_directories(file_paths::data_path);

	if (!std::filesystem::exists(file_paths::data_file))
		create_default_data_file();

	/* Read the json data file */
	const std::string json_string = flip_utils::read_file(file_paths::data_file);

	try
	{
		json_data = nlohmann::json::parse(json_string);
	}
	catch (const std::exception& e)
	{
		std::cout << "The database is possibly corrupted. Restore a backup to proceed.\nError: " << e.what() << '\n';
		exit(1);
	}

	/* Validate the database before using it to avoid nuking any backups on write
	 * if there's still one around */

	if (!validate(json_data))
	{
		std::cout << "The json database is missing information. Restore a backup to proceed\n";
		exit(1);
	}
}

db::db(const nlohmann::json& json_data)
:json_data(json_data)
{}

void db::add_flip(const flips::flip& flip)
{
	json_data["flips"].emplace_back(flip.to_json());
}

TEST_CASE("Add a new flip")
{
	flips::flip flip_a;
	flip_a.account = "main";
	flip_a.buy_price = 1468;
	flip_a.cancelled = false;
	flip_a.done = false;
	flip_a.item = "Yew shieldbow";
	flip_a.buylimit = 4950;
	flip_a.sell_price = 1649;
	flip_a.sold_price = 0;

	nlohmann::json db_json;

	db db(db_json);

	CHECK(db.total_flip_count() == 0);
	db.add_flip(flip_a);
	CHECK(db.total_flip_count() == 1);
}

size_t db::total_flip_count() const
{
	return json_data.contains("flips") ? json_data["flips"].size() : 0;
}

flips::flip db::get_flip_obj(const u32 index) const
{
	return flips::flip(json_data["flips"].at(index));
}

std::vector<stats::avg_stat> db::get_flip_avg_stats() const
{
	return stats::flips_to_avg_stats(json_data["flips"]);
}

std::vector<u32> db::find_flips_by_name(const std::string& item_name) const
{
	std::vector<u32> result;
	const std::string item_name_lowercase = flip_utils::str_to_lower(item_name);

	/* Quit if zero flips done */
	if (get_stat(stat_key::flips_done) == 0)
		return result;

	for (u32 i = 0; i < total_flip_count(); ++i)
	{
		if (flip_utils::str_to_lower(get_flip<std::string>(i, flip_key::item)) != item_name_lowercase)
			continue;

		if (get_flip<bool>(i, flip_key::done))
			result.emplace_back(i);
	}

	return result;
}

std::vector<u32> db::find_flips_by_count(const u32 flip_count) const
{
	std::vector<u32> result;

	/* Quit if zero flips done */
	if (get_stat(stat_key::flips_done) == 0)
		return result;

	std::vector<stats::avg_stat> avg_stats = stats::flips_to_avg_stats(json_data["flips"]);
	for (size_t i = 0; i < avg_stats.size(); i++)
	{
		if (avg_stats[i].flip_count() <= flip_count)
			result.emplace_back(i);
	}

	return result;
}

i64 db::get_stat(const stat_key key) const
{
	return json_data["stats"][stat_key_to_str.at(key)];
}

void db::set_stat(const stat_key key, const i64 data)
{
	json_data["stats"][stat_key_to_str.at(key)] = data;
}

void db::write()
{
	assert(!file_paths::data_file.empty());

	/* Backup the file before writing anything */
	std::filesystem::copy_file(file_paths::data_file, file_paths::data_file + "_backup", std::filesystem::copy_options::overwrite_existing);

	flip_utils::write_json_file(json_data, file_paths::data_file);
}

bool db::validate(const nlohmann::json& json_obj)
{
	/* Check if the json file has all of the keys that should be there */

	bool key_flips = json_obj.contains("flips");
	bool key_stats = json_obj.contains("stats");
	bool key_flips_done = json_obj.at("stats").contains("flips_done");
	bool key_profit = json_obj.at("stats").contains("profit");

	return key_flips && key_stats && key_flips_done && key_profit;
}

void db::create_default_data_file()
{
	flip_utils::write_file(file_paths::data_file, DEFAULT_DATA_FILE);
}
