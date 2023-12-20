#include "FlipUtils.hpp"
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
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

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
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

		return (double)total_roi / value_count;
	}

	double AvgStat::AvgBuyLimit() const
	{
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

		return (double)total_item_count / value_count;
	}

	double AvgStat::FlipRecommendation() const
	{
		if (FlipCount() > 0)
			return std::round((RollingAvgProfit() * FlipUtils::Limes(2, 1.5, 1, AvgROI()) *  FlipUtils::Limes(1.1, 1, 1, FlipCount())) / 10000.0f);
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
			/* Asserts for checking if the json object has all of the required keys */
			assert(flips[i].contains("item"));
			assert(flips[i].contains("buy"));
			assert(flips[i].contains("limit"));
			assert(flips[i].contains("sell"));
			assert(flips[i].contains("sold"));
			assert(flips[i].contains("cancelled"));
			assert(flips[i].contains("done"));

			/* Ignore items that haven't sold yet */
			if (flips[i]["done"] == false)
				continue;

			int profit = Margin::CalcProfit(flips[i]);

			AvgStat& stat = avg_stats[flips[i]["item"]];
			stat.name = flips[i]["item"];
			stat.AddData(profit, Stats::CalcROI(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"]);

			assert(stat.name.empty() == false);
			assert(stat.FlipCount() > 0);
			assert(stat.AvgBuyLimit() > 0);

			/* Highly doubt someone is going to flip the same item
			 * more than 10 000 000 times */
			std::cout << "Flip: " << stat.FlipCount() << std::endl;
			assert(stat.FlipCount() < 10000000);
		}

		/* Convert the map into a vector */
		std::vector<AvgStat> result;
		std::transform(avg_stats.begin(), avg_stats.end(), std::back_inserter(result), [](const auto& element) { return element.second; });

		return result;
	}

	TEST_CASE("Convert flips to avgstats")
	{
		std::vector<nlohmann::json> json;

		nlohmann::json data_point_A;
		data_point_A["item"] = "Test item";
		data_point_A["limit"] = 5000;
		data_point_A["done"] = true;
		data_point_A["buy"] = 1000;
		data_point_A["sell"] = 1500;
		data_point_A["sold"] = 1400;
		data_point_A["cancelled"] = false;
		json.push_back(data_point_A);

		nlohmann::json data_point_B;
		data_point_B["item"] = "Another test item";
		data_point_B["limit"] = 2000;
		data_point_B["done"] = false;
		data_point_B["buy"] = 500;
		data_point_B["sell"] = 1000;
		data_point_B["sold"] = 900;
		data_point_B["cancelled"] = false;
		json.push_back(data_point_B);

		nlohmann::json data_point_C;
		data_point_C["item"] = "Test item";
		data_point_C["limit"] = 2000;
		data_point_C["done"] = true;
		data_point_C["buy"] = 500;
		data_point_C["sell"] = 1000;
		data_point_C["sold"] = 900;
		data_point_C["cancelled"] = false;
		json.push_back(data_point_C);

		std::vector<AvgStat> avg_stats = FlipsToAvgstats(json);

		CHECK(avg_stats.size() == 1);
		CHECK(avg_stats[0].name == "Test item");
		CHECK(avg_stats[0].FlipCount() == 2);
	}
}
