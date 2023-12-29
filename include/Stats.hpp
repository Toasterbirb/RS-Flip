#pragma once
#include "pch.hpp"
#include "AvgStat.hpp"

namespace Stats
{
	double CalcROI(const int buy_price, const int sell_price);
	double CalcROI(const nlohmann::json& flip);
	std::vector<AvgStat> SortFlipsByROI(std::vector<AvgStat> flips);
	std::vector<AvgStat> SortFlipsByProfit(std::vector<AvgStat> flips);
	std::vector<AvgStat> SortFlipsByRecommendation(std::vector<AvgStat> flips);
}
