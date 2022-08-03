#pragma once
#include "pch.hpp"
#include "AvgStat.hpp"

namespace Stats
{
	double CalcROI(const int& buy_price, const int& sell_price);
	double CalcROI(const nlohmann::json& flip);
	std::vector<AvgStat> SortFlipsByROI(const std::vector<AvgStat>& flips);
	std::vector<AvgStat> SortFlipsByProfit(const std::vector<AvgStat>& flips);
}
