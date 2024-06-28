#include "AvgStat.hpp"
#include "Dailygoal.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Random.hpp"
#include "Stats.hpp"
#include "Table.hpp"

#include <algorithm>
#include <iostream>

constexpr char DEFAULT_DATA_FILE[] = "{\"stats\":{\"profit\":0,\"flips_done\":0},\"flips\":[]}\n";
constexpr int RECOMMENDATION_THRESHOLD = 750'000;

namespace flips
{
	/* Declare some functions */
	int find_real_id_with_undone_id(const int undone_id);
	void apply_flip_array();
	void create_default_data_file();
	void load_flip_array();
	void write_json();


	flip::flip()
	{
		item 		= "null";
		buy_price 	= 0;
		sell_price 	= 0;
		sold_price 	= 0;
		buylimit 	= 0;
		done 		= false;
	}

	flip::flip(const nlohmann::json& j)
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

	flip::flip(const std::string& item, const int buy_price, const int sell_price, const int buy_amount, const std::string& account_name)
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

	void flip::sell(const int sell_price)
	{
		this->sell_price 	= sell_price;
		this->done 			= true;
	}

	nlohmann::json flip::to_json() const
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

	void create_default_data_file()
	{
		flip_utils::write_file(data_file, DEFAULT_DATA_FILE);
	}

	void write_json()
	{
		assert(data_file.empty() == false);

		/* Backup the file before writing anything */
		std::filesystem::copy_file(data_file, data_file + "_backup", std::filesystem::copy_options::overwrite_existing);

		flip_utils::write_json_file(json_data, data_file);
	}

	void load_flip_array()
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

	void init()
	{
		assert(data_path.empty() == false);
		assert(data_file.empty() == false);

		if (!std::filesystem::exists(data_path))
			std::filesystem::create_directories(data_path);

		if (!std::filesystem::exists(data_file))
			create_default_data_file();

		/* Read the json data file */
		const std::string json_string = flip_utils::read_file(data_file);
		json_data = nlohmann::json::parse(json_string);

		load_flip_array();
	}

	void apply_flip_array()
	{
		json_data["flips"] = flips;
		write_json();
	}

	void print_stats(const int top_value_count)
	{
		init();

		/* Print top performing flips */
		const std::vector<stats::avg_stat> stats = stats::flips_to_avg_stats(flips);

		if (stats.empty())
		{
			std::cout << "There are no flips to print any statistics about yet!\n";
			return;
		}

		flip_utils::print_title("Stats");
		std::cout << "Total profit: " << flip_utils::round_big_numbers(json_data["stats"]["profit"]) << std::endl;
		std::cout << "Flips done: " << flip_utils::round_big_numbers(json_data["stats"]["flips_done"]) << std::endl;
		std::cout << "\n";

		/* Quit if zero flips done */
		if (json_data["stats"]["flips_done"] == 0)
			return;

		flip_utils::print_title("Top flips by ROI-%");
		const std::vector<stats::avg_stat> topROI = stats::sort_flips_by_roi(stats);

		table flips_by_roi({"Item", "ROI-%", "Average profit"});

		for (int i = 0; i < std::clamp(static_cast<int>(topROI.size()), 0, top_value_count); i++)
			flips_by_roi.add_row({topROI[i].name, std::to_string(topROI[i].avg_roi()), flip_utils::round_big_numbers(topROI[i].avg_profit())});

		flips_by_roi.print();

		std::cout << "\n";

		flip_utils::print_title("Top flips by Profit");
		const std::vector<stats::avg_stat> topProfit = stats::sort_flips_by_profit(stats);

		table flips_by_profit({"Item", "Average profit", "ROI-%"});

		for (int i = 0; i < std::clamp(static_cast<int>(topProfit.size()), 0, top_value_count); i++)
		{
			std::string avgprofit_string = flip_utils::round_big_numbers(topProfit[i].avg_profit());
			flips_by_profit.add_row({topProfit[i].name, avgprofit_string, std::to_string(topProfit[i].avg_roi())});
		}

		flips_by_profit.print();
	}

