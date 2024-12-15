#include "Table.hpp"

#include <assert.h>
#include <doctest/doctest.h>
#include <iomanip>
#include <iostream>
#include <numeric>

constexpr int COLUMN_PADDING = 4;

table::table(const std::vector<std::string>& column_names)
:column_names(column_names)
{
	assert(column_names.size() > 1);
}

void table::add_row(const std::vector<std::string>& data)
{
	assert(data.size() == column_names.size());
	this->data.push_back(data);
}

u64 table::row_count() const
{
	return data.size();
}

void table::print() const
{
	assert(data.size() > 0);

	/* Get the column sizes */
	const std::vector<size_t> column_size = get_column_sizes();

	assert(column_size.size() == column_names.size());
	assert(column_size.size() == data.at(0).size());

	std::cout << std::left;

	/* Print the column names */
	for (size_t i = 0; i < column_names.size() - 1; ++i)
		std::cout << "\033[1m" << std::setw(column_size.at(i)) << column_names.at(i) << "\033[0m";

	std::cout << column_names.at(column_names.size() - 1) << "\n";

	/* Print a divider */
	const size_t divider_size = std::accumulate(column_size.begin(), column_size.end(), 0) - COLUMN_PADDING;
	for (size_t i = 0; i < divider_size; ++i) std::cout << "─";
	std::cout << '\n';

	/* Print the data */
	for (size_t i = 0; i < data.size(); ++i)
	{
		for (size_t j = 0; j < data.at(i).size() - 1; ++j)
			std::cout << std::setw(column_size.at(j)) << data.at(i).at(j);
		std::cout << std::setw(column_size.at(data.at(i).size() - 1)) << data.at(i).at(data.at(i).size() - 1) << '\n';
	}
}

TEST_CASE("Print a table")
{
	/* Create a table */
	std::vector<std::string> column_names{"Item", "Cost", "Count", "Account"};
	table table(column_names);

	/* Add data to the table */
	table.add_row({"Death rune", "900", "24950", "User 1"});
	table.add_row({"Small crate (historic components)", "50200", "1000", "Account 2"});
	table.add_row({"Perfect juju prayer potion (4)", "1400", "500", "Alt account 3"});
	table.add_row({"Monkfish", "254", "10000", "User 2"});

	/* Print out the table */
	table.print();
}

void table::clear()
{
	data.clear();
}

std::vector<size_t> table::get_column_sizes() const
{
	assert(data.empty() == false);

	std::vector<size_t> column_size(column_names.size());

	/* Reset the table sizes */
	for (size_t i = 0; i < column_size.size(); ++i)
		column_size[i] = 0;

	/* Go through the data linearly and find the longest lines */
	for (size_t i = 0; i < data.size(); ++i)
	{
		for (size_t j = 0; j < data.at(i).size(); ++j)
		{
			if (column_size[j] < data[i][j].size())
				column_size[j] = data[i][j].size();
		}
	}

	/* Also check the column names */
	for (size_t i = 0; i < column_names.size(); ++i)
	{
		if (column_size[i] < column_names[i].size())
			column_size[i] = column_names[i].size();
	}

	/* Pad all of the columns a little bit */
	for (size_t i = 0; i < column_size.size(); ++i)
		column_size[i] += COLUMN_PADDING;

	return column_size;
}
