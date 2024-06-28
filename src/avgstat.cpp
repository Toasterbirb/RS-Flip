#include "AvgStat.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Stats.hpp"

#include <doctest/doctest.h>

namespace stats
{
	constexpr int PROFIT_QUEUE_SIZE = 10;

	avg_stat::avg_stat()
	:name("null")
	{}

	avg_stat::avg_stat(const std::string& item_name)
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

	void avg_stat::add_data(const int profit, const double ROI, const int item_count, const int sell, const int sold, const int latest_trade_index)
	{
		profit_list.push_back(profit);

		total_profit 		+= profit;
		total_roi 			+= ROI;
		total_item_count 	+= item_count;
		value_count++;

		if (this->_latest_trade_index < latest_trade_index)
		{
			this->_latest_trade_index = latest_trade_index;

			// Attempt to keep the total flip amount up-to-date
			if (latest_trade_index > total_flip_count)
				total_flip_count = latest_trade_index;
		}

		if (lowest_item_count == 0 || lowest_item_count > item_count)
			lowest_item_count = item_count;

		if (highest_item_count == 0 || highest_item_count < item_count)
			highest_item_count = item_count;

		total_sell_sold_distance += std::abs(sell - sold);
	}

	double avg_stat::avg_profit() const
	{
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

		return (double)total_profit / value_count;
	}

	double avg_stat::rolling_avg_profit() const
	{
		/* Count the total "rolling" profit */
		int rolling_total_profit = 0;
		int rolling_profit_count = 1;

		if (profit_list.size() >= PROFIT_QUEUE_SIZE)
		{
			for (size_t i = profit_list.size() - PROFIT_QUEUE_SIZE; i < profit_list.size(); ++i)
				rolling_total_profit += profit_list.at(i);

			rolling_profit_count = PROFIT_QUEUE_SIZE;
		}
		else
		{
			for (size_t i = 0; i < profit_list.size(); ++i)
				rolling_total_profit += profit_list.at(i);

			rolling_profit_count = profit_list.size();
		}

		assert(rolling_profit_count != 0);

		return (double)rolling_total_profit / rolling_profit_count;
	}

	double avg_stat::avg_roi() const
	{
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

		return (double)total_roi / value_count;
	}

	double avg_stat::avg_buy_limit() const
	{
		assert(value_count > 0);

		/* Avoid division by zero */
		if (value_count == 0)
			return 0;

		return (double)total_item_count / value_count;
	}

	double avg_stat::flip_recommendation() const
	{
		if (flip_count() > 0)
		{
			assert(total_flip_count > 0);
			assert(total_flip_count - _latest_trade_index >= 0);

			constexpr double flip_age_penaly = 0.005; // Higher value lowers the score more for stale flips
			constexpr double flip_index_age_exponent = 1.1; // Increase the impact of flip age
			double flip_age_debuff = 1.0 - (flip_age_penaly * std::pow(total_flip_count - _latest_trade_index, flip_index_age_exponent));

			// Set limits to the age penalty
			constexpr double min_penalty = 0.001;
			constexpr double max_penalty = 1.0;
			flip_age_debuff = std::clamp(flip_age_debuff, min_penalty, max_penalty);

			return std::round((flip_age_debuff * rolling_avg_profit() * flip_utils::limes(2, 1.5, 1, avg_roi()) *  flip_utils::limes(1.1, 1, 1, flip_count())) / 10000.0);
		}
		else
		{
			return 0;
		}
	}

	int avg_stat::flip_count() const
	{
		return value_count;
	}

	int avg_stat::latest_trade_index() const
	{
		return _latest_trade_index;
	}

	TEST_CASE("Average stats per item")
	{
		avg_stat statA("Item A");
		statA.add_data(100, 25, 1000);
		statA.add_data(100, 25, 1000);
		statA.add_data(100, 25, 1000);
		CHECK(statA.avg_profit() == 100);
		CHECK(statA.avg_roi() == 25);
		CHECK(statA.avg_buy_limit() == 1000);
		CHECK(statA.flip_count() == 3);

		avg_stat statB("Item B");
		statB.add_data(100, 25, 500);
		statB.add_data(50, 50, 450);
		statB.add_data(120, 75, 475);

		CHECK(statB.avg_profit() == 90);
		CHECK(statB.avg_roi() == 50);
		CHECK(statB.avg_buy_limit() == 475);
		CHECK(statB.flip_count() == 3);

		avg_stat statC("Item C");
		statC.add_data(10, 1, 1000);
		statC.add_data(100000, 1, 20000);

		CHECK(statC.avg_profit() == 50005);
		CHECK(statC.avg_buy_limit() == 10500);
		CHECK(statC.flip_count() == 2);
	}

	std::vector<avg_stat> flips_to_avg_stats(const std::vector<nlohmann::json>& flips)
	{
		std::unordered_map<std::string, avg_stat> avg_stats;

		/* Initialize the result list with the first flip */

		/* Find the first item that has sold */
		size_t i;
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

			const int profit = margin::calc_profit(flips[i]);

			avg_stat& stat = avg_stats[flips[i]["item"]];
			stat.name = flips[i]["item"];
			stat.add_data(profit, stats::calc_roi(flips[i]), flips[i]["limit"], flips[i]["sell"], flips[i]["sold"], i);

			assert(stat.name.empty() == false);
			assert(stat.flip_count() > 0);
			assert(stat.avg_buy_limit() > 0);

			/* Highly doubt someone is going to flip the same item
			 * more than 10 000 000 times */
			assert(stat.flip_count() < 10000000);
		}

		/* Convert the map into a vector */
		std::vector<avg_stat> result;
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

		std::vector<avg_stat> avg_stats = flips_to_avg_stats(json);

		CHECK(avg_stats.size() == 1);
		CHECK(avg_stats[0].name == "Test item");
		CHECK(avg_stats[0].flip_count() == 2);
	}
}
