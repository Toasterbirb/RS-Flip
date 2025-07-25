#include "AvgStat.hpp"
#include "Dailygoal.hpp"
#include "FilePaths.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Random.hpp"
#include "Stats.hpp"
#include "Table.hpp"
#include "Types.hpp"

#include <algorithm>
#include <doctest/doctest.h>
#include <format>
#include <iostream>

namespace flips
{
	flip::flip()
	:item("null"),
	 buy_price(0),
	 sell_price(0),
	 sold_price(0),
	 buylimit(0),
	 done(false)
	{}

	flip::flip(const nlohmann::json& j)
	:item(j["item"]),
	 buy_price(j["buy"]),
	 sell_price(j["sell"]),
	 sold_price(j["sold"]),
	 buylimit(j["limit"]),
	 cancelled(j["cancelled"]),
	 done(j["done"])
	{
		/* Asserts for checking if the json object has all of the required keys */
		assert(j.contains("item"));
		assert(j.contains("buy"));
		assert(j.contains("sell"));
		assert(j.contains("sold"));
		assert(j.contains("limit"));
		assert(j.contains("cancelled"));
		assert(j.contains("done"));

		/* Assume "main" as the account if its not specified */
		if (!j.contains("account"))
			account = "main";
		else
			account = j["account"];
	}

	flip::flip(const std::string& item, const i32 buy_price, const i32 sell_price, const i32 buy_amount, const std::string& account_name)
	:item(item),
	 buy_price(buy_price),
	 sell_price(sell_price),
	 sold_price(0),
	 buylimit(buy_amount),
	 cancelled(false),
	 done(false),
	 account(account_name)
	{}

	void flip::sell(const i32 sell_price)
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

	void print_stats(const db& db, const i32 top_value_count)
	{
		/* Print top performing flips */
		const std::vector<stats::avg_stat> stats = db.get_flip_avg_stats();

		if (stats.empty())
		{
			std::cout << "There are no flips to print any statistics about yet!\n";
			return;
		}

		flip_utils::print_title("Stats");
		std::cout << "Total profit: " << flip_utils::round_big_numbers(db.get_stat(db::stat_key::profit)) << '\n';
		std::cout << "Flips done: " << flip_utils::round_big_numbers(db.get_stat(db::stat_key::flips_done)) << '\n';
		std::cout << "\n";

		/* Quit if zero flips done */
		if (db.get_stat(db::stat_key::flips_done) == 0)
			return;

		flip_utils::print_title("Top flips by ROI-%");
		const std::vector<stats::avg_stat> topROI = stats::sort_flips_by_roi(stats);

		table flips_by_roi({"Item", "ROI-%", "Average profit"});

		for (i32 i = 0; i < std::clamp(static_cast<int>(topROI.size()), 0, top_value_count); i++)
			flips_by_roi.add_row({topROI[i].name, std::to_string(topROI[i].avg_roi()), flip_utils::round_big_numbers(topROI[i].avg_profit())});

		flips_by_roi.print();

		std::cout << "\n";

		flip_utils::print_title("Top flips by Profit");
		const std::vector<stats::avg_stat> topProfit = stats::sort_flips_by_profit(stats);

		table flips_by_profit({"Item", "Average profit", "ROI-%"});

		for (i32 i = 0; i < std::clamp(static_cast<int>(topProfit.size()), 0, top_value_count); i++)
		{
			std::string avgprofit_string = flip_utils::round_big_numbers(topProfit[i].avg_profit());
			flips_by_profit.add_row({topProfit[i].name, avgprofit_string, std::to_string(topProfit[i].avg_roi())});
		}

		flips_by_profit.print();
	}

