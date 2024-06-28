#include "DB.hpp"
#include "FilePaths.hpp"
#include "FlipUtils.hpp"

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

	/* Create the flip array */
	for (size_t i = 0; i < json_data["flips"].size(); i++)
	{
		/* Don't load cancelled flips */
		if (json_data["flips"][i]["cancelled"] == true)
			continue;

		flips.push_back(json_data["flips"][i]);
	}
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
