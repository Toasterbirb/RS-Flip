#include "DB.hpp"
#include "Flips.hpp"

#include <array>
#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

TEST_CASE("Misc. flipping unit tests")
{
	flips::flip flip_a;
	flip_a.account = "main";
	flip_a.buy_price = 1468;
	flip_a.cancelled = false;
	flip_a.done = false;
	flip_a.item = "Yew shieldbow";
	flip_a.buylimit = 4950;
	flip_a.sell_price = 1649;
	flip_a.sold_price = 0;

	flips::flip flip_b;
	flip_b.account = "main";
	flip_b.buy_price = 5000;
	flip_b.cancelled = false;
	flip_b.done = false;
	flip_b.item = "Adamant bar";
	flip_b.buylimit = 9950;
	flip_b.sell_price = 5200;
	flip_b.sold_price = 0;

	flips::flip flip_c;
	flip_c.account = "main";
	flip_c.buy_price = 12503;
	flip_c.cancelled = false;
	flip_c.done = true;
	flip_c.item = "Dagannoth bones";
	flip_c.buylimit = 3723;
	flip_c.sell_price = 13443;
	flip_c.sold_price = 13443;

	flips::flip flip_d;
	flip_d.account = "alt1";
	flip_d.buy_price = 6301;
	flip_d.cancelled = false;
	flip_d.done = false;
	flip_d.item = "Prayer potion (3)";
	flip_d.buylimit = 950;
	flip_d.sell_price = 19997;
	flip_d.sold_price = 0;

	constexpr i64 original_flips_done = 8;
	constexpr i64 original_total_profit = 200;

	nlohmann::json db_json;
	db_json["stats"]["flips_done"] = original_flips_done;
	db_json["stats"]["profit"] = original_total_profit;

	db db(db_json);
	daily_progress daily_progress;
	const i32 original_daily_progress = daily_progress.current_progress();

	db.add_flip(flip_a);
	db.add_flip(flip_b);
	db.add_flip(flip_c);
	db.add_flip(flip_d);

	constexpr i32 flip_count = 4;

	const std::array<bool, flip_count> original_done_states = {
		flip_a.done,
		flip_b.done,
		flip_c.done,
		flip_d.done
	};

	const std::array<bool, flip_count> original_cancel_states = {
		flip_a.cancelled,
		flip_b.cancelled,
		flip_c.cancelled,
		flip_d.cancelled
	};

	const std::array<flips::flip, flip_count> original_flips = {
		flip_a, flip_b, flip_c, flip_d
	};

	CHECK(db.total_flip_count() == flip_count);

	const auto ensure_original_done_states = [&](const i32 changed_id = -1)
	{
		for (i32 i = 0; i < flip_count; ++i)
		{
			if (i == changed_id)
				CHECK_FALSE(db.get_flip<bool>(i, db::flip_key::done) == original_done_states.at(i));
			else
				CHECK(db.get_flip<bool>(i, db::flip_key::done) == original_done_states.at(i));
		}
	};

	const auto ensure_original_cancel_states = [&](const i32 changed_id = -1)
	{
		for (i32 i = 0; i < flip_count; ++i)
		{
			if (i == changed_id)
				CHECK_FALSE(db.get_flip<bool>(i, db::flip_key::cancelled) == original_cancel_states.at(i));
			else
				CHECK(db.get_flip<bool>(i, db::flip_key::cancelled) == original_cancel_states.at(i));
		}
	};

	const auto ensure_original_flip_states = [&](const i32 id_to_ignore = -1)
	{
		for (i32 i = 0; i < flip_count; ++i)
		{
			if (i == id_to_ignore)
				continue;

			CHECK(db.get_flip<std::string>(i, db::flip_key::account) == original_flips.at(i).account);
			CHECK(db.get_flip<i32>(i, db::flip_key::buy) == original_flips.at(i).buy_price);
			CHECK(db.get_flip<bool>(i, db::flip_key::cancelled) == original_flips.at(i).cancelled);
			CHECK(db.get_flip<bool>(i, db::flip_key::done) == original_flips.at(i).done);
			CHECK(db.get_flip<std::string>(i, db::flip_key::item) == original_flips.at(i).item);
			CHECK(db.get_flip<i32>(i, db::flip_key::limit) == original_flips.at(i).buylimit);
			CHECK(db.get_flip<i32>(i, db::flip_key::sell) == original_flips.at(i).sell_price);
			CHECK(db.get_flip<i32>(i, db::flip_key::sold) == original_flips.at(i).sold_price);
		}
	};

	SUBCASE("Cancel flip")
	{
		i32 flip_to_cancel{};

		ensure_original_flip_states();

		SUBCASE("Flip A")
		{
			flip_to_cancel = 0;
		}

		SUBCASE("Flip B")
		{
			flip_to_cancel = 1;
		}

		flips::cancel(db, flip_to_cancel);

		CHECK(db.get_flip<bool>(flip_to_cancel, db::flip_key::cancelled));

		ensure_original_cancel_states(flip_to_cancel);
		ensure_original_flip_states(flip_to_cancel);
	}

	SUBCASE("Cancel a flip that has an invalid ID")
	{
		// ID 3 is invalid because flip_c is already done
		// and thus the last flip flip_d has an ID of 2
		constexpr i32 flip_to_cancel = 3;

		flips::cancel(db, flip_to_cancel);

		// If everything goes right, the program doesn't crash
		// and all of the flips stay unharmed
		ensure_original_flip_states();
	}

	SUBCASE("Find real ID with undone ID A")
	{
		const i32 id = flips::find_real_id_with_undone_id(db, 2);
		CHECK(id == 3);
	}

	SUBCASE("Find real ID with undone ID B")
	{
		const i32 id = flips::find_real_id_with_undone_id(db, 0);
		CHECK(id == 0);
	}

	SUBCASE("Find real ID with undone ID C")
	{
		const i32 id = flips::find_real_id_with_undone_id(db, 1);
		CHECK(id == 1);
	}

	SUBCASE("Find real ID with undone ID D")
	{
		const i32 id = flips::find_real_id_with_undone_id(db, 9);
		CHECK(id == -1);
	}

	SUBCASE("Sell a flip")
	{
		constexpr i32 flip_to_sell = 1;
		constexpr f32 tax = 0.98f;
		const i64 assumed_profit = ((flip_b.sell_price * tax) - flip_b.buy_price) * flip_b.buylimit;

		CHECK(db.get_flip<u64>(flip_to_sell, db::flip_key::sold) == 0);

		flips::sell(db, daily_progress, flip_to_sell, 0, 0);

		CHECK(db.get_flip<u64>(flip_to_sell, db::flip_key::sold) == flip_b.sell_price);
		CHECK(db.get_stat(db::stat_key::profit) == original_total_profit + assumed_profit);
		CHECK(daily_progress.current_progress() == assumed_profit + original_daily_progress);

		// Selling a flip shouldn't cancel anything
		ensure_original_cancel_states();
		ensure_original_done_states(flip_to_sell);
	}

	SUBCASE("Sell a non-existent flip")
	{
		constexpr i32 flip_to_sell = 14;

		flips::sell(db, daily_progress, flip_to_sell, 0, 0);

		// The program shouldn't crash and the flips and statistics should stay unharmed
		CHECK(db.get_stat(db::stat_key::profit) == original_total_profit);
		CHECK(daily_progress.current_progress() == original_daily_progress);
		ensure_original_flip_states();
	}
}

