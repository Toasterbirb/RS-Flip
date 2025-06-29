#pragma once

#include "AvgStat.hpp"
#include "FlipUtils.hpp"
#include "Types.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <array>

static inline std::array<std::function<f64(const stats::avg_stat& stat)>, 2> recommendation_algorithms = {
	[](const stats::avg_stat& stat) -> f64 // v1
	{
		if (stat.flip_count() == 0)
			return 0;

		constexpr f64 flip_age_penaly = 0.005; // Higher value lowers the score more for stale flips
		constexpr f64 flip_index_age_exponent = 0.9; // Increase the impact of flip age
		f64 flip_age_debuff = 1.0 - (flip_age_penaly * std::pow(stat.total_flip_count() - stat.latest_trade_index(), flip_index_age_exponent));

		// Set limits to the age penalty
		constexpr f64 min_penalty = 0.001;
		constexpr f64 max_penalty = 1.0;
		flip_age_debuff = std::clamp(flip_age_debuff, min_penalty, max_penalty);

		const f64 roi_modifier = flip_utils::limes(2, 1.5, 1, stat.avg_roi());
		const f64 flip_count_modifier = flip_utils::limes(2, 1, 3, stat.flip_count());

		constexpr f32 profit_exponent = 1.50f;

		constexpr f32 inverse_divisor = 1.0 / 10000.0;
		constexpr u32 rolling_avg_window_size = 10;
		return std::round((std::pow(stat.rolling_avg_profit(rolling_avg_window_size), profit_exponent) * flip_age_debuff * roi_modifier * flip_count_modifier) * inverse_divisor);
	},
	[](const stats::avg_stat& stat) -> f64 // v2
	{
		return 0;
	}
};
