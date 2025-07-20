#include "Recommendations.hpp"

#include <cassert>
#include <iostream>

f64 v2_recommendation_algorithm(const stats::avg_stat& stat, const std::array<f64, v2_variable_count>& weights)
{
	// variables and their weights
	const f64 variables[v2_variable_count] = {
		// avg profit
		stat.normalized_avg_profit(),

		// success rate
		stat.profitable_flip_count() / static_cast<f64>(stat.flip_count()),

		// consistency
		1.0 / (stat.profit_standard_deviation() + 1),

		// cancellation penalty
		1.0 - stat.cancellation_ratio(),

		// how many flips have been done
		stat.flip_count() >= 15 ? 1.0 : stat.flip_count() / 15.0,

		// average return on investment
		stat.normalized_avg_roi(),

		// average buy limit
		stat.normalized_avg_buy_limit(),

		// reversed average buy limit
		1.0 - stat.normalized_avg_buy_limit()
	};

	f64 composite_score{0};
	for (u8 i = 0; i < v2_variable_count; ++i)
		composite_score += variables[i] * weights[i];

	assert(composite_score <= static_cast<f64>(v2_variable_count));

	// lower the score for flips with out-of-date data
	const f64 flip_age = stat.latest_trade_index() / static_cast<f64>(stat.total_flip_count());
	const f64 flip_age_penalty = std::clamp(flip_age, 0.90, 1.0);

	return composite_score * flip_age_penalty;
}
