#include "AvgStat.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Stats.hpp"

#include <doctest/doctest.h>

namespace stats
{
	constexpr i32 PROFIT_QUEUE_SIZE = 10;

	avg_stat::avg_stat()
	:name("null")
	{}

	avg_stat::avg_stat(const std::string& item_name)
	:name(item_name)
	{
		total_profit 		= 0;
		total_roi 			= 0;
		total_item_count 	= 0;
	}

	void avg_stat::add_data(const i64 profit, const f64 ROI, const u32 item_count, const u32 latest_trade_index)
	{
		profit_list.push_back(profit);

		total_profit 		+= profit;
		total_roi 			+= ROI;
		total_item_count 	+= item_count;

		if (this->_latest_trade_index < latest_trade_index)
		{
			this->_latest_trade_index = latest_trade_index;

			// Attempt to keep the total flip amount up-to-date
			if (latest_trade_index > total_flip_count)
				total_flip_count = latest_trade_index;
		}
	}

	f64 avg_stat::avg_profit() const
	{
		return flip_count() == 0 ? 0 : total_profit / static_cast<double>(flip_count());
	}

	f64 avg_stat::rolling_avg_profit() const
	{
		if (profit_list.empty())
			return 0;

		/* Count the total "rolling" profit */

		const bool flip_list_longer_than_profit_queue = profit_list.size() >= PROFIT_QUEUE_SIZE;
		const size_t first_flip = flip_list_longer_than_profit_queue ? profit_list.size() - PROFIT_QUEUE_SIZE : 0;
		const u32 rolling_profit_count = flip_list_longer_than_profit_queue ? PROFIT_QUEUE_SIZE : profit_list.size();

		const i64 rolling_total_profit = std::accumulate(profit_list.begin() + first_flip, profit_list.end(), 0);

		assert(rolling_profit_count != 0 && "Zero division");
		return rolling_total_profit / static_cast<double>(rolling_profit_count);
	}

	TEST_CASE("Rolling average profit")
	{
		SUBCASE("Granite (5kg)")
		{
			avg_stat granite("Granite (5kg)");
			const f64 roi = calc_roi(8614, 8613);
			constexpr i64 profit = -509556;

			CHECK(roi < 0);

			granite.add_data(profit, roi, 2941);
			CHECK(granite.flip_count() == 1);
			CHECK(granite.avg_roi() == roi);
			CHECK(granite.avg_profit() == profit);
			CHECK(granite.rolling_avg_profit() == profit);
		}
	}

	f64 avg_stat::avg_roi() const
	{
		return flip_count() == 0 ? 0 : total_roi / static_cast<double>(flip_count());
	}

	f64 avg_stat::avg_buy_limit() const
	{
		return flip_count() == 0 ? 0 : total_item_count / static_cast<double>(flip_count());
	}

	f64 avg_stat::flip_recommendation() const
	{
		if (flip_count() == 0)
			return 0;

		assert(total_flip_count > 0);

		constexpr f64 flip_age_penaly = 0.005; // Higher value lowers the score more for stale flips
		constexpr f64 flip_index_age_exponent = 0.9; // Increase the impact of flip age
		f64 flip_age_debuff = 1.0 - (flip_age_penaly * std::pow(total_flip_count - _latest_trade_index, flip_index_age_exponent));

		// Set limits to the age penalty
		constexpr f64 min_penalty = 0.001;
		constexpr f64 max_penalty = 1.0;
		flip_age_debuff = std::clamp(flip_age_debuff, min_penalty, max_penalty);

		const f64 roi_modifier = flip_utils::limes(2, 1.5, 1, avg_roi());
		const f64 flip_count_modifier = flip_utils::limes(2, 1, 3, flip_count());

		constexpr f32 profit_exponent = 1.50f;

		constexpr f32 inverse_divisor = 1.0 / 10000.0;
		return std::round((std::pow(rolling_avg_profit(), profit_exponent) * flip_age_debuff * roi_modifier * flip_count_modifier) * inverse_divisor);
	}

	u32 avg_stat::flip_count() const
	{
		return profit_list.size();
	}

	i32 avg_stat::latest_trade_index() const
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

		/* Convert flips into avg stats */
		for (size_t i = 0; i < flips.size(); i++)
		{
			const flips::flip flip(flips[i]);

			/* Ignore items that haven't sold yet */
			if (flip.done == false)
				continue;

			/* Ignore items that have been cancelled */
			if (flip.cancelled == true)
				continue;

			avg_stat& stat = avg_stats[flip.item];
			stat.name = flip.item;
			stat.add_data(
					margin::calc_profit(flip),
					stats::calc_roi(flip.buy_price, flip.sold_price),
					flip.buylimit,
					i
				);

			assert(stat.name.empty() == false);
			assert(stat.flip_count() > 0);
			assert(stat.avg_buy_limit() > 0);

			/* Highly doubt someone is going to flip the same item
			 * more than 10 000 000 times */
			assert(stat.flip_count() < 10'000'000);
		}

		/* Convert the map into a vector */
		std::vector<avg_stat> result;
		result.reserve(avg_stats.size());
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