	void fix_stats(db& db)
	{
		std::cout << "Recalculating statistics...\n";

		/* Get the old statistics so that they can be compared to the newly
		   calculated results */
		const i64 old_total_profit = db.get_stat(db::stat_key::profit);
		const i64 old_flip_count = db.get_stat(db::stat_key::flips_done);

		i64 total_profit = 0;
		i64 flip_count = 0;

		const auto warning_prefix = [&db](const size_t flip_index) -> std::string
		{
			return std::format("Warning: flip {} [{}] ", flip_index, db.get_flip<std::string>(flip_index, db::flip_key::item));
		};

		const auto action_postfix = [](const std::string& action) -> std::string
		{
			return std::format(" -> {}\n", action);
		};

		const auto cancel_flip = [&db, &action_postfix](const size_t flip_index)
		{
			std::cout << action_postfix("Marking the flip as cancelled");
			db.set_flip<bool>(flip_index, db::flip_key::done, false);
			db.set_flip<bool>(flip_index, db::flip_key::cancelled, true);
		};

		for (size_t i = 0; i < db.total_flip_count(); i++)
		{
			/* Warn about problematic flip data */

			if (db.get_flip<bool>(i, db::flip_key::cancelled) && db.get_flip<bool>(i, db::flip_key::done))
			{
				std::cout << warning_prefix(i)
					<< "is cancelled and done at the same time\n";
				cancel_flip(i);
			}

			if (db.get_flip<u64>(i, db::flip_key::limit) == 0)
			{
				std::cout << warning_prefix(i)
					<< "has a buy limit of zero\n";
				cancel_flip(i);
			}

			if (db.get_flip<u64>(i, db::flip_key::buy) == 0)
			{
				std::cout << warning_prefix(i)
					<< "has a buy price of zero\n";
				cancel_flip(i);
			}

			/* Check if the flip is done */
			if (db.get_flip<bool>(i, db::flip_key::done) == false)
				continue;

			/* Perform more checks */
			if (db.get_flip<u64>(i, db::flip_key::sell) == 0)
			{
				std::cout << warning_prefix(i)
					<< "has a sell price of zero\n";
				cancel_flip(i);
			}

			if (db.get_flip<u64>(i, db::flip_key::sold) == 0)
			{
				std::cout << warning_prefix(i)
					<< "has a sold price of zero\n";
				cancel_flip(i);
			}


			/* Skip cancelled flips */
			if (db.get_flip<bool>(i, db::flip_key::cancelled) == true)
				continue;

			flip_count++;

			/* Flip is done, but the sold price is missing */
			if (db.get_flip<u64>(i, db::flip_key::sold) == 0)
				db.set_flip(i, db::flip_key::sold, db.get_flip<u64>(i, db::flip_key::sell));

			/* Calculate the profit */
			i32 buy_price 	= db.get_flip<u64>(i, db::flip_key::buy);
			i32 sell_price 	= db.get_flip<u64>(i, db::flip_key::sold);
			i32 limit 		= db.get_flip<u64>(i, db::flip_key::limit);
			total_profit += margin::calc_profit(buy_price, sell_price, limit);
		}

		/* Update the stats values */
		db.set_stat(db::stat_key::profit, total_profit);
		db.set_stat(db::stat_key::flips_done, flip_count);

		/* Print changes */
		if (old_total_profit != total_profit)
			std::cout << "Total profit: "
				<< flip_utils::round_big_numbers(old_total_profit)
				<< " -> "
				<< flip_utils::round_big_numbers(total_profit)
				<< '\n';

		if (old_flip_count != flip_count)
			std::cout << "Flip count: " << old_flip_count << " -> " << flip_count << '\n';
	}

	void list(const db& db, const daily_progress& daily_progress, const std::string& account_filter)
	{
		std::vector<u32> undone_flips;

		for (size_t i = 0; i < db.total_flip_count(); i++)
		{
			/* Check if the flip is done yet */
			if (db.get_flip<bool>(i, db::flip_key::done))
				continue;

			/* Check if the flip has been cancelled */
			if (db.get_flip<bool>(i, db::flip_key::cancelled))
				continue;

			undone_flips.push_back(i);
		}

		if (undone_flips.empty())
		{
			std::cout 	<< "No active flips were found...\n"
						<< "You might wanna go to the Grand Exchange and start some.\n";
			return;
		}

		/* Keep track of the accounts listed. If only main account was used, we can
		 * skip printing the Account column in the flip list command */
		bool flips_only_with_main = true;
		for (size_t i = 0; i < undone_flips.size(); ++i)
		{
			/* If account other than main was used, print the account
			 * column to the table */
			if (db.get_flip<std::string>(undone_flips[i], db::flip_key::account) != "main")
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
			const std::string& flip_name	= db.get_flip<std::string>(undone_flips[i], db::flip_key::item);
			const u32 flip_item_count		= db.get_flip<u32>(undone_flips[i], db::flip_key::limit);
			const u64 flip_buy				= db.get_flip<u64>(undone_flips[i], db::flip_key::buy);
			const u64 flip_sell				= db.get_flip<u64>(undone_flips[i], db::flip_key::sell);
			const std::string& account		= db.get_flip<std::string>(undone_flips[i], db::flip_key::account);

			/* The minimum price and count for an item is 1 */
			assert(!flip_name.empty());
			assert(flip_item_count > 0);
			assert(flip_buy > 0);
			assert(flip_sell > 0);
			assert(!account.empty());

			/* If using account filtering, skip rows with non-matching accounts */
			if (!account_filter.empty() && account != account_filter)
				continue;

			std::vector<std::string> data_row = {std::to_string(i), flip_name, std::to_string(flip_item_count), std::to_string(flip_buy), std::to_string(flip_sell)};

			/* Add Account column if other accounts than main were also used */
			if (flips_only_with_main == false)
				data_row.push_back(account);

			ongoing_flips.add_row(data_row);
		}

		// Only print the table if there are flips to print
		if (ongoing_flips.row_count() > 0)
			ongoing_flips.print();
		else
			std::cout << "There are no on-going flips on account '" << account_filter << "'\n";

		/* Print out daily goal */
		std::cout << "\n";
		daily_progress.print_progress();
	}

