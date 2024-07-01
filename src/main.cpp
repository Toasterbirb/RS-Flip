#define DOCTEST_CONFIG_IMPLEMENT

#include "DB.hpp"
#include "Dailygoal.hpp"
#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Types.hpp"

#include <clipp.h>
#include <doctest/doctest.h>

enum class mode
{
	tips, calc, flip, sold, cancel, list, filtering, stats, repair, help, test
};

struct options
{
	u64 buy_price{};
	u64 sell_price{};
	u32 item_count{};

	u16 id{};
	std::string item_name;
	std::string account;

	u32 flip_count{};
	u32 result_count = 10;

	struct tips
	{
		i64 profit_threshold = 750'000;
	};

	tips tips;
};

int main(int argc, char** argv)
{
	mode selected_mode = mode::tips;
	options options;

	auto tips = (
		clipp::command("tips").set(selected_mode, mode::tips) % "mode",
		(clipp::option("-t") & clipp::value("profit", options.tips.profit_threshold)) % "profit threshold"
	) % "recommend flips based on past flipping data";

	auto calc = (
		clipp::command("calc").set(selected_mode, mode::calc) % "mode",
		(clipp::option("-b").required(true) & clipp::value("price").set(options.buy_price)) % "insta buy price",
		(clipp::option("-s").required(true) & clipp::value("price").set(options.sell_price)) % "insta sell price",
		(clipp::option("-l").required(true) & clipp::value("limit").set(options.item_count)) % "buy limit for the item"
	) % "calculate the margin for an item and possible profits";

	auto flip = (
		clipp::command("add").set(selected_mode, mode::flip) % "mode",
		(clipp::option("-i").required(true) & clipp::value("name").set(options.item_name)) % "item name",
		(clipp::option("-b").required(true) & clipp::value("price").set(options.buy_price)) % "buying price",
		(clipp::option("-s").required(true) & clipp::value("price").set(options.sell_price)) % "assumed future selling price",
		(clipp::option("-l").required(true) & clipp::value("limit").set(options.item_count)) % "item count to buy (usually the buy limit or slightly below)",
		(clipp::option("-a").required(false) & clipp::value("account").set(options.account)) % "the name of the account used for the flip"
	) % "add a flip to the database";

	auto sold = (
		clipp::command("sold").set(selected_mode, mode::sold) % "mode",
		(clipp::option("-i").required(true) & clipp::value("id").set(options.id)) % "the id number can be found with the 'list' command",
		(clipp::option("-s").required(false) & clipp::value("price").set(options.sell_price)) % "final selling price",
		(clipp::option("-l").required(false) & clipp::value("count").set(options.item_count)) % "final amount of items sold"
	) % "finish an on-going flip";

	auto cancel = (
		clipp::command("cancel").set(selected_mode, mode::cancel) % "mode",
		clipp::value("id").set(options.id) % "the id of the flip to cancel"
	) % "cancels an on-going flip and removes it from the database";

	auto list = (
		clipp::command("list").set(selected_mode, mode::list) % "mode",
		clipp::value("account").set(options.account).required(false) % "list only flips made with this account"
	) % "list all on-going flips with their ids, buy and sell values";

	auto filtering = (
		clipp::command("filter").set(selected_mode, mode::filtering) % "mode",
		clipp::one_of(
			(clipp::option("-i") & clipp::value("name").set(options.item_name)) % "find stats for a specific item",
			(clipp::option("-c") & clipp::option("count").set(options.flip_count)) % "find flips that have been done count <= times"
		)
	) % "look for items with filters";

	auto stats = (
		clipp::command("stats").set(selected_mode, mode::stats) % "mode",
		(clipp::option("-c") & clipp::value("count").set(options.result_count)) % "set the amount of values to show"
	) % "print out profit statistics";

	auto repair = (
		clipp::command("repair").set(selected_mode, mode::repair) % "attempts to repair the statistics from the flip data in-case of some bug"
	);

	auto help = (
		clipp::command("help").set(selected_mode, mode::help) % "show help"
	);

	auto test (
		clipp::command("test").set(selected_mode, mode::test) % "run unit tests"
	);

	auto cli = (
		( tips | calc | flip | sold | cancel | list | filtering | stats | repair | help | test )
	);

	if (!clipp::parse(argc, argv, cli))
	{
		std::cout << "Invalid arguments were provided. Please check 'rs-flip --help'\n";
		return 1;
	}

	db db;
	daily_progress daily_progress;

	switch (selected_mode)
	{
		case mode::tips:
			flips::flip_recommendations(db, options.tips.profit_threshold);
			return 0;

		case mode::calc:
			margin::print_flip_estimation(options.buy_price, options.sell_price, options.item_count);
			return 0;

		case mode::flip:
		{
			const flips::flip flip_obj(options.item_name,
					options.buy_price,
					options.sell_price,
					options.item_count,
					options.account);

			std::cout << "Adding item: " << options.item_name << '\n'
					<< "Buy price: " << flip_utils::round_big_numbers(options.buy_price) << '\n'
					<< "Sell price: " << flip_utils::round_big_numbers(options.sell_price) << '\n'
					<< "Buy count: " << options.item_count << "\n\n";

			i32 profit = margin::calc_profit(flip_obj);
			std::cout << "Estimated profit: " << flip_utils::round_big_numbers(profit) << '\n';

			db.add_flip(flip_obj);
			break;
		}

		case mode::sold:
			flips::sell(db, daily_progress, options.id, options.sell_price, options.item_count);
			break;

		case mode::cancel:
			flips::cancel(db, options.id);
			break;

		case mode::list:
			flips::list(db, daily_progress, options.account);
			break;

		case mode::filtering:
		{
			if (!options.item_name.empty())
				flips::filter_name(db, options.item_name);
			else if (options.flip_count > 0)
				flips::filter_count(db, options.flip_count);
			else
				std::cout << "Not really sure how to filter because no filters were defined\n";
			return 0;
		}

		case mode::stats:
			flips::print_stats(db, options.result_count);
			return 0;
			break;

		case mode::repair:
			flips::fix_stats(db);
			break;

		case mode::help:
		{
			auto fmt = clipp::doc_formatting{}.doc_column(30);
			std::cout << clipp::make_man_page(cli, "rs-flip", fmt);
			return 0;
		}

		case mode::test:
		{
			doctest::Context context;
			return context.run();
		}
	}

	/* Update the database files
	 * If no update is required, exit early */
	db.write();
	daily_progress.write();
}