	void fix_stats()
	{
		init();
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
				total_profit += margin::calc_profit(buy_price, sell_price, limit);
				//total_profit += (sell_price - buy_price) * limit;
			}
		}

		/* Update the stats values */
		json_data["stats"]["profit"] 		= total_profit;
		json_data["stats"]["flips_done"] 	= flip_count;

		/* Update the data file */
		write_json();
	}

	void restore_backup()
	{
		std::cout << "Restoring the backup. Make sure to create a new one if you want to keep it, because the old one is lost" << std::endl;

		/* Check if the backup exists */
		if (std::filesystem::exists(data_file + "_backup"))
			std::filesystem::rename(data_file + "_backup", data_file);
	}

	void list(const std::string& account_filter)
	{
		init();

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

		table ongoing_flips(table_column_names);

		flip_utils::print_title("On-going flips");
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
		const daily_progress daily_progress;
		std::cout << "\n";
		daily_progress.print_progress();
	}

	int find_real_id_with_undone_id(const int undone_id)
	{
		int undone_index = 0;
		int result;
		bool result_found = false;
		for (size_t i = 0; i < flips.size(); i++)
		{
			/* Skip flips that are already done */
			if (flips[i]["done"] == true)
				continue;

			if (undone_index != undone_id)
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
			std::cout << "No items matching ID [" << undone_id << "] were found!" << std::endl;
			return -1;
		}
		else
		{
			return result;
		}
	}

	void add(const flip& flip)
	{
		init();
		flips.push_back(flip.to_json());
		apply_flip_array();
	}

	void cancel(const int ID)
	{
		init();

		/* Mark the flip as cancelled. It will be removed when the flip array
		 * is loaded next time around and saved */
		const int flip_to_cancel = find_real_id_with_undone_id(ID);
		flips[flip_to_cancel]["cancelled"] = true;

		std::cout << "Flip [" << flips[flip_to_cancel]["item"] << "] cancelled!\n";

		apply_flip_array();
	}

	void sell(const int index, int sell_value, int sell_amount)
	{
		init();
		const int result = find_real_id_with_undone_id(index);
		if (result == -1)
			return;

		nlohmann::json& result_flip = flips[result];

		/* Update the flip values */
		if (sell_amount == 0)
			sell_amount = result_flip["limit"];
		else
			result_flip["limit"] = sell_amount;

		result_flip["done"] = true;

		if (sell_value == 0)
			sell_value = result_flip["sell"];

		result_flip["sold"] = sell_value;

		{
			int flips_done = json_data["stats"]["flips_done"];
			++flips_done;
			json_data["stats"]["flips_done"] = flips_done;
		}

		const int buy_price = result_flip["buy"];
		const int profit = margin::calc_profit(buy_price, sell_value, sell_amount);

		long total_profit = json_data["stats"]["profit"];
		total_profit += profit;
		json_data["stats"]["profit"] = total_profit;

		flip_utils::print_title("Flip complete");
		std::cout << "Item: " << result_flip["item"] << std::endl;
		std::cout << "Profit: " << profit << " (" << flip_utils::round_big_numbers(profit) << ")" << std::endl;
		std::cout << "Total profit so far: " << total_profit << " (" << flip_utils::round_big_numbers(total_profit) << ")" << std::endl;

		/* Handle daily progress */
		{
			daily_progress daily_progress;
			daily_progress.add_progress(profit);
			std::cout << "\n";
			daily_progress.print_progress();
		}

		/* Update the json file */
		apply_flip_array();
	}

	std::vector<nlohmann::json> find_flips_by_name(const std::string& item_name)
	{
		std::vector<nlohmann::json> result;

		/* Quit if zero flips done */
		const int flip_count = flips::json_data["stats"]["flips_done"];
		if (flip_count == 0)
			return result;

		std::copy_if(flips::json_data["flips"].begin(), flips::json_data["flips"].end(), std::back_inserter(result),
			[item_name](const nlohmann::json j) {
				return j["item"] == item_name && j["done"] == true;
			}
		);

		return result;
	}

	void filter_name(const std::string& name)
	{
		init();
		std::cout << "Filter: " << name << std::endl;

		/* Quit if zero flips done */
		const int flip_count = flips::json_data["stats"]["flips_done"];
		if (flip_count == 0)
			return;

		std::vector<nlohmann::json> found_flips = find_flips_by_name(name);

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
			flip flip(found_flips[i]);

			const std::string buy_price = flip_utils::round_big_numbers(flip.buy_price);
			const std::string sell_price = flip_utils::round_big_numbers(flip.sold_price);
			//int profit = (flip.sold_price - flip.buy_price) * flip.buylimit;
			const int profit = margin::calc_profit(flip.buy_price, flip.sold_price, flip.buylimit);
			total_profit += profit;
			const std::string profit_text = flip_utils::round_big_numbers(profit);
			const std::string item_count = std::to_string(flip.buylimit);

			std::cout << "| " << buy_price << std::setw(14 - buy_price.length()) << " | " <<
				sell_price << std::setw(14 - sell_price.length())	<< " | " <<
				item_count << std::setw(10 - item_count.length()) 	<< " | " <<
				profit_text << std::setw(14 - profit_text.length()) << " |\n";
		}

		std::cout << "+-------------+-------------+---------+-------------+\n";

		/* Calculate average profit */
		std::cout << "\n\033[33mAverage profit: " << flip_utils::round_big_numbers((double)total_profit / found_flips.size()) << "\033[0m" << std::endl;
		std::cout << "\033[32mTotal profit:   " << flip_utils::round_big_numbers((double)total_profit) << "\033[0m" << std::endl;

		/** Calculate median buy and sell prices **/
		const int middle_index = std::floor(found_flips.size() / 2.0);
		std::vector<nlohmann::json> sorted_list = found_flips;

		std::cout << "\n";

		/* Sort by buying price */
		flip_utils::json_sort(sorted_list, "buy");
		std::cout << "Median buy price:  " << sorted_list.at(middle_index)["buy"] << "\n";

		/* Sort by selling price */
		flip_utils::json_sort(sorted_list, "sold");
		std::cout << "Median sell price: " << sorted_list.at(middle_index)["sold"] << "\n";

		std::cout << "\n";

		/** Calculate average buying and selling prices **/
		std::cout << "\033[37mAverage buy price:  " << flip_utils::json_average(found_flips, "buy") << "\033[0m\n";
		std::cout << "\033[37mAverage sell price: " << flip_utils::json_average(found_flips, "sold") << "\033[0m\n";

		std::cout << "\n";

		/** Find min and max buy/sell prices **/
		std::cout << "\033[34mMin buy price: " << flip_utils::json_min_int(sorted_list, "buy") << "\033[0m\n";
		std::cout << "\033[34mMax buy price: " << flip_utils::json_max_int(sorted_list, "buy") << "\033[0m\n";

		std::cout << "\n";

		std::cout << "\033[35mMin sell price: " << flip_utils::json_min_int(sorted_list, "sold") << "\033[0m\n";
		std::cout << "\033[35mMax sell price: " << flip_utils::json_max_int(sorted_list, "sold") << "\033[0m\n";
	}

	void filter_count(const int flip_count)
	{
		init();

		/* Don't do anything if there are no flips in the db */
		if (flips.size() == 0)
			return;

		/* Don't do anything if the flip count given is dumb */
		if (flip_count < 1)
			return;

		std::vector<stats::avg_stat> avgStats = stats::flips_to_avg_stats(flips);

		for (size_t i = 0; i < avgStats.size(); i++)
		{
			if (avgStats[i].flip_count() <= flip_count)
				std::cout << avgStats[i].name << std::endl;
		}
	}

	bool flip_recommendations()
	{
		init();

		if (flips.size() < 10)
			return false;

		/* Read in the item recommendation blacklist */
		const std::unordered_set<std::string> item_blacklist = flip_utils::read_file_items(item_blacklist_file);

		flip_utils::print_title("Recommended flips");

		const std::vector<stats::avg_stat> avgStats = stats::flips_to_avg_stats(flips);
		const std::vector<stats::avg_stat> recommendedFlips = stats::sort_flips_by_recommendation(avgStats);

		table recommendation_table({"Item name", "Average profit", "Count"});

		/* Print recommendations until the recommendation_count has been reached */
		int i = 0; 		/* The current recommended item */
		int count = 0; 	/* How many recommendations have been shown */

		/* How many items to recommend in total */
		const int max = std::clamp(static_cast<int>(recommendedFlips.size()), 1, recommendation_count);

		while (count < max && i < static_cast<int>(recommendedFlips.size()))
		{
			/* Skip items that are blacklisted */
			if (item_blacklist.contains(recommendedFlips[i].name))
			{
				++i;
				continue;
			}

			/* Skip items with average profit below a certain threshold */
			if (recommendedFlips[i].avg_profit() < RECOMMENDATION_THRESHOLD)
			{
				++i;
				continue;
			}

			recommendation_table.add_row({recommendedFlips[i].name, flip_utils::round_big_numbers(recommendedFlips[i].rolling_avg_profit()), std::to_string(recommendedFlips[i].flip_count())});

			++i;
			++count;
		}

		if (count == 0)
		{
			std::cout << "Couldn't find any flips to recommend...\n";
			return true;
		}

		recommendation_table.print();

		// Pick some random flips
		const int max_random_count = std::clamp(random_flip_count, 0, static_cast<int>(recommendedFlips.size()) - max);

		if (max_random_count > 0)
		{
			std::cout << "\n";
			recommendation_table.clear();
			flip_utils::print_title("Random flips");
			class random rng;

			for (int j = 0; j < max_random_count; ++j)
			{
				const int index = rng.range(max, static_cast<int>(recommendedFlips.size() - 1));
				recommendation_table.add_row({recommendedFlips[index].name, flip_utils::round_big_numbers(recommendedFlips[index].rolling_avg_profit()), std::to_string(recommendedFlips[index].flip_count())});
			}

			recommendation_table.print();
		}


		return true;
	}
}
