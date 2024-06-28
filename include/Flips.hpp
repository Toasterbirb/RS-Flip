#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace flips
{
	static const std::string user_home = (std::string)getenv("HOME");
	static const std::string data_path = user_home + "/.local/share/rs-flip";
	static const std::string data_file = data_path + "/flips.json";
	static const std::string item_blacklist_file = data_path + "/item_blacklist.txt";

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

	void init();
	void print_stats(const int top_value_count = 10);
	void fix_stats();
	void restore_backup();
	void list(const std::string& account_filter = ""); /* List on-going flips */
	void add(const flip& flip); /* Add a new flip */
	void cancel(const int ID); /* Cancel an existing flip */
	void sell(const int index, int sell_value, int sell_amount);

	static nlohmann::json json_data;
	static std::vector<nlohmann::json> flips;

	/** Filtering **/

	/* Find finished flips with item name */
	std::vector<nlohmann::json> find_flips_by_name(const std::string& item_name);

	/* Print filtered data */
	void filter_name(const std::string& name);
	void filter_count(const int flip_count);

	/* Flip recommendations */
	bool flip_recommendations();
}
