#pragma once

#include "Types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace stats
{
	enum class recommendation_algorithm
	{
		v1 = 0,
		v2 = 1
	};

	class avg_stat
	{
	public:
		avg_stat();
		explicit avg_stat(const std::string& item_name);
		void add_data(const i64 profit, const f64 ROI, const u32 item_count, const u32 latest_trade_index = 0);
		f64 avg_profit() const;
		f64 rolling_avg_profit() const; /* Get the avg. profit of the latest flips */
		f64 avg_roi() const;
		f64 avg_buy_limit() const;
		f64 flip_recommendation() const;
		u32 flip_count() const;
		i32 latest_trade_index() const;

		// how many flips have been done in total
		static u32 total_flip_count();

		static void set_recommendation_algorithm(const recommendation_algorithm& algorithm);

		std::string name;

	private:
		std::vector<int> profit_list;

		i64 total_profit = 0;
		f64 total_roi = 0;
		i32 total_item_count = 0;
		u32 _latest_trade_index = 0;

		/// The total amount (count) of flip data added to avgstats
		static inline u32 _total_flip_count = 0;
	};

	std::vector<avg_stat> flips_to_avg_stats(const std::vector<nlohmann::json>& flips);
}
