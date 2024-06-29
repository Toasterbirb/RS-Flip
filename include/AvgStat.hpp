#pragma once

#include "Types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

constexpr int PROFIT_FILTER = 500000;
constexpr int BAD_PROFIT_MODIFIER = 100;

namespace stats
{
	class avg_stat
	{
	public:
		avg_stat();
		explicit avg_stat(const std::string& item_name);
		void add_data(const i64 profit, const f64 ROI, const u32 item_count, const i32 sell = 0, const i32 sold = 0, const u32 latest_trade_index = 0);
		f64 avg_profit() const;
		f64 rolling_avg_profit() const; /* Get the avg. profit of the latest flips */
		f64 avg_roi() const;
		f64 avg_buy_limit() const;
		f64 flip_recommendation() const;
		u32 flip_count() const;
		i32 latest_trade_index() const;

		std::string name;

	private:
		std::vector<int> profit_list;

		i64 total_profit = 0;
		f64 total_roi = 0;
		i32 total_item_count = 0;
		u32 value_count = 0;
		u32 _latest_trade_index = 0;

		u32 lowest_item_count = 0;
		u32 highest_item_count = 0;

		i32 total_sell_sold_distance = 0;

		/// The total amount (count) of flip data added to avgstats
		static inline u32 total_flip_count = 0;
	};

	std::vector<avg_stat> flips_to_avg_stats(const std::vector<nlohmann::json>& flips);
}