	i32 find_real_id_with_undone_id(const db& db, const u32 undone_id)
	{
		u32 undone_index = 0;
		i32 result;
		bool result_found = false;
		for (size_t i = 0; i < db.total_flip_count(); i++)
		{
			/* Skip flips that are already done */
			if (db.get_flip<bool>(i, db::flip_key::done) == true)
				continue;

			/* Skip cancelled flips */
			if (db.get_flip<bool>(i, db::flip_key::cancelled) == true)
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
			std::cout << "No items matching ID [" << undone_id << "] were found!\n";
			return -1;
		}

		return result;
	}

	void cancel(db& db, const i32 ID)
	{
		/* Mark the flip as cancelled. It will be removed when the flip array
		 * is loaded next time around and saved */
		const i32 flip_to_cancel = find_real_id_with_undone_id(db, ID);

		// flip_to_cancel will be -1 if the ID was bogus
		if (flip_to_cancel < 0)
			return;

		assert(!db.get_flip<bool>(flip_to_cancel, db::flip_key::done));

		db.set_flip(flip_to_cancel, db::flip_key::cancelled, true);

		std::cout << "Flip [" << db.get_flip<std::string>(flip_to_cancel, db::flip_key::item) << "] cancelled!\n";
	}

	void update(db& db, const i32 ID, u32 buy_price, u32 sell_price, u32 buy_amount, std::string account_name)
	{
		/* Since the given ID is the ID from the flip list, we'll need to convert it into a flip index */
		const i32 flip_index = find_real_id_with_undone_id(db, ID);

		/* If no flips were found, bail out */
		if (flip_index == -1)
			return;

		/* Update variables that have been changed by the user */

		std::cout << "Updating item [" << db.get_flip<std::string>(flip_index, db::flip_key::item) << "]\n";

		if (buy_price != 0)
		{
			std::cout << "Buy price: " << db.get_flip<u32>(flip_index, db::flip_key::buy) << " -> " << buy_price << '\n';
			db.set_flip(flip_index, db::flip_key::buy, buy_price);
		}

		if (sell_price != 0)
		{
			std::cout << "Sell price: " << db.get_flip<u32>(flip_index, db::flip_key::sell) << " -> " << sell_price << '\n';
			db.set_flip(flip_index, db::flip_key::sell, sell_price);
		}

		if (buy_amount != 0)
		{
			std::cout << "Item count: " << db.get_flip<u32>(flip_index, db::flip_key::limit) << " -> " << buy_amount << '\n';
			db.set_flip(flip_index, db::flip_key::limit, buy_amount);
		}

		if (!account_name.empty())
		{
			std::cout << "Account: " << db.get_flip<std::string>(flip_index, db::flip_key::account) << " -> " << account_name << '\n';
			db.set_flip(flip_index, db::flip_key::account, account_name);
		}
	}

