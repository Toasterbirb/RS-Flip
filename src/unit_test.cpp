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
