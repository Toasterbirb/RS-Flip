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
		total_profit 	= 0;
		total_roi 		= 0;
		value_count 	= 0;
	}

	void AvgStat::AddData(const int& profit, const double& ROI)
	{
		total_profit += profit;
		total_roi += ROI;
		value_count++;
	}

	double AvgStat::AvgProfit() const
	{
		return (double)total_profit / value_count;
	}

	double AvgStat::AvgROI() const
	{
		return (double)total_roi / value_count;
	}

	TEST_CASE("Average stats per item")
	{
		AvgStat statA("Item A");
		statA.AddData(100, 25);
		statA.AddData(100, 25);
		statA.AddData(100, 25);
		CHECK(statA.AvgProfit() == 100);
		CHECK(statA.AvgROI() == 25);

		AvgStat statB("Item B");
		statB.AddData(100, 25);
		statB.AddData(50, 50);
		statB.AddData(120, 75);
		CHECK(statB.AvgProfit() == 90);
		CHECK(statB.AvgROI() == 50);

		AvgStat statC("Item C");
		statC.AddData(10, 1);
		statC.AddData(100000, 1);
		CHECK(statC.AvgProfit() == 50005);
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
					result[j].AddData(Margin::CalcProfit(flips[i]), Stats::CalcROI(flips[i]));
					valueFound = true;
					break;
				}
			}

			if (!valueFound)
			{
				/* Add a new item and the values for it */
				result.push_back(AvgStat(flips[i]["item"]));
				result[result.size() - 1].AddData(Margin::CalcProfit(flips[i]), Stats::CalcROI(flips[i]));
			}
		}

		return result;
	}
}
