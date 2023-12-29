#pragma once
#include "pch.hpp"

/* Stability algo parameters */
#define PROFIT_WEIGHT 0.8
#define BUYLIMIT_WEIGHT 0.2
#define FLIP_COUNT_MULTIPLIER 0.95
#define LOSS_MODIFIER 2
#define PROFIT_MODIFIER 0.85

#define PROFIT_FILTER 500000
#define BAD_PROFIT_MODIFIER 100

namespace Stats
{
	class AvgStat
	{
	public:
		AvgStat();
		AvgStat(const std::string& item_name);
		void AddData(const int profit, const double ROI, const int item_count, const int sell = 0, const int sold = 0);
		double AvgProfit() const;
		double RollingAvgProfit() const; /* Get the avg. profit of the latest flips */
		double AvgROI() const;
		double AvgBuyLimit() const;
		double FlipRecommendation() const;
		int FlipCount() const;

		std::string name;

	private:
		std::vector<int> profit_list;

		int total_profit = 0;
		double total_roi = 0;
		int total_item_count = 0;
		int value_count = 0;

		int lowest_item_count = 0;
		int highest_item_count = 0;

		int total_sell_sold_distance = 0;
	};

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips);
}