TEST_CASE("Flip selling")
{
	constexpr char json_str[] = R"~~~({
    "flips": [
        {
            "account": "alt2",
            "buy": 2461,
            "cancelled": false,
            "done": false,
            "item": "Huge plated steel salvage",
            "limit": 20000,
            "sell": 4856,
            "sold": 0
        },
        {
            "account": "alt2",
            "buy": 1101,
            "cancelled": false,
            "done": true,
            "item": "Tomato",
            "limit": 1376,
            "sell": 1271,
            "sold": 1207
        },
        {
            "account": "alt2",
            "buy": 6460,
            "cancelled": false,
            "done": true,
            "item": "Emerald necklace",
            "limit": 4950,
            "sell": 6992,
            "sold": 6992
        },
        {
            "account": "alt1",
            "buy": 6301,
            "cancelled": false,
            "done": false,
            "item": "Prayer potion (3)",
            "limit": 950,
            "sell": 19997,
            "sold": 0
        },
        {
            "account": "alt1",
            "buy": 811,
            "cancelled": false,
            "done": true,
            "item": "Regular ritual candle",
            "limit": 9950,
            "sell": 999,
            "sold": 999
        },
        {
            "account": "alt1",
            "buy": 1020,
            "cancelled": false,
            "done": true,
            "item": "Red dragonhide",
            "limit": 9950,
            "sell": 1289,
            "sold": 1037
        },
        {
            "account": "main",
            "buy": 2254,
            "cancelled": false,
            "done": true,
            "item": "Iron bar",
            "limit": 9950,
            "sell": 2532,
            "sold": 2480
        },
        {
            "account": "main",
            "buy": 4207,
            "cancelled": false,
            "done": false,
            "item": "Medium plated adamant salvage",
            "limit": 24950,
            "sell": 6599,
            "sold": 0
        },
        {
            "account": "main",
            "buy": 5000,
            "cancelled": false,
            "done": false,
            "item": "Adamant bar",
            "limit": 9950,
            "sell": 5287,
            "sold": 0
        },
        {
            "account": "main",
            "buy": 1468,
            "cancelled": false,
            "done": false,
            "item": "Yew shieldbow",
            "limit": 4950,
            "sell": 1649,
            "sold": 0
        },
        {
            "account": "main",
            "buy": 5,
            "cancelled": false,
            "done": true,
            "item": "item",
            "limit": 100,
            "sell": 20,
            "sold": 20
        },
        {
            "account": "main",
            "buy": 100,
            "cancelled": true,
            "done": false,
            "item": "Another test",
            "limit": 1000,
            "sell": 200,
            "sold": 0
        },
        {
            "account": "alt1",
            "buy": 6216,
            "cancelled": false,
            "done": false,
            "item": "Prayer potion (3)",
            "limit": 1950,
            "sell": 18699,
            "sold": 0
        },
        {
            "account": "alt1",
            "buy": 2041,
            "cancelled": false,
            "done": false,
            "item": "Red topaz",
            "limit": 4950,
            "sell": 7619,
            "sold": 7619
        },
        {
            "account": "alt2",
            "buy": 3001,
            "cancelled": false,
            "done": false,
            "item": "White oak",
            "limit": 4950,
            "sell": 3248,
            "sold": 0
        },
        {
            "account": "alt2",
            "buy": 1883,
            "cancelled": false,
            "done": false,
            "item": "Adamantite ore",
            "limit": 24950,
            "sell": 2801,
            "sold": 0
        },
        {
            "account": "alt1",
            "buy": 3502,
            "cancelled": false,
            "done": false,
            "item": "Yew incense sticks",
            "limit": 950,
            "sell": 4197,
            "sold": 0
        }
    ],
    "stats": {
        "flips_done": 6,
        "profit": 5445099
    }
}
)~~~";

	nlohmann::json json_data = nlohmann::json::parse(json_str);
	db db(json_data);
	daily_progress daily_progress;

	constexpr i32 known_ongoing_flip_count = 10;

	std::vector<i32> ongoing_flips;

	// Find the IDs for the "undone flips"
	for (i32 i = 0; i < known_ongoing_flip_count; ++i)
	{
		const i32 id = flips::find_real_id_with_undone_id(db, i);
		CHECK(id != -1);
		ongoing_flips.push_back(id);
	}

	// Verify the flip names
	CHECK(db.get_flip<std::string>(ongoing_flips.at(0), db::flip_key::item) == "Huge plated steel salvage");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(1), db::flip_key::item) == "Prayer potion (3)");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(2), db::flip_key::item) == "Medium plated adamant salvage");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(3), db::flip_key::item) == "Adamant bar");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(4), db::flip_key::item) == "Yew shieldbow");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(5), db::flip_key::item) == "Prayer potion (3)");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(6), db::flip_key::item) == "Red topaz");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(7), db::flip_key::item) == "White oak");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(8), db::flip_key::item) == "Adamantite ore");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(9), db::flip_key::item) == "Yew incense sticks");

	// Verify account names
	CHECK(db.get_flip<std::string>(ongoing_flips.at(0), db::flip_key::account) == "alt2");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(1), db::flip_key::account) == "alt1");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(2), db::flip_key::account) == "main");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(3), db::flip_key::account) == "main");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(4), db::flip_key::account) == "main");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(5), db::flip_key::account) == "alt1");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(6), db::flip_key::account) == "alt1");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(7), db::flip_key::account) == "alt2");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(8), db::flip_key::account) == "alt2");
	CHECK(db.get_flip<std::string>(ongoing_flips.at(9), db::flip_key::account) == "alt1");

	// Confirm that all of the listed flips are not done and are not cancelled
	for (i32 i = 0; i < known_ongoing_flip_count; ++i)
	{
		CHECK(db.get_flip<bool>(ongoing_flips.at(i), db::flip_key::cancelled) == false);
		CHECK(db.get_flip<bool>(ongoing_flips.at(i), db::flip_key::done) == false);
	}

	// Try selling the item "White oak"
	const i32 white_oak_index = ongoing_flips.at(7);

	CHECK_FALSE(db.get_flip<bool>(white_oak_index, db::flip_key::done));
	CHECK(db.get_flip<i32>(white_oak_index, db::flip_key::sell) == 3248);
	CHECK(db.get_flip<i32>(white_oak_index, db::flip_key::sold) == 0);

	// The ID shown in the list is 7
	flips::sell(db, daily_progress, 7, 0, 0);

	CHECK(db.get_flip<bool>(white_oak_index, db::flip_key::done));
	CHECK(db.get_flip<i32>(white_oak_index, db::flip_key::sold) == db.get_flip<i32>(white_oak_index, db::flip_key::sell));

	CHECK(db.get_flip<bool>(white_oak_index, db::flip_key::cancelled) == false);
	CHECK(db.get_flip<bool>(white_oak_index, db::flip_key::done) == true);
}
