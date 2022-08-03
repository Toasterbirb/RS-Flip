#pragma once
#include "pch.hpp"

namespace Stats
{
	class AvgStat
	{
	public:
		AvgStat();
		AvgStat(const std::string& item_name);
		void AddData(const int& profit, const double& ROI);
		double AvgProfit() const;
		double AvgROI() const;

		std::string name;

	private:

		int total_profit;
		double total_roi;
		int value_count;
	};

	std::vector<AvgStat> FlipsToAvgstats(const std::vector<nlohmann::json>& flips);
}
