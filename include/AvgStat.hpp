#pragma once
#include "pch.hpp"

#define PROFIT_WEIGHT 0.8
#define BUYLIMIT_WEIGHT 0.2
#define FLIP_COUNT_MULTIPLIER 0.90

namespace Stats
{
	class AvgStat
	{
	public:
		AvgStat();
		AvgStat(const std::string& item_name);
		void AddData(const int& profit, const double& ROI, const int& item_count, const int& sell = 0, const int& sold = 0);
		double AvgProfit() const;
		double AvgROI() const;
		double AvgBuyLimit() const;
		double FlipStability() const;
		int FlipCount() const;

		std::string name;

	private:

		int total_profit;
		double total_roi;
		int total_item_count;
		int value_count;

		int lowest_profit;
		int highest_profit;
		int lowest_item_count;
		int highest_item_count;

		int total_sell_sold_distance;
	};

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips);
}
