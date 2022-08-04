#include "Utils.hpp"
#include "AvgStat.hpp"
#include "Stats.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "doctest/doctest.h"

namespace Stats
{
	AvgStat::AvgStat()
	:name("null")
	{}

	AvgStat::AvgStat(const std::string& item_name)
	:name(item_name)
	{
		total_profit 		= 0;
		total_roi 			= 0;
		total_item_count 	= 0;
		value_count 		= 0;

		lowest_profit 		= 0;
		highest_profit 		= 0;
		lowest_item_count 	= 0;
		highest_item_count 	= 0;

		total_sell_sold_distance 	= 0;
	}

	void AvgStat::AddData(const int& profit, const double& ROI, const int& item_count, const int& sell, const int& sold)
	{
		total_profit 		+= profit;
		total_roi 			+= ROI;
		total_item_count 	+= item_count;
		value_count++;

		if (lowest_profit == 0 || lowest_profit > profit)
			lowest_profit = profit;

		if (highest_profit == 0 || highest_profit < profit)
			highest_profit = profit;

		if (lowest_item_count == 0 || lowest_item_count > item_count)
			lowest_item_count = item_count;

		if (highest_item_count == 0 || highest_item_count < item_count)
			highest_item_count = item_count;

		total_sell_sold_distance += std::abs(sell - sold);
	}

	double AvgStat::AvgProfit() const
	{
		return (double)total_profit / value_count;
	}

	double AvgStat::AvgROI() const
	{
		return (double)total_roi / value_count;
	}

	double AvgStat::AvgBuyLimit() const
	{
		return (double)total_item_count / value_count;
	}

	double AvgStat::FlipStability() const
	{
		int low_profit = lowest_profit;
		if (lowest_profit == 0)
			low_profit = -1;

		int low_item = lowest_item_count;
		if (lowest_item_count == 0)
			low_item = 1;

		double profit_stability 	= (1 - ((double)low_profit / highest_profit)) * PROFIT_WEIGHT;
		double buy_limit_stability 	= (1 - ((double)low_item / highest_item_count)) * BUYLIMIT_WEIGHT;
		double average_sell_sold_distance = (double)total_sell_sold_distance / value_count;

		if (average_sell_sold_distance == 0)
			average_sell_sold_distance = 1;

		/* Flips that make profit can be unstable, but they are at least
		 * making profit. This modifier will punish flips that have made a loss
		 * at some point and give a boost to flips that at least made some money */
		double profitability_modifier = 1;
		if (lowest_profit < 0)
			profitability_modifier = LOSS_MODIFIER;
		else
			profitability_modifier = PROFIT_MODIFIER;

		return (profit_stability + buy_limit_stability) * (std::pow(FLIP_COUNT_MULTIPLIER, value_count)) * average_sell_sold_distance * profitability_modifier * 100;
	}

	TEST_CASE("Flip stability stress test")
	{
		bool success = true;

		for (int i = 0; i < 100; i++)
		{
			AvgStat stat("Item");
			stat.AddData(100, 25, 500);
			stat.AddData(50, 50, 450);
			stat.AddData(120, 75, 475);

			if (stat.FlipStability() > 100)
			{
				success = false;
				break;
			}
		}

		CHECK(success);
	}

	int AvgStat::FlipCount() const
	{
		return value_count;
	}

	TEST_CASE("Average stats per item")
	{
		AvgStat statA("Item A");
		statA.AddData(100, 25, 1000);
		statA.AddData(100, 25, 1000);
		statA.AddData(100, 25, 1000);
		CHECK(statA.FlipStability() == 0);
		CHECK(statA.AvgProfit() == 100);
		CHECK(statA.AvgROI() == 25);
		CHECK(statA.AvgBuyLimit() == 1000);
		CHECK(statA.FlipCount() == 3);

		AvgStat statB("Item B");
		statB.AddData(100, 25, 500);
		statB.AddData(50, 50, 450);
		statB.AddData(120, 75, 475);

		CHECK(statB.FlipStability() != 0);
		CHECK(statB.AvgProfit() == 90);
		CHECK(statB.AvgROI() == 50);
		CHECK(statB.AvgBuyLimit() == 475);
		CHECK(statB.FlipCount() == 3);

		AvgStat statC("Item C");
		statC.AddData(10, 1, 1000);
		statC.AddData(100000, 1, 20000);

		CHECK(statC.FlipStability() != 0);
		CHECK(statC.AvgProfit() == 50005);
		CHECK(statC.AvgBuyLimit() == 10500);
		CHECK(statC.FlipCount() == 2);
	}

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips)
	{
		std::vector<AvgStat> result;

		/* Initialize the result list with the first flip */

		/* Find the first item that has sold */
		int i;
		for (i = 0; i < flips.size(); i++)
		{
			if (flips[i]["done"] == true)
				break;
		}
		//result.push_back(AvgStat(flips[i]["item"]));

		/* Convert flips into avg stats */
		for (; i < flips.size(); i++)
		{
			/* Ignore items that haven't sold yet */
			if (flips[i]["done"] == false)
				continue;

			bool valueFound = false;
			for (int j = 0; j < result.size(); j++)
			{
				/* Check if the flip is already in the avg stat array */
				if (flips[i]["item"] == result[j].name)
				{
					result[j].AddData(Margin::CalcProfit(flips[i]), Stats::CalcROI(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"]);
					valueFound = true;
					break;
				}
			}

			if (!valueFound)
			{
				/* Add a new item and the values for it */
				result.push_back(AvgStat(flips[i]["item"]));
				result[result.size() - 1].AddData(Margin::CalcProfit(flips[i]), Stats::CalcROI(flips[i]), flips[i]["limit"]);
			}
		}

		return result;
	}
}
