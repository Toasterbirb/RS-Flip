#pragma once

#include <nlohmann/json.hpp>
#include <vector>

class db
{
public:
	db();

	std::vector<nlohmann::json> flips;
	nlohmann::json json_data;

	void apply_flip_array();
	void write(); /* Write the DB to disk */

private:
	void create_default_data_file();
};