	void sell(db& db, daily_progress& daily_progress, const i32 index, i32 sell_value, i32 sell_amount)
	{
		const i32 flip_index = find_real_id_with_undone_id(db, index);
		if (flip_index == -1)
			return;

		/* Update the flip values */
		if (sell_amount == 0)
			sell_amount = db.get_flip<u32>(flip_index, db::flip_key::limit);
		else
			db.set_flip(flip_index, db::flip_key::limit, sell_amount);

		db.set_flip(flip_index, db::flip_key::done, true);

		if (sell_value == 0)
			sell_value = db.get_flip<u64>(flip_index, db::flip_key::sell);

		db.set_flip(flip_index, db::flip_key::sold, sell_value);

		/* Increment the total flip counter by one */
		db.set_stat(db::stat_key::flips_done, db.get_stat(db::stat_key::flips_done) + 1);

		const i32 profit = margin::calc_profit(db.get_flip<u32>(flip_index, db::flip_key::buy), sell_value, sell_amount);

		i64 total_profit = db.get_stat(db::stat_key::profit);
		total_profit += profit;
		db.set_stat(db::stat_key::profit, total_profit);

		flip_utils::print_title("Flip complete");
		std::cout << "Item: " << db.get_flip<std::string>(flip_index, db::flip_key::item) << '\n'
				<< "Profit: " << profit << " (" << flip_utils::round_big_numbers(profit) << ")\n"
				<< "Total profit so far: " << total_profit << " (" << flip_utils::round_big_numbers(total_profit) << ")\n";

		/* Handle daily progress */
		daily_progress.add_progress(profit);
		std::cout << "\n";
		daily_progress.print_progress();
	}

	void filter_name(const db& db, const std::string& name)
	{
		std::cout << "Filter: " << name << '\n';

		/* Quit if zero flips done */
		if (db.get_stat(db::stat_key::flips_done) == 0)
			return;

		std::vector<u32> found_flips = db.find_flips_by_name(name);

		std::cout << "Results: " << found_flips.size() << '\n';
		if (found_flips.size() == 0)
			return;

		/* List out the flips */
		std::cout	<< "┌─────────────┬─────────────┬─────────┬─────────────┐\n"
					<< "│ Buy         │ Sell        │ Count   │ Profit      │\n"
					<< "├─────────────┼─────────────┼─────────┼─────────────┤\n";

		constexpr u8 buy_cell_width		= 11;
		constexpr u8 sell_cell_width	= 11;
		constexpr u8 count_cell_width	= 7;
		constexpr u8 profit_cell_width	= 11;

		i64 total_profit = 0;
		for (size_t i = 0; i < found_flips.size(); i++)
		{
			flip flip = db.get_flip_obj(found_flips.at(i));

			const std::string buy_price = flip_utils::round_big_numbers(flip.buy_price);
			const std::string sell_price = flip_utils::round_big_numbers(flip.sold_price);

			const i64 profit = margin::calc_profit(flip.buy_price, flip.sold_price, flip.buylimit);
			total_profit += profit;

			const std::string profit_text = flip_utils::round_big_numbers(profit);
			const std::string item_count = std::to_string(flip.buylimit);

			std::cout << "│ "
				<< std::setw(buy_cell_width) << buy_price << " │ "
				<< std::setw(sell_cell_width) << sell_price << " │ "
				<< std::setw(count_cell_width) << item_count << " │ "
				<< std::setw(profit_cell_width) << profit_text << " │\n";
		}

		std::cout << "└─────────────┴─────────────┴─────────┴─────────────┘\n";

		/* Calculate average profit */
		std::cout << "\n\033[33mAverage profit: " << flip_utils::round_big_numbers((double)total_profit / found_flips.size()) << "\033[0m\n";
		std::cout << "\033[32mTotal profit:   " << flip_utils::round_big_numbers((double)total_profit) << "\033[0m\n";

		std::cout << "\n";

		/** Calculate average buying and selling prices **/
		std::cout << "\033[37mAverage buy price:  " << db.get_flip_average<u64>(found_flips, db::flip_key::buy) << "\033[0m\n";
		std::cout << "\033[37mAverage sell price: " << db.get_flip_average<u64>(found_flips, db::flip_key::sold) << "\033[0m\n";

		std::cout << "\n";

		/** Find min and max buy/sell prices **/
		std::cout << "\033[34mMin buy price: " << db.get_flip_min<u64>(found_flips, db::flip_key::buy) << "\033[0m\n";
		std::cout << "\033[34mMax buy price: " << db.get_flip_max<u64>(found_flips, db::flip_key::buy) << "\033[0m\n";

		std::cout << "\n";

		std::cout << "\033[35mMin sell price: " << db.get_flip_min<u64>(found_flips, db::flip_key::sold) << "\033[0m\n";
		std::cout << "\033[35mMax sell price: " << db.get_flip_max<u64>(found_flips, db::flip_key::sold) << "\033[0m\n";
	}

