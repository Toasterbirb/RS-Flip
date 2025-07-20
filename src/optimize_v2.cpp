#include "AvgStat.hpp"
#include "Optimize_v2.hpp"
#include "Random.hpp"
#include "Recommendations.hpp"
#include "Stats.hpp"

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
	constexpr u8 min_flip_data = 4;
	std::vector<stats::avg_stat> flips;
	for (const stats::avg_stat& flip : raw_flips)
	{
		if (flip.flip_count() >= min_flip_data)
			flips.push_back(flip);
	}

	assert(!flips.empty());

	class random rng;

	constexpr u8 initial_info_text_width = 36;

	// check what the reward value would be with the current weights
	// this should be good for checking if the newly generated weights are better ones
	{
		stats::sort_flips_by_recommendation_direct(flips);
		const f64 reward = reward_function(flips, rng);
		std::cout << std::left << std::setw(initial_info_text_width) << "profit with current weights: " << flip_utils::round_big_numbers(reward) << '\n';
	}

	// reset all weights to even values
	// the weight array is stored in Recommendations.hpp
	for (size_t i = 0; i < v2_recommendation_algorithm_weights.size(); ++i)
		v2_recommendation_algorithm_weights[i] = 1.0 / v2_recommendation_algorithm_weights.size();

	// measure the margin of error with a few runs
	const f64 margin_of_error = [&flips, &rng]() -> f64
	{
		std::vector<f64> profits;
		constexpr u16 margin_of_error_round_count = 1000;
		for (u16 i = 0; i < margin_of_error_round_count; ++i)
		{
			const f64 profit = reward_function(flips, rng);
			profits.push_back(profit);
		}

		const f64 min = *std::min_element(profits.begin(), profits.end());
		const f64 max = *std::max_element(profits.begin(), profits.end());
		std::cout << std::setw(initial_info_text_width) << "margin of error: " << flip_utils::round_big_numbers(max - min) << '\n';
		return max - min;
	}();

	// start cooking the numbers
	f64 best_reward = reward_function(stats::sort_flips_by_recommendation(flips), rng);
	std::array<f64, v2_variable_count> best_weights = v2_recommendation_algorithm_weights;
	std::cout << std::left << std::setw(initial_info_text_width) << "starting profit with even weights: " << flip_utils::round_big_numbers(best_reward) << '\n';

	// use the right alignment for the result printing
	std::cout << std::right;
	size_t i{0};

	// start out by exploring different random options and then start improving
	// the best result that could be found while still slowly exploring some
	// other random options
	size_t last_new_best_time = time(0);
	bool explore = true;

	// TODO: throw multithreading at the problem, its too slow :(

	while (true)
	{
		const u8 strategy = explore ? 2 : 8;

		// print progress and check if the strategy should be changed
		if (i % 64 == 0)
		{
			constexpr u32 explore_threshold_seconds = 15 * 60;
			const size_t cur_time = time(0);
			const size_t elapsed_time = cur_time - last_new_best_time;

			if (elapsed_time > explore_threshold_seconds)
				explore = false;

			std::cout << "\riteration: " << i << " (" << ( explore ? "explore" : "exploit" ) << ")" << std::flush;
		}

		// randomly tweak the weights
		if (rng.next() % strategy != 0)
		{
			// tweak a random amount of variables by a random amount
			// this can tweak the same variable multiple times, thus multiplying the effect
			const u8 vars_to_tweak = rng.range(1, v2_variable_count - 1);

			for (u8 i = 0; i < vars_to_tweak; ++i)
			{
				const u8 index = rng.next() % v2_variable_count;
				f64& weight = v2_recommendation_algorithm_weights[index];

				// multiplying very small values doesn't really make a difference, so use
				// addition with those
				if (weight > 0.1)
					weight *= rng.range_float(0.9f, 1.1f);
				else
					weight += rng.range_float(0.001f, 0.1f);
			}
		}
		else
		{
			for (u8 j = 0; j < v2_variable_count; ++j)
				v2_recommendation_algorithm_weights[j] = rng.range_float(0.0f, 1.0f);
		}

		// make sure that the weights sum up to one
		f64 weight_sum{0};
		for (size_t k = 0; k < v2_variable_count; ++k)
			weight_sum += v2_recommendation_algorithm_weights[k];

		if (weight_sum == 0.0)
			continue;

		for (f64& weight : v2_recommendation_algorithm_weights)
			weight /= weight_sum;

		stats::sort_flips_by_recommendation_direct(flips);
		const f64 reward = reward_function(flips, rng);

		// if the reward is lower than before, set the weights back to the previous ones
		// only consider the reward being higher, if the difference is higher than the margin
		// of error
		if (reward - margin_of_error <= best_reward)
		{
			v2_recommendation_algorithm_weights = best_weights;
		}
		else
		{
			best_reward = reward;
			best_weights = v2_recommendation_algorithm_weights;
			last_new_best_time = time(0);
			std::cout << "\r" << std::setw(7) << i << " | " << std::setw(8) << flip_utils::round_big_numbers(best_reward) << " | ";

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

	constexpr u16 simulation_repetitions = 500;
	constexpr u8 hours = 48;
	constexpr u8 cooldown_duration = 4;
	constexpr u8 top_flip_count = 50;
	assert(sorted_flips.size() >= top_flip_count);

	const auto simulation_run = [&sorted_flips, &rng]() -> f64
	{
		f64 total_profit{0};
		std::array<u8, top_flip_count> buy_limit_cooldowns{0};

		for (u8 hour = 0; hour < hours; ++hour)
		{
			// find 8 items (ge slot count) that are not on a cooldown
			constexpr u8 max_concurrent_flip_count = 8;
			u8 flipped_item_count{0};

			for (size_t i = 0; i < sorted_flips.size() && i < top_flip_count && flipped_item_count < max_concurrent_flip_count; ++i)
			{
				if (buy_limit_cooldowns.at(i) > 0)
					continue;

				// check if the flip was cancelled
				// still give the item a cooldown, since it wouldn't make sense to
				// flip it again right after and waste more time
				//
				// also make the cooldown slightly longer
				const bool got_cancelled = rng.range_float(0.0f, 1.0f) < sorted_flips[i].cancellation_ratio();
				if (got_cancelled)
				{
					buy_limit_cooldowns[i] = cooldown_duration * 1.25;
					continue;
				}

				const std::vector<i32>& profit_list = sorted_flips[i].profits();

				// only consider the last few flips done with the item
				// this should help a little bit with cases where the item has been flipped
				// for ages and the profitability has gone down over time
				constexpr u8 flips_to_consider = 15;

				const u8 divisor = profit_list.size() >= flips_to_consider ? flips_to_consider : profit_list.size();
				const i64 profit = profit_list.at(profit_list.size() - (i % divisor) - 1);

				total_profit += profit;
				buy_limit_cooldowns[i] = cooldown_duration;
				flipped_item_count++;
			}
			assert(flipped_item_count != 0);

			// reduce the cooldown of all flips by one hour
			for (u8& cooldown : buy_limit_cooldowns)
			{
				if (cooldown > 0) [[likely]]
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
