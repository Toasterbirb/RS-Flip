#include "Stats.hpp"
#include "Table.hpp"
#include "Margin.hpp"
#include "AvgStat.hpp"
#include "Flips.hpp"
#include "FlipUtils.hpp"
#include "Dailygoal.hpp"

#define DEFAULT_DATA_FILE "{\"stats\":{\"profit\":0,\"flips_done\":0},\"flips\":[]}\n"
constexpr int RECOMMENDATION_THRESHOLD = 750000;

namespace Flips
{
	Flip::Flip()
	{
		item 		= "null";
		buy_price 	= 0;
		sell_price 	= 0;
		sold_price 	= 0;
		buylimit 	= 0;
		done 		= false;
	}

	Flip::Flip(nlohmann::json j)
	{
		/* Asserts for checking if the json object has all of the required keys */
		assert(j.contains("item"));
		assert(j.contains("buy"));
		assert(j.contains("sell"));
		assert(j.contains("sold"));
		assert(j.contains("limit"));
		assert(j.contains("cancelled"));
		assert(j.contains("done"));

		item 		= j["item"];
		buy_price 	= j["buy"];
		sell_price 	= j["sell"];
		sold_price 	= j["sold"];
		buylimit 	= j["limit"];
		cancelled 	= j["cancelled"];
		done 		= j["done"];

		/* Assume "main" as the account if its not specified */
		if (!j.contains("account"))
			account = "main";
		else
			account = j["account"];
	}

	Flip::Flip(const std::string& item, const int buy_price, const int sell_price, const int buy_amount, const std::string& account_name)
	{
		this->item 			= item;
		this->buy_price 	= buy_price;
		this->sell_price 	= sell_price;
		this->sold_price 	= 0;
		this->buylimit 		= buy_amount;
		this->account 		= account_name;
		this->cancelled 	= false;
		this->done 			= false;
	}

	void Flip::Sell(const int& sell_price)
	{
		this->sell_price 	= sell_price;
		this->done 			= true;
	}

	nlohmann::json Flip::ToJson()
	{
		nlohmann::json j;

		j["item"] 		= item;
		j["buy"] 		= buy_price;
		j["sell"] 		= sell_price;
		j["sold"] 		= sold_price;
		j["limit"] 		= buylimit;
		j["cancelled"] 	= cancelled;
		j["done"] 		= done;

		/* If the account value is empty, default it to "main" */
		if (account.empty())
			account = "main";

		j["account"] 	= account;

		return j;
	}

	void CreateDefaultDataFile()
	{
		FlipUtils::WriteFile(data_file, DEFAULT_DATA_FILE);
	}

	void WriteJson()
	{
		assert(data_file.empty() == false);

		/* Backup the file before writing anything */
		std::filesystem::copy_file(data_file, data_file + "_backup", std::filesystem::copy_options::overwrite_existing);

		FlipUtils::WriteJsonFile(json_data, data_file);
	}

	void LoadFlipArray()
	{
		flips.clear();

		/* Add existing flips to the array */
		for (size_t i = 0; i < json_data["flips"].size(); i++)
		{
			/* Don't load cancelled flips */
			if (json_data["flips"][i]["cancelled"] == true)
				continue;

			flips.push_back(json_data["flips"][i]);
		}
	}

	void Init()
	{
		assert(data_path.empty() == false);
		assert(data_file.empty() == false);

		if (!std::filesystem::exists(data_path))
			std::filesystem::create_directories(data_path);

		if (!std::filesystem::exists(data_file))
			CreateDefaultDataFile();

		/* Read the json data file */
		std::string json_string = FlipUtils::ReadFile(data_file);
		json_data = nlohmann::json::parse(json_string);

		LoadFlipArray();
	}

	void ApplyFlipArray()
	{
		json_data["flips"] = flips;
		WriteJson();
	}

