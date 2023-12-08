#pragma once
#include "pch.hpp"

class Table
{
public:
	Table(std::vector<std::string> column_names);
	void add_row(std::vector<std::string> data);
	void print() const;
	void update_column_sizes();

private:
	const std::vector<std::string> column_names;
	std::vector<size_t> column_size;
	std::vector<std::vector<std::string>> data;
};
