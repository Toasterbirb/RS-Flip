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
	/* Declare some functions */
	int FindRealIDWithUndoneID(const int undone_ID);
	void ApplyFlipArray();
	void CreateDefaultDataFile();
	void LoadFlipArray();
	void WriteJson();


	Flip::Flip()
	{
		item 		= "null";
		buy_price 	= 0;
		sell_price 	= 0;
		sold_price 	= 0;
		buylimit 	= 0;
		done 		= false;
	}

	Flip::Flip(const nlohmann::json& j)
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

	void Flip::Sell(const int sell_price)
	{
		this->sell_price 	= sell_price;
		this->done 			= true;
	}

	nlohmann::json Flip::ToJson() const
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
			j["account"] = "main";
		else
			j["account"] = account;

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
		const std::string json_string = FlipUtils::ReadFile(data_file);
		json_data = nlohmann::json::parse(json_string);

		LoadFlipArray();
	}

	void ApplyFlipArray()
	{
		json_data["flips"] = flips;
		WriteJson();
	}

	void PrintStats(const int top_value_count)
	{
		Init();

		/* Print top performing flips */
		const std::vector<Stats::AvgStat> stats = Stats::FlipsToAvgstats(flips);

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
		const std::vector<Stats::AvgStat> topROI = Stats::SortFlipsByROI(stats);

		Table flips_by_roi({"Item", "ROI-%", "Average profit"});

		for (int i = 0; i < FlipUtils::Clamp(topROI.size(), 0, top_value_count); i++)
			flips_by_roi.add_row({topROI[i].name, std::to_string(topROI[i].AvgROI()), FlipUtils::RoundBigNumbers(topROI[i].AvgProfit())});

		flips_by_roi.print();

		std::cout << "\n";

		FlipUtils::PrintTitle("Top flips by Profit");
		const std::vector<Stats::AvgStat> topProfit = Stats::SortFlipsByProfit(stats);

		Table flips_by_profit({"Item", "Average profit", "ROI-%"});

		for (int i = 0; i < FlipUtils::Clamp(topProfit.size(), 0, top_value_count); i++)
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

		long total_profit = 0;
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

	void List(const std::string& account_filter)
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

		/* Set the account variable for the undone flips
		 * This is because the variable might be missing due to backwards compat reasons
		 *
		 * Also keep track of the accounts listed. If only main account was used, we can
		 * skip printing the Account column in the flip list command */
		bool flips_only_with_main = true;
		for (size_t i = 0; i < undone_flips.size(); ++i)
		{
			/* Check if the account variable was set */
			if (!undone_flips[i].contains("account"))
			{
				undone_flips[i]["account"] = "main";
				continue;
			}

			/* If account other than main was used, print the account
			 * column to the table */
			if (undone_flips[i]["account"] != "main")
				flips_only_with_main = false;
		}

		std::vector<std::string> table_column_names = {"ID", "Item", "Count", "Buy", "Sell"};

		/* Only add the "Account" column if other than the main account was used in any of the flips */
		if (flips_only_with_main == false)
			table_column_names.push_back("Account");

		Table ongoing_flips(table_column_names);

		FlipUtils::PrintTitle("On-going flips");
		for (size_t i = 0; i < undone_flips.size(); i++)
		{
			const std::string& flip_name	= undone_flips[i]["item"];
			const int flip_item_count		= undone_flips[i]["limit"];
			const int flip_buy				= undone_flips[i]["buy"];
			const int flip_sell				= undone_flips[i]["sell"];
			const std::string& account		= undone_flips[i]["account"];

			/* If using account filtering, skip rows with non-matching accounts */
			if (!account_filter.empty() && account != account_filter)
				continue;

			std::vector<std::string> data_row = {std::to_string(i), flip_name, std::to_string(flip_item_count), std::to_string(flip_buy), std::to_string(flip_sell)};

			/* Add Account column if other accounts than main were also used */
			if (flips_only_with_main == false)
				data_row.push_back(account);

			ongoing_flips.add_row(data_row);
		}

		ongoing_flips.print();

		/* Print out daily goal */
		const DailyProgress daily_progress;
		std::cout << "\n";
		daily_progress.PrintProgress();
	}

	int FindRealIDWithUndoneID(const int undone_ID)
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

	void Add(const Flip& flip)
	{
		Init();
		flips.push_back(flip.ToJson());
		ApplyFlipArray();
	}

	void Cancel(const int ID)
	{
		Init();

		/* Mark the flip as cancelled. It will be removed when the flip array
		 * is loaded next time around and saved */
		const int flip_to_cancel = FindRealIDWithUndoneID(ID);
		flips[flip_to_cancel]["cancelled"] = true;

		std::cout << "Flip [" << flips[flip_to_cancel]["item"] << "] cancelled!\n";

		ApplyFlipArray();
	}

	void Sell(const int index, int sell_value, int sell_amount)
	{
		Init();
		const int result = FindRealIDWithUndoneID(index);
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

		long total_profit = json_data["stats"]["profit"];
		int buy_price = flips[result]["buy"];
		const int profit = Margin::CalcProfit(buy_price, sell_value, sell_amount);
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

	std::vector<nlohmann::json> FindFlipsByName(const std::string& item_name)
	{
		std::vector<nlohmann::json> result;

		/* Quit if zero flips done */
		const int flip_count = Flips::json_data["stats"]["flips_done"];
		if (flip_count == 0)
			return result;

		std::copy_if(Flips::json_data["flips"].begin(), Flips::json_data["flips"].end(), std::back_inserter(result),
			[item_name](const nlohmann::json j) {
				return j["item"] == item_name && j["done"] == true;
			}
		);

		return result;
	}

	void FilterName(const std::string& name)
	{
		Init();
		std::cout << "Filter: " << name << std::endl;

		/* Quit if zero flips done */
		const int flip_count = Flips::json_data["stats"]["flips_done"];
		if (flip_count == 0)
			return;

		std::vector<nlohmann::json> found_flips = FindFlipsByName(name);

		std::cout << "Results: " << found_flips.size() << std::endl;
		if (found_flips.size() == 0)
			return;

		/* List out the flips */
		std::cout << "+-------------+-------------+---------+-------------+\n";
		std::cout << "| Buy         | Sell        | Count   | Profit      |\n";
		std::cout << "+-------------+-------------+---------+-------------+\n";

		int total_profit = 0;
		for (size_t i = 0; i < found_flips.size(); i++)
		{
			Flip flip(found_flips[i]);

			const std::string buy_price = FlipUtils::RoundBigNumbers(flip.buy_price);
			const std::string sell_price = FlipUtils::RoundBigNumbers(flip.sold_price);
			//int profit = (flip.sold_price - flip.buy_price) * flip.buylimit;
			const int profit = Margin::CalcProfit(flip.buy_price, flip.sold_price, flip.buylimit);
			total_profit += profit;
			const std::string profit_text = FlipUtils::RoundBigNumbers(profit);
			const std::string item_count = std::to_string(flip.buylimit);

			std::cout << "| " << buy_price << std::setw(14 - buy_price.length()) << " | " <<
				sell_price << std::setw(14 - sell_price.length())	<< " | " <<
				item_count << std::setw(10 - item_count.length()) 	<< " | " <<
				profit_text << std::setw(14 - profit_text.length()) << " |\n";
		}

		std::cout << "+-------------+-------------+---------+-------------+\n";

		/* Extrapolate one point further with the previous points of data */
		if (found_flips.size() > 1)
		{
			std::cout << "|                   extrapolated                    |\n";
			std::cout << "+-------------+-------------+---------+-------------+\n";

			const Flip extrapolated_flip = ExtrapolateFlipData(name);

			const int extrapolated_profit = Margin::CalcProfit(extrapolated_flip.buy_price, extrapolated_flip.sold_price, extrapolated_flip.buylimit);

			const std::string extrapolated_buy_str = FlipUtils::RoundBigNumbers(extrapolated_flip.buy_price);
			const std::string extrapolated_sell_str = FlipUtils::RoundBigNumbers(extrapolated_flip.sold_price);
			const std::string extrapolated_buy_limit_str = std::to_string(extrapolated_flip.buylimit);
			const std::string extrapolated_profit_str = FlipUtils::RoundBigNumbers(extrapolated_profit);

			std::cout << "\033[3m| " << extrapolated_buy_str << std::setw(14 - extrapolated_buy_str.length())	<< " | " <<
						extrapolated_sell_str		<< std::setw(14 - extrapolated_sell_str.length())		<< " | " <<
						extrapolated_buy_limit_str	<< std::setw(10 - extrapolated_buy_limit_str.length())	<< " | " <<
						extrapolated_profit_str 	<< std::setw(19 - extrapolated_profit_str.length())		<< " | \033[0m\n";
			std::cout << "+-------------+-------------+---------+-------------+" << std::endl;
		}

		/* Calculate average profit */
		std::cout << "\n\033[33mAverage profit: " << FlipUtils::RoundBigNumbers((double)total_profit / found_flips.size()) << "\033[0m" << std::endl;
		std::cout << "\033[32mTotal profit:   " << FlipUtils::RoundBigNumbers((double)total_profit) << "\033[0m" << std::endl;

		/** Calculate median buy and sell prices **/
		const int middle_index = std::floor(found_flips.size() / 2.0);
		std::vector<nlohmann::json> sorted_list = found_flips;

		std::cout << "\n";

		/* Sort by buying price */
		FlipUtils::JsonSort(sorted_list, "buy");
		std::cout << "Median buy price:  " << sorted_list.at(middle_index)["buy"] << "\n";

		/* Sort by selling price */
		FlipUtils::JsonSort(sorted_list, "sold");
		std::cout << "Median sell price: " << sorted_list.at(middle_index)["sold"] << "\n";

		std::cout << "\n";

		/** Calculate average buying and selling prices **/
		std::cout << "\033[37mAverage buy price:  " << FlipUtils::JsonAverage(found_flips, "buy") << "\033[0m\n";
		std::cout << "\033[37mAverage sell price: " << FlipUtils::JsonAverage(found_flips, "sold") << "\033[0m\n";

		std::cout << "\n";

		/** Find min and max buy/sell prices **/
		std::cout << "\033[34mMin buy price: " << FlipUtils::JsonMinInt(sorted_list, "buy") << "\033[0m\n";
		std::cout << "\033[34mMax buy price: " << FlipUtils::JsonMaxInt(sorted_list, "buy") << "\033[0m\n";

		std::cout << "\n";

		std::cout << "\033[35mMin sell price: " << FlipUtils::JsonMinInt(sorted_list, "sold") << "\033[0m\n";
		std::cout << "\033[35mMax sell price: " << FlipUtils::JsonMaxInt(sorted_list, "sold") << "\033[0m\n";
	}

	void FilterCount(const int flip_count)
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
		const std::unordered_set<std::string> item_blacklist = FlipUtils::ReadFileItems(item_blacklist_file);

		FlipUtils::PrintTitle("Recommended flips");

		const std::vector<Stats::AvgStat> avgStats = Stats::FlipsToAvgstats(flips);
		const std::vector<Stats::AvgStat> recommendedFlips = Stats::SortFlipsByRecommendation(avgStats);

		Table recommendation_table({"Item name", "Average profit", "Forecast", "Count"});

		/* Print recommendations until the recommendation_count has been reached */
		int i = 0; 		/* The current recommended item */
		int count = 0; 	/* How many recommendations have been shown */

		/* How many items to recommend in total */
		const int max = FlipUtils::Clamp(recommendedFlips.size(), 1, recommendation_count);

		while (count < max && i < static_cast<int>(recommendedFlips.size()))
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

			/* Calculate extrapolated profit for the item */
			const Flip extrapolated_flip = ExtrapolateFlipData(recommendedFlips[i].name);
			const std::string extrapolated_profit = FlipUtils::RoundBigNumbers(Margin::CalcProfit(extrapolated_flip.buy_price, extrapolated_flip.sold_price, extrapolated_flip.buylimit));

			recommendation_table.add_row({recommendedFlips[i].name, FlipUtils::RoundBigNumbers(recommendedFlips[i].RollingAvgProfit()), extrapolated_profit, std::to_string(recommendedFlips[i].FlipCount())});

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

	Flip ExtrapolateFlipData(const std::string& item_name)
	{
		Flip extrapolated_flip;

		/* Find all finished flips for the item */
		const std::vector<nlohmann::json> item_data = FindFlipsByName(item_name);

		/* If less than two flips were found, return an empty result */
		if (item_data.size() < 2)
			return extrapolated_flip;

		const nlohmann::json& second_last_flip	= item_data[item_data.size() - 2];
		const nlohmann::json& last_flip			= item_data[item_data.size() - 1];

		extrapolated_flip.buy_price		= FlipUtils::LinearExtrapolation(second_last_flip["buy"], last_flip["buy"]);
		extrapolated_flip.sell_price 	= FlipUtils::LinearExtrapolation(second_last_flip["sell"], last_flip["sell"]);
		extrapolated_flip.sold_price 	= FlipUtils::LinearExtrapolation(second_last_flip["sold"], last_flip["sold"]);

		/* Calculate the extrapolated buy limit as the average of all buy limits */
		extrapolated_flip.buylimit = FlipUtils::JsonAverage(item_data, "limit");

		return extrapolated_flip;
	}
}
