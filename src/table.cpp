#include "Table.hpp"
#include "pch.hpp"

constexpr int COLUMN_PADDING = 4;

Table::Table(std::vector<std::string> column_names)
:column_names(column_names)
{
	assert(column_names.size() > 1);
}

void Table::add_row(std::vector<std::string> data)
{
	assert(data.size() == column_names.size());
	this->data.push_back(data);
}

void Table::print() const
{
	assert(data.size() > 0);

	/* Get the column sizes */
	std::vector<size_t> column_size = get_column_sizes();

	assert(column_size.size() == column_names.size());
	assert(column_size.size() == data.at(0).size());

	/* Print the column names */
	std::cout << std::left;
	for (int i = 0; i < column_names.size() - 1; ++i)
		std::cout << std::setw(column_size.at(i) - i) << column_names.at(i);
	std::cout << column_names.at(column_names.size() - 1) << std::left << "\n";

	/* Print a divider */
	int divider_size = 0;
	for (int i = 0; i < column_size.size(); ++i)
		divider_size += column_size.at(i);

	std::string divider(divider_size - (COLUMN_PADDING * (column_names.size() - 2)) + 1, '-');
	std::cout << divider << '\n';


	/* Print the data */
	std::cout << std::left;
	for (int i = 0; i < data.size(); ++i)
	{
		for (int j = 0; j < data.at(i).size() - 1; ++j)
			std::cout << std::setw(column_size.at(j) - j) << data.at(i).at(j);
		std::cout << std::left << std::setw(column_size.at(data.at(i).size() - 1)) << data.at(i).at(data.at(i).size() - 1) << std::left;

		std::cout << "\n";
	}
}

TEST_CASE("Print a table")
{
	/* Create a table */
	std::vector<std::string> column_names{"Item", "Cost", "Count", "Account"};
	Table table(column_names);

	/* Add data to the table */
	table.add_row({"Death rune", "900", "24950", "User 1"});
	table.add_row({"Small crate (historic components)", "50200", "1000", "Account 2"});
	table.add_row({"Perfect juju prayer potion (4)", "1400", "500", "Alt account 3"});
	table.add_row({"Monkfish", "254", "10000", "User 2"});

	/* Print out the table */
	table.print();
}

std::vector<size_t> Table::get_column_sizes() const
{
	assert(data.empty() == false);

	std::vector<size_t> column_size(column_names.size());

	/* Reset the table sizes */
	for (int i = 0; i < column_size.size(); ++i)
		column_size[i] = 0;

	/* Go through the data linearly and find the longest lines */
	for (int i = 0; i < data.size(); ++i)
	{
		for (int j = 0; j < data.at(i).size(); ++j)
		{
			if (column_size[j] < data[i][j].size())
				column_size[j] = data[i][j].size();
		}
	}

	/* Also check the column names */
	for (int i = 0; i < column_names.size(); ++i)
		if (column_size[i] < column_names[i].size())
			column_size[i] = column_names[i].size();

	/* Pad all of the columns a little bit */
	for (int i = 0; i < column_size.size(); ++i)
		column_size[i] += COLUMN_PADDING;

	return column_size;
}
