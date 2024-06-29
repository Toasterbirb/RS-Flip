#pragma once

#include "DB.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace flips
{
	static constexpr int recommendation_count = 35;
	static constexpr int random_flip_count = 5;

	struct flip
	{
		flip();
		flip(const nlohmann::json& j);
		flip(const std::string& item, const int buy_price, const int sell_price, const int buy_amount, const std::string& account_name = "main");
		void sell(const int sell_price);
		nlohmann::json to_json() const;

		std::string item;
		int buy_price;
		int sell_price;
		int sold_price;
		int buylimit;
		bool cancelled;
		bool done; /* Is the flip completed */
		std::string account; /* The runescape account that has the flip active */
	};

	void print_stats(const db& db, const int top_value_count = 10);
	void fix_stats(db& db);
	void list(const db& db, const std::string& account_filter = ""); /* List on-going flips */
	void cancel(db& db, const int ID); /* Cancel an existing flip */
	void sell(db& db, const int index, int sell_value, int sell_amount);

	/** Filtering **/

	/* Find finished flips with item name */
	std::vector<nlohmann::json> find_flips_by_name(const db& db, const std::string& item_name);

	/* Print filtered data */
	void filter_name(const db& db, const std::string& name);
	void filter_count(const db& db, const u32 flip_count);

	/* Flip recommendations */
	bool flip_recommendations(const db& db);
}
