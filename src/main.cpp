#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "Margin.hpp"
#include "Flips.hpp"
#include "FlipUtils.hpp"

enum class mode
{
	calc, flip, sold, filtering, none
};

/* Function declarations */
bool accepted_modes(const std::vector<mode>& acceptedModes, mode mode);
void print_help();


bool accepted_modes(const std::vector<mode>& acceptedModes, mode mode)
{
	for (size_t i = 0; i < acceptedModes.size(); i++)
		if (acceptedModes[i] == mode)
			return true;

	return false;
}

void print_help()
{
	flip_utils::print_title("Help");
	std::cout <<
	"  calc 	Calculate the margin for an item and possible profits\n" <<
	"\t-b [Insta buy price]\n" <<
	"\t-s [Insta sell price]\n" <<
	"\t-l [Buy limit for the item]\n" <<
	"\n" <<
	"  add 	Add a flip to the database\n" <<
		"\t-i [Item name]\n" <<
		"\t-b [Buying price]\n" <<
		"\t-s [Assumed future selling price]\n" <<
		"\t-l [Buy limit for the item]\n" <<
	"\n" <<
	"  sold  Finish an on-going flip\n" <<
		"\t-i [ID] 		The ID number can be found with the `--list` command\n" <<
		"\t-s [Selling price] 	Optional. This arg is for cases where final sell value changed\n" <<
		"\t-l [Amount sold] 	Optional. This arg is for cases where the full buy limit didn't\n" <<
		"\t\t\t\tbuy or the amount sold was partial.\n" <<
	"\n" <<
	"  filter  Look for items with filters\n" <<
		"\t-i [Item name] 		Find stats for a specific item\n" <<
		"\t-c [Flip count] 	Find flips that have been done count <= times\n" <<
	"\n" <<
	"  cancel [ID]\t\tCancels an on-going flip and removes it from the database\n" <<
	"  list\t\t\tLists all on-going flips with their IDs, buy and sell values\n" <<
	"  list [account]\tLists all on-going flips for the given account with their IDs, buy and sell values\n" <<
	"  stats [--stability] [count]\tPrints out profit statistics. Count sets the length of top-lists\n" <<
	"  repair\t\tAttempts to repair the statistics from the flip data in-case of some bug.\n";
}

int main(int argc, char** argv)
{
	doctest::Context context;

	bool run_tests = false;

	/* No arguments were given */
	if (argc == 1)
	{
		if (!flips::flip_recommendations())
			print_help();
		return 0;
	}

	/* Single arg commands */
	if (argc == 2)
	{
		if (!strcmp(argv[1], "test")) run_tests = true;
		if (!strcmp(argv[1], "stats"))
		{
			flips::print_stats();
			return 0;
		}
		if (!strcmp(argv[1], "list"))
		{
			flips::list();
			return 0;
		}
		if (!strcmp(argv[1], "repair"))
		{
			/* Restore backup */
			//Flips::RestoreBackup();

			/* Attempt to repair issues in the json data */
			flips::fix_stats();
			return 0;
		}
		if (!strcmp(argv[1], "help"))
		{
			print_help();
			return 0;
		}
	}

	/* 2 arg commands */
	if (argc == 3)
	{
		if (!strcmp(argv[1], "cancel"))
		{
			flips::cancel(std::atoi(argv[2]));
			return 0;
		}
		if (!strcmp(argv[1], "list"))
		{
			flips::list(argv[2]);
			return 0;
		}
		if (!strcmp(argv[1], "stats"))
		{
			int result_count = std::atoi(argv[2]);
			if (result_count < 1)
				result_count = 10;

			flips::print_stats(result_count);
		}
	}

	if (run_tests)
	{
		int res = context.run();
		return res;
	}

	/* Go trough all of the arguments */

	mode mode = mode::none;

	/* Calc mode */
	std::string item_name = "";
	std::string account_name = "";
	int buyValue = 0;
	int sellValue = 0;
	int buyLimit = 0;
	int sel_index = -1;

	/* Filtering values */
	int flip_count = 0;

	int processed_args = 1;
	while (processed_args < argc)
	{
		/* Modes */
		if (mode == mode::none)
		{
			if (!strcmp(argv[processed_args], "calc") && argc == 8)
			{
				mode = mode::calc;
				processed_args++;
				continue;
			}

			if (!strcmp(argv[processed_args], "add") && (argc == 10 || argc == 12))
			{
				mode = mode::flip;
				processed_args++;
				continue;
			}

			if (!strcmp(argv[processed_args], "sold") && (argc == 4 || argc == 6 || argc == 8))
			{
				mode = mode::sold;
				processed_args++;
				continue;
			}

			if (!strcmp(argv[processed_args], "filter") && argc == 4)
			{
				mode = mode::filtering;
				processed_args++;
				continue;
			}
		}

		/* Mode sub values */
		if (!strcmp(argv[processed_args], "-b") && accepted_modes({mode::calc, mode::flip}, mode))
		{
			buyValue = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-s") && accepted_modes({mode::calc, mode::flip, mode::sold}, mode))
		{
			sellValue = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-l") && accepted_modes({mode::calc, mode::flip, mode::sold}, mode))
		{
			buyLimit = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-a") && accepted_modes({mode::flip, mode::sold}, mode))
		{
			account_name = argv[processed_args + 1];
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-i") && accepted_modes({mode::flip, mode::sold, mode::filtering}, mode))
		{
			if (mode == mode::flip || mode == mode::filtering)
				item_name = argv[processed_args + 1];
			else
				sel_index = atoi(argv[processed_args + 1]);

			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-c") && accepted_modes({mode::filtering}, mode))
		{
			flip_count = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else
		{
			std::cout << "Invalid argument: " << argv[processed_args] << "! Quitting..." << std::endl;
			return 0;
		}

		std::cout << "Missing args! Quitting..." << std::endl;
		break;
	}

	/* Do the maths */
	switch (mode)
	{
		case (mode::calc):
			margin::print_flip_estimation(buyValue, sellValue, buyLimit);
			break;

		case (mode::flip):
		{
			flips::flip flip(item_name, buyValue, sellValue, buyLimit, account_name);

			std::cout << "Adding item: " << item_name << std::endl;
			std::cout << "Buy price: " << flip_utils::round_big_numbers(buyValue) << std::endl;
			std::cout << "Sell price: " << flip_utils::round_big_numbers(sellValue) << std::endl;
			std::cout << "Buy count: " << buyLimit << "\n\n";

			int profit = margin::calc_profit(flip);
			std::cout << "Estimated profit: " << flip_utils::round_big_numbers(profit) << std::endl;

			flips::add(flip);
			break;
		}

		case (mode::sold):
		{
			if (sel_index < 0)
			{
				std::cout << "No valid item ID was given" << std::endl;
				break;
			}

			flips::sell(sel_index, sellValue, buyLimit);
			break;
		}

		case (mode::filtering):
		{
			/* Filter by name */
			if (item_name != "")
				flips::filter_name(item_name);
			else if (flip_count != 0)
				flips::filter_count(flip_count);

			break;
		}

		case (mode::none):
			break;
	}

	return 0;
}
