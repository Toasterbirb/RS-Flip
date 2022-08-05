#pragma once
#include "pch.hpp"

namespace Flips
{
	static const std::string user_home = (std::string)getenv("HOME");
	static const std::string data_path = user_home + "/.local/share/rs-flip";
	static const std::string data_file = data_path + "/flips.json";

	struct Flip
	{
		Flip();
		Flip(nlohmann::json j);
		Flip(const std::string& item, const int& buy_price, const int& sell_price, const int& buy_amount);
		void Sell(const int& sell_price);
		nlohmann::json ToJson();

		std::string item;
		int buy_price;
		int sell_price;
		int sold_price;
		int buylimit;
		bool cancelled;
		bool done; /* Is the flip completed */
	};

	void Init();
	void PrintStats(const int& topValueCount = 10, const bool& only_stability = false);
	void FixStats();
	void RestoreBackup();
	void List(); /* List on-going flips */
	void Add(Flip flip); /* Add a new flip */
	void Cancel(const int& ID); /* Cancel an existing flip */
	void Sell(const int& index, int sell_value, int sell_amount);

	static nlohmann::json json_data;
	static std::vector<nlohmann::json> flips;

	/* Filtering */
	void FilterName(const std::string& name);
	void FilterCount(const int& flip_count);

	/* Flip recommendations */
	bool FlipRecommendations();
}
