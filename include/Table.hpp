#pragma once
#include "pch.hpp"

class Table
{
public:
	Table(std::vector<std::string> column_names);
	void add_row(std::vector<std::string> data);
	void print() const;

private:
	std::vector<size_t> get_column_sizes() const;
	const std::vector<std::string> column_names;
	std::vector<std::vector<std::string>> data;
};
