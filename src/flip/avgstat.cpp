#include "FlipUtils.hpp"
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

		lowest_item_count 	= 0;
		highest_item_count 	= 0;

		total_sell_sold_distance 	= 0;
	}

	void AvgStat::AddData(const int& profit, const double& ROI, const int& item_count, const int& sell, const int& sold)
	{
		profit_list.push_back(profit);

		total_profit 		+= profit;
		total_roi 			+= ROI;
		total_item_count 	+= item_count;
		value_count++;

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
		/* Sort the profit list. Highest value first */
		std::vector<int> sorted_profits = profit_list;

		int placeholder = 0;
		bool swapped;
		do
		{
			swapped = false;
			for (size_t i = 0; i < profit_list.size() - 1; i++)
			{
				if (sorted_profits[i] < sorted_profits[i + 1])
				{
					placeholder = sorted_profits[i];
					sorted_profits[i] = sorted_profits[i + 1];
					sorted_profits[i + 1] = placeholder;
					swapped = true;
				}
			}
		} while (swapped);

		/** Do some analysis on the profit list **/
		int lowest_profit = sorted_profits[profit_list.size() - 1];
		if (lowest_profit == 0)
			lowest_profit = -1;

		int highest_profit = sorted_profits[0];

		/* Find the first flip that makes less than the average profit
		 * and then calculate its percentile */
		int underperforming_index = 0;
		float avg_profit = AvgProfit();
		for (size_t i = 0; i < sorted_profits.size(); i++)
		{
			if (sorted_profits[i] < avg_profit)
			{
				underperforming_index = i;
				break;
			}
		}
		float underperforming_percentile = 1 - ((float)underperforming_index / sorted_profits.size());

		/* Got zero from the underperforming_percentile, the flip is probably awesome
		 * or there's not enough data yet. Let's say that its somewhat awesome */
		if (underperforming_percentile == 0)
			underperforming_percentile = 0.4f;

		float bad_profit_multiplier = 1;
		if (avg_profit < PROFIT_FILTER)
			bad_profit_multiplier = BAD_PROFIT_MODIFIER;


		int low_item = lowest_item_count;
		if (lowest_item_count == 0)
			low_item = 1;

		double profit_stability 	= (1 - ((double)lowest_profit / avg_profit)) * PROFIT_WEIGHT;
		double buy_limit_stability 	= (1 - ((double)low_item / ((double)total_item_count / value_count))) * BUYLIMIT_WEIGHT;
		double average_sell_sold_distance = (double)total_sell_sold_distance / value_count;

		/* Cap sell_sold distance at 10 */
		if (average_sell_sold_distance > 10)
			average_sell_sold_distance = 10;

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

		return (profit_stability + buy_limit_stability) * (std::pow(FLIP_COUNT_MULTIPLIER, value_count)) * average_sell_sold_distance * profitability_modifier * underperforming_percentile * bad_profit_multiplier * 100;
	}

	double AvgStat::FlipRecommendation() const
	{
		if (FlipCount() > 0)
			return std::round((AvgProfit() * AvgROI() * (1 - 1.0f / FlipCount()) + 0.1) / 10000.0f);
		else
			return 0;
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
					int profit = Margin::CalcProfit(flips[i]);
					result[j].AddData(profit, Stats::CalcROI(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"]);
					valueFound = true;
					break;
				}
			}

			if (!valueFound)
			{
				/* Add a new item and the values for it */
				result.push_back(AvgStat(flips[i]["item"]));
				result[result.size() - 1].AddData(Margin::CalcProfit(flips[i]), Stats::CalcROI(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"]);
			}
		}

		return result;
	}

	TEST_CASE("Convert flips to average stats")
	{
		std::vector<nlohmann::json> test_flips;
		std::vector<AvgStat> avgStats;

		Flips::Flip flipA;
		flipA.buy_price 	= 268;
		flipA.cancelled 	= false;
		flipA.done 			= true;
		flipA.item 			= "Yew logs";
		flipA.buylimit 		= 24999;
		flipA.sell_price 	= 294;
		flipA.sold_price 	= 294;
		test_flips.push_back(flipA.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(avgStats[0].AvgProfit() == 649974);

		Flips::Flip flipB;
		flipB.buy_price 	= 271;
		flipB.cancelled 	= false;
		flipB.done 			= true;
		flipB.item 			= "Yew logs";
		flipB.buylimit 		= 23806;
		flipB.sell_price 	= 294;
		flipB.sold_price 	= 294;
		test_flips.push_back(flipB.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(avgStats[0].AvgProfit() == 598756);

		Flips::Flip flipC;
		flipC.buy_price 	= 281;
		flipC.cancelled 	= false;
		flipC.done 			= true;
		flipC.item 			= "Yew logs";
		flipC.buylimit 		= 22332;
		flipC.sell_price 	= 299;
		flipC.sold_price 	= 299;
		test_flips.push_back(flipC.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(std::round(avgStats[0].AvgProfit()) == std::round(533162.6667));

		Flips::Flip flipD;
		flipD.buy_price 	= 284;
		flipD.cancelled 	= false;
		flipD.done 			= true;
		flipD.item 			= "Yew logs";
		flipD.buylimit 		= 24999;
		flipD.sell_price 	= 305;
		flipD.sold_price 	= 305;
		test_flips.push_back(flipD.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(std::round(avgStats[0].AvgProfit()) == std::round(531116.75));

		Flips::Flip flipE;
		flipE.buy_price 	= 277;
		flipE.cancelled 	= false;
		flipE.done 			= true;
		flipE.item 			= "Yew logs";
		flipE.buylimit 		= 24999;
		flipE.sell_price 	= 279;
		flipE.sold_price 	= 276;
		test_flips.push_back(flipE.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(std::round(avgStats[0].AvgProfit()) == std::round(419893.6));

		Flips::Flip flipF;
		flipF.buy_price 	= 273;
		flipF.cancelled 	= false;
		flipF.done 			= true;
		flipF.item 			= "Yew logs";
		flipF.buylimit 		= 24999;
		flipF.sell_price 	= 275;
		flipF.sold_price 	= 275;
		test_flips.push_back(flipF.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(std::round(avgStats[0].AvgProfit()) == std::round(358244.3333));

		Flips::Flip flipG;
		flipG.buy_price 	= 268;
		flipG.cancelled 	= false;
		flipG.done 			= true;
		flipG.item 			= "Yew logs";
		flipG.buylimit 		= 24999;
		flipG.sell_price 	= 271;
		flipG.sold_price 	= 271;
		test_flips.push_back(flipG.ToJson());

		avgStats = FlipsToAvgstats(test_flips);
		CHECK(avgStats.size() == 1);
		CHECK(avgStats[0].name == "Yew logs");
		CHECK(avgStats[0].FlipCount() == 7);
		CHECK(std::round(avgStats[0].AvgProfit()) == std::round(317780.4286));
	}
}