	void PrintStats(const int& topValueCount)
	{
		Init();

		/* Print top performing flips */
		std::vector<Stats::AvgStat> stats = Stats::FlipsToAvgstats(flips);

		if (stats.empty())
		{
			std::cout << "There are no flips to print any statistics about yet!\n";
			return;
		}

		FlipUtils::PrintTitle("Stats");
		std::cout << "Total profit: " << FlipUtils::RoundBigNumbers(json_data["stats"]["profit"]) << std::endl;
		std::cout << "Flips done: " << FlipUtils::RoundBigNumbers(json_data["stats"]["flips_done"]) << std::endl;
		std::cout << "\n";

		/* Quit if zero flips done */
		if (json_data["stats"]["flips_done"] == 0)
			return;

		FlipUtils::PrintTitle("Top flips by ROI-%");
		std::vector<Stats::AvgStat> topROI = Stats::SortFlipsByROI(stats);

		Table flips_by_roi({"Item", "ROI-%", "Average profit"});

		for (int i = 0; i < FlipUtils::Clamp(topROI.size(), 0, topValueCount); i++)
			flips_by_roi.add_row({topROI[i].name, std::to_string(topROI[i].AvgROI()), FlipUtils::RoundBigNumbers(topROI[i].AvgProfit())});

		flips_by_roi.print();

		std::cout << "\n";

		FlipUtils::PrintTitle("Top flips by Profit");
		std::vector<Stats::AvgStat> topProfit = Stats::SortFlipsByProfit(stats);

		Table flips_by_profit({"Item", "Average profit", "ROI-%"});

		for (int i = 0; i < FlipUtils::Clamp(topProfit.size(), 0, topValueCount); i++)
		{
			std::string avgprofit_string = FlipUtils::RoundBigNumbers(topProfit[i].AvgProfit());
			flips_by_profit.add_row({topProfit[i].name, avgprofit_string, std::to_string(topProfit[i].AvgROI())});
		}

		flips_by_profit.print();
	}

	void FixStats()
	{
		Init();
		std::cout << "Recalculating statistics..." << std::endl;

		int total_profit = 0;
		int flip_count = 0;

		for (size_t i = 0; i < json_data["flips"].size(); i++)
		{
			/* Check if the flip is done */
			if (json_data["flips"][i]["done"] == true)
			{
				flip_count++;

				/* Flip is done, but the sold price is missing */
				if (json_data["flips"][i]["sold"] == 0)
					json_data["flips"][i]["sold"] = json_data["flips"][i]["sell"];

				/* Calculate the profit */
				int buy_price 	= json_data["flips"][i]["buy"];
				int sell_price 	= json_data["flips"][i]["sold"];
				int limit 		= json_data["flips"][i]["limit"];
				total_profit += Margin::CalcProfit(buy_price, sell_price, limit);
				//total_profit += (sell_price - buy_price) * limit;
			}
		}

		/* Update the stats values */
		json_data["stats"]["profit"] 		= total_profit;
		json_data["stats"]["flips_done"] 	= flip_count;

		/* Update the data file */
		WriteJson();
	}

	void RestoreBackup()
	{
		std::cout << "Restoring the backup. Make sure to create a new one if you want to keep it, because the old one is lost" << std::endl;

		/* Check if the backup exists */
		if (std::filesystem::exists(data_file + "_backup"))
			std::filesystem::rename(data_file + "_backup", data_file);
	}

	void List()
	{
		Init();

		std::vector<nlohmann::json> undone_flips;

		for (size_t i = 0; i < flips.size(); i++)
		{
			/* Check if the flip is done yet */
			if (flips[i]["done"] == true)
				continue;

			undone_flips.push_back(flips[i]);
		}

		if (undone_flips.empty())
		{
			std::cout 	<< "No active flips were found...\n"
						<< "You might wanna go to the Grand Exchange and start some.\n";
			return;
		}

		Table ongoing_flips({"ID", "Item", "Count", "Buy", "Sell", "Account"});

		FlipUtils::PrintTitle("On-going flips");
		for (size_t i = 0; i < undone_flips.size(); i++)
		{
			std::string flip_name 	= undone_flips[i]["item"];
			int flip_item_count 	= undone_flips[i]["limit"];
			int flip_buy 			= undone_flips[i]["buy"];
			int flip_sell 			= undone_flips[i]["sell"];

			std::string account;
			if (undone_flips[i].contains("account"))
				account = undone_flips[i]["account"];
			else
				account = "main";

			ongoing_flips.add_row({std::to_string(i), flip_name, std::to_string(flip_item_count), std::to_string(flip_buy), std::to_string(flip_sell), account});
		}

		ongoing_flips.print();

		/* Print out daily goal */
		DailyProgress daily_progress;
		std::cout << "\n";
		daily_progress.PrintProgress();
	}