	void filter_count(const db& db, const u32 flip_count)
	{
		/* Don't do anything if there are no flips in the db */
		if (db.total_flip_count() == 0)
			return;

		/* Don't do anything if the flip count given is dumb */
		if (flip_count < 1)
			return;

		std::vector<u32> flips = db.find_flips_by_count(flip_count);
		for (const u32 flip : flips)
			std::cout << db.get_flip<std::string>(flip, db::flip_key::item) << '\n';
	}

	bool flip_recommendations(const db& db, const tip_config& config)
	{
		if (config.max_result_count < 1)
		{
			std::cout << "A recommendation count of at least 1 is required\n";
			return false;
		}

		if (db.total_flip_count() < 10)
			return false;

		// set the recommendation algorithm if the user wants to change it
		if (config.recommendation_algorithm)
			stats::avg_stat::set_recommendation_algorithm(config.recommendation_algorithm);

		/* Read in the item recommendation blacklist */
		const std::unordered_set<std::string> item_blacklist = flip_utils::read_file_items(file_paths::item_blacklist_file);

		const std::vector<stats::avg_stat> recommended_flips = stats::sort_flips_by_recommendation(db.get_flip_avg_stats());

		const std::vector<std::string> recommendation_table_column_names = { "Item name", "Average profit", "Count" };
		table recommendation_table(recommendation_table_column_names);

		/* Print recommendations until the recommendation_count has been reached */
		static constexpr u32  rolling_avg_profit_window_size = 10;

		/* How many items to recommend in total */
		const size_t max = std::clamp(static_cast<u32>(recommended_flips.size()), 1U, config.max_result_count);

		const auto should_flip_be_skipped = [&item_blacklist, &config](const stats::avg_stat& flip) -> bool
		{
			const bool is_below_threshold = flip.rolling_avg_profit(rolling_avg_profit_window_size) < config.profit_threshold;
			const bool is_blacklisted = item_blacklist.contains(flip.name) && config.use_blacklist;
			const bool not_enough_data = flip.flip_count() < 2;
			return is_below_threshold || is_blacklisted || not_enough_data;
		};

		// If we are printing in ge-inspector format, build the string
		// only and don't build the recommendation table
		if (config.ge_inspector_format)
		{
			u32 count{};
			std::string ge_inspector_format_str;

			for (const stats::avg_stat& flip : recommended_flips)
			{
				// Check if we have reached the requested amount of recommendations
				if (count >= max)
					break;

				if (should_flip_be_skipped(flip))
					continue;

				ge_inspector_format_str += flip.name + ';';
				++count;
			}

			// Remove the last semicolon
			ge_inspector_format_str.erase(ge_inspector_format_str.end() - 1);

			std::cout << ge_inspector_format_str << '\n';

			return true;
		}

		// Build the recommendation table
		for (size_t i = 0; i < recommended_flips.size() && recommendation_table.row_count() < max; ++i)
		{
			if (should_flip_be_skipped(recommended_flips[i]))
				continue;

			recommendation_table.add_row({
				recommended_flips[i].name,
				flip_utils::round_big_numbers(recommended_flips[i].rolling_avg_profit(rolling_avg_profit_window_size)),
				std::to_string(recommended_flips[i].flip_count())
			});
		}

		// If we are printing the full table instead, check if there's anything to print
		if (recommendation_table.row_count() == 0)
		{
			std::cout << "Couldn't find any flips to recommend...\n";
			return true;
		}

		flip_utils::print_title("Recommended flips");
		recommendation_table.print();

		// Pick some random flips
		const size_t max_random_count = config.max_random_flip_count < recommended_flips.size() - recommendation_table.row_count()
			? config.max_random_flip_count
			: recommended_flips.size() - recommendation_table.row_count();

		// If there are no random flips to print, return early
		if (max_random_count == 0 || max > recommendation_table.row_count())
			return true;

		std::cout << "\n";

		flip_utils::print_title("Random flips");
		class random rng;

		table random_table(recommendation_table_column_names);

		for (u32 j = 0; j < max_random_count; ++j)
		{
			const u32 index = rng.range(max, recommended_flips.size() - 1);
			random_table.add_row({
				recommended_flips[index].name,
				flip_utils::round_big_numbers(recommended_flips[index].rolling_avg_profit(rolling_avg_profit_window_size)),
				std::to_string(recommended_flips[index].flip_count())
			});
		}

		random_table.print();

		return true;
	}
}
