#include "AvgStat.hpp"
#include "Stats.hpp"
#include "Flips.hpp"
#include "Margin.hpp"

namespace Stats
{
	constexpr int PROFIT_QUEUE_SIZE = 10;

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

	double AvgStat::RollingAvgProfit() const
	{
		/* Count the total "rolling" profit */
		int rolling_total_profit = 0;
		int rolling_profit_count = 1;

		if (profit_list.size() >= PROFIT_QUEUE_SIZE)
		{
			for (int i = profit_list.size() - PROFIT_QUEUE_SIZE; i < profit_list.size(); ++i)
				rolling_total_profit += profit_list.at(i);

			rolling_profit_count = PROFIT_QUEUE_SIZE;
		}
		else
		{
			for (int i = 0; i < profit_list.size(); ++i)
				rolling_total_profit += profit_list.at(i);

			rolling_profit_count = profit_list.size();
		}

		assert(rolling_profit_count != 0);

		return (double)rolling_total_profit / rolling_profit_count;
	}

	double AvgStat::AvgROI() const
	{
		return (double)total_roi / value_count;
	}

	double AvgStat::AvgBuyLimit() const
	{
		return (double)total_item_count / value_count;
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
		CHECK(statA.AvgProfit() == 100);
		CHECK(statA.AvgROI() == 25);
		CHECK(statA.AvgBuyLimit() == 1000);
		CHECK(statA.FlipCount() == 3);

		AvgStat statB("Item B");
		statB.AddData(100, 25, 500);
		statB.AddData(50, 50, 450);
		statB.AddData(120, 75, 475);

		CHECK(statB.AvgProfit() == 90);
		CHECK(statB.AvgROI() == 50);
		CHECK(statB.AvgBuyLimit() == 475);
		CHECK(statB.FlipCount() == 3);

		AvgStat statC("Item C");
		statC.AddData(10, 1, 1000);
		statC.AddData(100000, 1, 20000);

		CHECK(statC.AvgProfit() == 50005);
		CHECK(statC.AvgBuyLimit() == 10500);
		CHECK(statC.FlipCount() == 2);
	}

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips)
	{
		std::unordered_map<std::string, AvgStat> avg_stats;

		/* Initialize the result list with the first flip */

		/* Find the first item that has sold */
		int i;
		for (i = 0; i < flips.size(); i++)
		{
			if (flips[i]["done"] == true)
				break;
		}

		/* Convert flips into avg stats */
		for (; i < flips.size(); i++)
		{
			/* Ignore items that haven't sold yet */
			if (flips[i]["done"] == false)
				continue;

			int profit = Margin::CalcProfit(flips[i]);

			avg_stats[flips[i]["item"]].name = flips[i]["item"];
			avg_stats[flips[i]["item"]].AddData(profit, Stats::CalcROI(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"]);
		}

		/* Convert the map into a vector */
		std::vector<AvgStat> result;
		std::transform(avg_stats.begin(), avg_stats.end(), std::back_inserter(result), [](const auto& element) { return element.second; });

		return result;
	}
}
