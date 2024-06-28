#pragma once

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
		avg_stat(const std::string& item_name);
		void add_data(const int profit, const double ROI, const int item_count, const int sell = 0, const int sold = 0, const int latest_trade_index = 0);
		double avg_profit() const;
		double rolling_avg_profit() const; /* Get the avg. profit of the latest flips */
		double avg_roi() const;
		double avg_buy_limit() const;
		double flip_recommendation() const;
		int flip_count() const;
		int latest_trade_index() const;

		std::string name;

	private:
		std::vector<int> profit_list;

		int total_profit = 0;
		double total_roi = 0;
		int total_item_count = 0;
		int value_count = 0;
		int _latest_trade_index = 0;

		int lowest_item_count = 0;
		int highest_item_count = 0;

		int total_sell_sold_distance = 0;

		/// The total amount (count) of flip data added to avgstats
		static inline int total_flip_count = 0;
	};

	std::vector<avg_stat> flips_to_avg_stats(const std::vector<nlohmann::json>& flips);
}
