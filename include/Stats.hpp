#pragma once

#include "AvgStat.hpp"

#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace stats
{
	__attribute__((hot, const))
	f64 calc_roi(const i32 buy_price, const i32 sell_price);

	__attribute__((hot, const))
	f64 calc_roi(const nlohmann::json& flip);

	__attribute__((cold))
	std::vector<avg_stat> sort_flips_by_roi(std::vector<avg_stat> flips);

	__attribute__((cold))
	std::vector<avg_stat> sort_flips_by_profit(std::vector<avg_stat> flips);

	__attribute__((cold))
	std::vector<avg_stat> sort_flips_by_recommendation(std::vector<avg_stat> flips);

	// optimized version of sort_flips_by_recommendation,
	// but avoids unnecessary copying
	//
	// mainly useful for the v2 algo optimization
	__attribute__((hot))
	void sort_flips_by_recommendation_direct(std::vector<avg_stat>& flips);
}
