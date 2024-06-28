#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

class table
{
public:
	table(const std::vector<std::string>& column_names);
	void add_row(const std::vector<std::string>& data);
	u64 row_count() const;
	void print() const;
	void clear();

private:
	std::vector<size_t> get_column_sizes() const;
	const std::vector<std::string> column_names;
	std::vector<std::vector<std::string>> data;
};