	int FindRealIDWithUndoneID(const int& undone_ID)
	{
		int undone_index = 0;
		int result;
		bool result_found = false;
		for (size_t i = 0; i < flips.size(); i++)
		{
			/* Skip flips that are already done */
			if (flips[i]["done"] == true)
				continue;

			if (undone_index != undone_ID)
				undone_index++;
			else
			{
				result = i;
				result_found = true;
				break;
			}
		}

		if (!result_found)
		{
			std::cout << "No items matching ID [" << undone_ID << "] were found!" << std::endl;
			return -1;
		}
		else
		{
			return result;
		}
	}

	void Add(Flip flip)
	{
		Init();
		flips.push_back(flip.ToJson());
		ApplyFlipArray();
	}

	void Cancel(const int& ID)
	{
		Init();

		/* Mark the flip as cancelled. It will be removed when the flip array
		 * is loaded next time around and saved */
		int flip_to_cancel = FindRealIDWithUndoneID(ID);
		flips[flip_to_cancel]["cancelled"] = true;

		std::cout << "Flip [" << flips[flip_to_cancel]["item"] << "] cancelled!\n";

		ApplyFlipArray();
	}

	void Sell(const int& index, int sell_value, int sell_amount)
	{
		Init();
		int result = FindRealIDWithUndoneID(index);
		if (result == -1)
			return;

		/* Update the flip values */
		if (sell_amount == 0)
			sell_amount = flips[result]["limit"];
		else
			flips[result]["limit"] = sell_amount;

		flips[result]["done"] = true;

		if (sell_value == 0)
			sell_value = flips[result]["sell"];

		flips[result]["sold"] = sell_value;

		/* Update the stats */
		int flips_done = json_data["stats"]["flips_done"];
		flips_done++;
		json_data["stats"]["flips_done"] = flips_done;

		int total_profit = json_data["stats"]["profit"];
		int buy_price = flips[result]["buy"];
		int profit = Margin::CalcProfit(buy_price, sell_value, sell_amount);
		//int profit = ((sell_value - buy_price) * sell_amount);
		total_profit += profit;
		json_data["stats"]["profit"] = total_profit;

		FlipUtils::PrintTitle("Flip complete");
		std::cout << "Item: " << flips[result]["item"] << std::endl;
		std::cout << "Profit: " << profit << " (" << FlipUtils::RoundBigNumbers(profit) << ")" << std::endl;
		std::cout << "Total profit so far: " << total_profit << " (" << FlipUtils::RoundBigNumbers(total_profit) << ")" << std::endl;

		/* Handle daily progress */
		DailyProgress daily_progress;
		daily_progress.AddProgress(profit);
		std::cout << "\n";
		daily_progress.PrintProgress();

		/* Update the json file */
		ApplyFlipArray();
	}

