#pragma once
#include "pch.hpp"

namespace Flips
{
	static const std::string user_home = (std::string)getenv("HOME");
	static const std::string data_path = user_home + "/.local/share/rs-flip";
	static const std::string data_file = data_path + "/flips.json";
	static const std::string item_blacklist_file = data_path + "/item_blacklist.txt";

	static constexpr int recommendation_count = 40;

	struct Flip
	{
		Flip();
		Flip(const nlohmann::json& j);
		Flip(const std::string& item, const int buy_price, const int sell_price, const int buy_amount, const std::string& account_name = "main");
		void Sell(const int sell_price);
		nlohmann::json ToJson() const;

		std::string item;
		int buy_price;
		int sell_price;
		int sold_price;
		int buylimit;
		bool cancelled;
		bool done; /* Is the flip completed */
		std::string account; /* The runescape account that has the flip active */
	};

	void Init();
	void PrintStats(const int top_value_count = 10);
	void FixStats();
	void RestoreBackup();
	void List(const std::string& account_filter = ""); /* List on-going flips */
	void Add(const Flip& flip); /* Add a new flip */
	void Cancel(const int ID); /* Cancel an existing flip */
	void Sell(const int index, int sell_value, int sell_amount);

	static nlohmann::json json_data;
	static std::vector<nlohmann::json> flips;

	/** Filtering **/

	/* Find finished flips with item name */
	std::vector<nlohmann::json> FindFlipsByName(const std::string& item_name);

	/* Print filtered data */
	void FilterName(const std::string& name);
	void FilterCount(const int flip_count);

	/* Flip recommendations */
	bool FlipRecommendations();

	/** Predicting the future **/

	/* Attempt to extrapolate the next flip for the given item
	 * based on past performance */
	Flip ExtrapolateFlipData(const std::string& item_name);
}
