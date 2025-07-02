#include "AvgStat.hpp"
#include "Optimize_v2.hpp"
#include "Random.hpp"
#include "Recommendations.hpp"
#include "Stats.hpp"

#include <future>
#include <iostream>

f64 reward_function(const std::vector<stats::avg_stat>& sorted_flips, class random& rng);

void optimize_v2_recommendation_algorithm(const db& db)
{
	if (db.total_flip_count() < 200)
	{
		std::cout << "the amount of flips is too low to even attempt optimizing anything\n";
		return;
	}

	// some initialization stuff
	stats::avg_stat::set_recommendation_algorithm(2);
	const std::vector<stats::avg_stat> raw_flips = db.get_flip_avg_stats();

	// filter out flips with lacking data
	constexpr u8 min_flip_data = 10;
	std::vector<stats::avg_stat> flips;
	for (const stats::avg_stat& flip : raw_flips)
	{
		if (flip.flip_count() >= min_flip_data)
			flips.push_back(flip);
	}

	assert(!flips.empty());

	class random rng;

	// reset all weights to even values
	// the weight array is stored in Recommendations.hpp
	for (size_t i = 0; i < v2_recommendation_algorithm_weights.size(); ++i)
		v2_recommendation_algorithm_weights[i] = 1.0 / v2_recommendation_algorithm_weights.size();

	// start cooking the numbers
	f64 best_reward = reward_function(stats::sort_flips_by_recommendation(flips), rng);
	std::array<f64, v2_variable_count> best_weights = v2_recommendation_algorithm_weights;
	std::cout << "starting profit: " << flip_utils::round_big_numbers(best_reward) << '\n';

	size_t i{0};
	size_t v2_var_to_tweak{0};

	// TODO: throw multithreading at the problem, its too slow :(

	while (true)
	{
		// randomly tweak the weights
		//
		// most of the time try to improve things and the rest of the time try
		// something completely new
		if (rng.next() % 7 != 0)
		{
			const f32 change_factor = rng.range_float(0.8f, 1.2f);
			v2_recommendation_algorithm_weights[v2_var_to_tweak] *= change_factor;
			v2_var_to_tweak = (v2_var_to_tweak + 1) % v2_variable_count;
		}
		else
		{
			for (u8 j = 0; j < v2_variable_count; ++j)
				v2_recommendation_algorithm_weights[j] = rng.range_float(0.0f, 1.0f);
		}

		// clamp the weights to 0.0 - 1.0
		for (f64& weight : v2_recommendation_algorithm_weights)
			weight = std::clamp(weight, 0.0, 1.0);

		// make sure that the weights sum up to one
		f64 weight_sum{0};
		for (size_t k = 0; k < v2_variable_count; ++k)
			weight_sum += v2_recommendation_algorithm_weights[k];

		if (weight_sum == 0.0)
			continue;

		for (f64& weight : v2_recommendation_algorithm_weights)
			weight /= weight_sum;

		const std::vector<stats::avg_stat> recommended_flips = stats::sort_flips_by_recommendation(flips);
		const f64 reward = reward_function(recommended_flips, rng);

		// if the reward is lower than before, set the weights back to the previous ones
		if (reward <= best_reward)
		{
			v2_recommendation_algorithm_weights = best_weights;
		}
		else
		{
			best_reward = reward;
			best_weights = v2_recommendation_algorithm_weights;
			std::cout << std::setw(6) << i << " | " << std::setw(7) << flip_utils::round_big_numbers(best_reward) << " | ";

			// print the weights
			std::cout << "{ ";
			for (u8 v = 0; v < v2_variable_count; ++v)
			{
				std::cout << v2_recommendation_algorithm_weights[v];
				if (v != v2_variable_count - 1)
					std::cout << ", ";
			}
			std::cout << " }\n";
		}

		i++;
	}
}

f64 reward_function(const std::vector<stats::avg_stat>& sorted_flips, class random& rng)
{
	// run a simulations with the flip recommendations

	constexpr u16 simulation_repetitions = 5;
	constexpr u8 hours = 24;
	constexpr u8 cooldown_duration = 4;
	constexpr u8 top_flip_count = 50;
	assert(sorted_flips.size() >= top_flip_count);

	const auto simulation_run = [&sorted_flips, &rng]() -> f64
	{
		f64 total_profit{0};
		std::map<size_t, u8> buy_limit_cooldowns;
		std::map<size_t, u32> item_trade_index;

		for (u8 hour = 0; hour < hours; ++hour)
		{
			// find 8 items (ge slot count) that are not on a cooldown
			constexpr u8 max_concurrent_flip_count = 8;
			u8 flipped_item_count{0};

			for (size_t i = 0; i < sorted_flips.size() && i < top_flip_count && flipped_item_count < max_concurrent_flip_count; ++i)
			{
				if (buy_limit_cooldowns.contains(i) && buy_limit_cooldowns.at(i) > 0)
					continue;

				// check if the flip was cancelled
				const bool got_cancelled = rng.range_float(0.0f, 1.0f) < sorted_flips[i].cancellation_ratio();
				if (got_cancelled)
					continue;

				// only consider the last 15 flips done with the item
				// this should help a little bit with cases where the item has been flipped
				// for ages and the profitability has gone down over time

				const auto update_trade_index = [&item_trade_index, &sorted_flips](const size_t i)
				{
					if (!item_trade_index.contains(i) || item_trade_index[i] >= sorted_flips[i].profits().size())
					{
						item_trade_index[i] = sorted_flips[i].profits().size() >= 15
							? sorted_flips[i].profits().size() - 15
							: 0;
					}
				};

				// do a flip with the item

				update_trade_index(i);
				const std::vector<i32>& profit_list = sorted_flips[i].profits();
				const i64 profit = profit_list.at(item_trade_index[i]++);
				update_trade_index(i);

				total_profit += profit;
				buy_limit_cooldowns[i] = cooldown_duration;
				flipped_item_count++;
			}

			assert(flipped_item_count != 0);

			// reduce the cooldown of all flips by one hour
			for (auto& [flip_index, cooldown] : buy_limit_cooldowns)
			{
				if (cooldown > 0)
					cooldown--;
			}
		}

		return total_profit;
	};

	f64 repetition_total_profit{0};
	for (size_t sim_rep = 0; sim_rep < simulation_repetitions; ++sim_rep)
		repetition_total_profit += simulation_run();

	return repetition_total_profit / simulation_repetitions;
}