	/* Filtering */
	void FilterName(const std::string& name)
	{
		Init();
		std::cout << "Filter: " << name << std::endl;

		/* Quit if zero flips done */
		int flip_count = Flips::json_data["stats"]["flips_done"];
		if (flip_count == 0)
			return;

		std::vector<nlohmann::json> found_flips;
		for (int i = 0; i < Flips::json_data["flips"].size(); i++)
		{
			std::string flip_name = flips[i]["item"];
			if (flip_name == name && flips[i]["done"])
				found_flips.push_back(flips[i]);
		}

		std::cout << "Results: " << found_flips.size() << std::endl;
		if (found_flips.size() == 0)
			return;

		/* List out the flips */
		std::cout << "|-------------|-------------|---------|-------------|" << std::endl;
		std::cout << "| Buy         | Sell        | Count   | Profit      |" << std::endl;
		std::cout << "|-------------|-------------|---------|-------------|" << std::endl;

		int total_profit = 0;
		for (size_t i = 0; i < found_flips.size(); i++)
		{
			Flip flip(found_flips[i]);

			std::string buy_price = FlipUtils::RoundBigNumbers(flip.buy_price);
			std::string sell_price = FlipUtils::RoundBigNumbers(flip.sold_price);
			//int profit = (flip.sold_price - flip.buy_price) * flip.buylimit;
			int profit = Margin::CalcProfit(flip.buy_price, flip.sold_price, flip.buylimit);
			total_profit += profit;
			std::string profit_text = FlipUtils::RoundBigNumbers(profit);
			std::string item_count = std::to_string(flip.buylimit);

			std::cout << "| " << buy_price << std::setw(14 - buy_price.length()) << " | " <<
				sell_price << std::setw(14 - sell_price.length()) << " | " <<
				item_count << std::setw(10 - item_count.length()) << " | " <<
				profit_text << std::setw(13 - profit_text.length()) << "|" << std::endl;
		}

		std::cout << "|-------------|-------------|---------|-------------|" << std::endl;

		/* Calculate average profit */
		std::cout << "\n\e[33mAverage profit: " << FlipUtils::RoundBigNumbers((double)total_profit / found_flips.size()) << "\e[0m" << std::endl;
		std::cout << "\e[32mTotal profit:   " << FlipUtils::RoundBigNumbers((double)total_profit) << "\e[0m" << std::endl;
	}

	void FilterCount(const int& flip_count)
	{
		Init();

		/* Don't do anything if there are no flips in the db */
		if (flips.size() == 0)
			return;

		/* Don't do anything if the flip count given is dumb */
		if (flip_count < 1)
			return;

		std::vector<Stats::AvgStat> avgStats = Stats::FlipsToAvgstats(flips);

		for (size_t i = 0; i < avgStats.size(); i++)
		{
			if (avgStats[i].FlipCount() <= flip_count)
				std::cout << avgStats[i].name << std::endl;
		}
	}

	bool FlipRecommendations()
	{
		Init();

		if (flips.size() < 10)
			return false;

		/* Read in the item recommendation blacklist */
		std::unordered_set<std::string> item_blacklist = FlipUtils::ReadFileItems(item_blacklist_file);

		FlipUtils::PrintTitle("Recommended flips");

		std::vector<Stats::AvgStat> avgStats = Stats::FlipsToAvgstats(flips);
		std::vector<Stats::AvgStat> recommendedFlips = Stats::SortFlipsByRecommendation(avgStats);

		Table recommendation_table({"Item name", "Recent avg. profit", "Avg. profit", "Flip count"});

		/* Print recommendations until the recommendation_count has been reached */
		int i = 0; 		/* The current recommended item */
		int count = 0; 	/* How many recommendations have been shown */

		/* How many items to recommend in total */
		int max = FlipUtils::Clamp(recommendedFlips.size(), 1, recommendation_count);

		while (count < max && i < recommendedFlips.size())
		{
			/* Skip items that are blacklisted */
			if (item_blacklist.contains(recommendedFlips[i].name))
			{
				++i;
				continue;
			}

			/* Skip items with average profit below a certain threshold */
			if (recommendedFlips[i].AvgProfit() < RECOMMENDATION_THRESHOLD)
			{
				++i;
				continue;
			}

			recommendation_table.add_row({recommendedFlips[i].name, FlipUtils::RoundBigNumbers(recommendedFlips[i].RollingAvgProfit()), FlipUtils::RoundBigNumbers(recommendedFlips[i].AvgProfit()), std::to_string(recommendedFlips[i].FlipCount())});

			++i;
			++count;
		}

		if (count == 0)
		{
			std::cout << "Couldn't find any flips to recommend...\n";
			return true;
		}

		recommendation_table.print();

		return true;
	}
}
