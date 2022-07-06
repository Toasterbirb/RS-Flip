#define DOCTEST_CONFIG_IMPLEMENT
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include "doctest/doctest.h"
#include "Margin.hpp"
#include "Flips.hpp"

enum Mode
{
	Calc, Flip, Sold, None
};

bool AcceptedModes(std::vector<Mode> acceptedModes, Mode mode)
{
	for (int i = 0; i < acceptedModes.size(); i++)
		if (acceptedModes[i] == mode)
			return true;

	return false;
}

int main(int argc, char** argv)
{
	doctest::Context context;

	bool run_tests = false;

	if (argc == 2)
	{
		if (!strcmp(argv[1], "--test")) run_tests = true;
		if (!strcmp(argv[1], "--stats"))
		{
			Flips::PrintStats();
			return 0;
		}
		if (!strcmp(argv[1], "--list"))
		{
			Flips::Init();
			Flips::List();
			return 0;
		}
	}

	if (run_tests)
	{
		int res = context.run();
		return res;
	}

	/* Go trough all of the arguments */

	Mode mode = Mode::None;

	/* Calc mode */
	std::string item_name;
	int buyValue = 0;
	int sellValue = 0;
	int buyLimit = 0;
	int sel_index;

	int processed_args = 1;
	while (processed_args < argc)
	{
		/* Modes */
		if (mode == Mode::None)
		{
			if (!strcmp(argv[processed_args], "--calc") && argc == 8)
			{
				mode = Mode::Calc;
				processed_args++;
				continue;
			}

			if (!strcmp(argv[processed_args], "--flip") && argc == 10)
			{
				mode = Mode::Flip;
				processed_args++;
				continue;
			}

			if (!strcmp(argv[processed_args], "--sold") && (argc == 6 || argc == 8))
			{
				mode = Mode::Sold;
				processed_args++;
				continue;
			}
		}

		/* Mode sub values */
		if (!strcmp(argv[processed_args], "-b") && AcceptedModes({Calc, Flip}, mode))
		{
			buyValue = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-s") && AcceptedModes({Calc, Flip, Sold}, mode))
		{
			sellValue = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-l") && AcceptedModes({Calc, Flip, Sold}, mode))
		{
			buyLimit = atoi(argv[processed_args + 1]);
			processed_args += 2;
			continue;
		}
		else if (!strcmp(argv[processed_args], "-i") && AcceptedModes({Flip, Sold}, mode))
		{
			if (mode == Mode::Flip)
				item_name = argv[processed_args + 1];
			else
				sel_index = atoi(argv[processed_args + 1]);

			processed_args += 2;
			continue;
		}
		else
		{
			std::cout << "Invalid argument! Quitting..." << std::endl;
			return 0;
		}

		std::cout << "Missing args! Quitting..." << std::endl;
		break;
	}

	/* Do the maths */
	switch (mode)
	{
		case (Mode::Calc):
			Margin::PrintFlipEstimation(buyValue, sellValue, buyLimit);
			break;

		case (Mode::Flip):
		{
			Flips::Flip flip(item_name, buyValue, sellValue);

			std::cout << "Adding item: " << item_name << std::endl;
			std::cout << "Buy price: " << buyValue << std::endl;
			std::cout << "Sell price: " << sellValue << std::endl;

			Flips::Add(flip);
			break;
		}

		case (Mode::Sold):
		{
			Flips::Sell(sel_index, sellValue, buyLimit);
			break;
		}

		case (Mode::None):
			break;
	}

	return 0;
}
