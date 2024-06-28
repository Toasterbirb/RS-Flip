#pragma once

#include "AvgStat.hpp"
#include "pch.hpp"

namespace stats
{
	double calc_roi(const int buy_price, const int sell_price);
	double calc_roi(const nlohmann::json& flip);
	std::vector<avg_stat> sort_flips_by_roi(std::vector<avg_stat> flips);
	std::vector<avg_stat> sort_flips_by_profit(std::vector<avg_stat> flips);
	std::vector<avg_stat> sort_flips_by_recommendation(std::vector<avg_stat> flips);
}
