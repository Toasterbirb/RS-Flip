#pragma once
#include "pch.hpp"

#define PROFIT_FILTER 500000
#define BAD_PROFIT_MODIFIER 100

namespace Stats
{
	class AvgStat
	{
	public:
		AvgStat();
		AvgStat(const std::string& item_name);
		void AddData(const int profit, const double ROI, const int item_count, const int sell = 0, const int sold = 0, const int latest_trade_index = 0);
		double AvgProfit() const;
		double RollingAvgProfit() const; /* Get the avg. profit of the latest flips */
		double AvgROI() const;
		double AvgBuyLimit() const;
		double FlipRecommendation() const;
		int FlipCount() const;
		int LatestTradeIndex() const;

		std::string name;

	private:
		std::vector<int> profit_list;

		int total_profit = 0;
		double total_roi = 0;
		int total_item_count = 0;
		int value_count = 0;
		int latest_trade_index = 0;

		int lowest_item_count = 0;
		int highest_item_count = 0;

		int total_sell_sold_distance = 0;

		/// The total amount (count) of flip data added to avgstats
		static inline int total_flip_count = 0;
	};

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips);
}
