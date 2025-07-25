#pragma once

#include "DB.hpp"
#include "Dailygoal.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace flips
{
	struct tip_config
	{
		i64 profit_threshold = 2'000'000;
		u32 max_result_count = 35;
		u32 max_random_flip_count = 0;
		bool ge_inspector_format = false;
		u8 recommendation_algorithm = 0; // set to non-zero value to pick a non-default algorithm
		bool use_blacklist = true;
	};

	struct flip
	{
		flip();
		explicit flip(const nlohmann::json& j);
		flip(const std::string& item, const i32 buy_price, const i32 sell_price, const i32 buy_amount, const std::string& account_name = "main");
		void sell(const i32 sell_price);
		nlohmann::json to_json() const;

		std::string item;
		i32 buy_price;
		i32 sell_price;
		i32 sold_price;
		i32 buylimit;
		bool cancelled;
		bool done; /* Is the flip completed */
		std::string account; /* The runescape account that has the flip active */
	};

	void print_stats(const db& db, const i32 top_value_count = 10);
	void fix_stats(db& db);
	void list(const db& db, const daily_progress& daily_progress, const std::string& account_filter = ""); /* List on-going flips */
	void cancel(db& db, const i32 ID); /* Cancel an existing flip */
	void update(db& db, const i32 ID, u32 buy_price, u32 sell_price, u32 buy_amount, std::string account_name); /* Update flip information */
	void sell(db& db, daily_progress& daily_progress, const i32 index, i32 sell_value, i32 sell_amount);

	/** Filtering **/

	/* Find finished flips with item name */
	std::vector<nlohmann::json> find_flips_by_name(const db& db, const std::string& item_name);

	/* Find unfinished flips with their ID */
	__attribute__((warn_unused_result))
	i32 find_real_id_with_undone_id(const db& db, const u32 undone_id);

	/* Print filtered data */
	void filter_name(const db& db, const std::string& name);
	void filter_count(const db& db, const u32 flip_count);

	/* Flip recommendations */
	bool flip_recommendations(const db& db, const tip_config& config);
}
