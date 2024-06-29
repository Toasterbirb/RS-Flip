#include "DB.hpp"
#include "FilePaths.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"

#include <assert.h>
#include <filesystem>

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
	json_data = nlohmann::json::parse(json_string);

	initialize_flip_array();
}

void db::initialize_flip_array()
{
	assert(!json_data.empty());
	assert(json_data.contains("flips"));

	/* Create the flip array */
	for (size_t i = 0; i < total_flip_count(); i++)
	{
		/* Don't load cancelled flips */
		if (get_flip<bool>(i, flip_key::cancelled) == true)
			continue;

		/* If the flip doesn't specify an account, set it to "main" */
		if (!json_data["flips"][i].contains("account"))
			json_data["flips"][i]["account"] = "main";

		flips.push_back(json_data["flips"][i]);
	}
}

void db::add_flip(const flips::flip& flip)
{
	flips.emplace_back(flip.to_json());
	apply_flip_array();
}

size_t db::total_flip_count() const
{
	return json_data["flips"].size();
}

flips::flip db::get_flip_obj(const u32 index) const
{
	return flips::flip(flips.at(index));
}

std::vector<stats::avg_stat> db::get_flip_avg_stats() const
{
	return stats::flips_to_avg_stats(flips);
}

std::vector<u32> db::find_flips_by_name(const std::string& item_name) const
{
	std::vector<u32> result;

	/* Quit if zero flips done */
	if (get_stat(stat_key::flips_done) == 0)
		return result;

	for (u32 i = 0; i < total_flip_count(); ++i)
	{
		if (get_flip<std::string>(i, flip_key::item) != item_name)
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

	std::vector<stats::avg_stat> avg_stats = stats::flips_to_avg_stats(flips);
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

void db::apply_flip_array()
{
	json_data["flips"] = flips;
	write();
}

void db::write()
{
	assert(!file_paths::data_file.empty());

	/* Backup the file before writing anything */
	std::filesystem::copy_file(file_paths::data_file, file_paths::data_file + "_backup", std::filesystem::copy_options::overwrite_existing);

	flip_utils::write_json_file(json_data, file_paths::data_file);
}

void db::create_default_data_file()
{
	flip_utils::write_file(file_paths::data_file, DEFAULT_DATA_FILE);
}
