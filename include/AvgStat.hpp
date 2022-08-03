#pragma once
#include "pch.hpp"

#define PROFIT_WEIGHT 0.7
#define BUYLIMIT_WEIGHT 0.3
#define FLIP_COUNT_MULTIPLIER 0.95

namespace Stats
{
	class AvgStat
	{
	public:
		AvgStat();
		AvgStat(const std::string& item_name);
		void AddData(const int& profit, const double& ROI, const int& item_count);
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

		int highest_profit;
		int highest_item_count;
	};

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips);
}
