#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Table.hpp"

#include <doctest/doctest.h>
#include <iostream>

namespace margin
{
	static constexpr f32 tax_rate = 0.98f;

	i64 calc_margin(const i32 insta_buy, const i32 insta_sell)
	{
		return insta_buy - insta_sell;
	}

	TEST_CASE("Calculate margin")
	{
		CHECK(calc_margin(2000, 1000) == 1000);
		CHECK(calc_margin(4000, 5000) == -1000);
	}

	i64 calc_profit_with_cut(const i64 margin, const i32 buy_limit, const i32 price_cut)
	{
		return buy_limit * (margin - (price_cut * 2));
	}

	TEST_CASE("Calculate profit")
	{
		CHECK(calc_profit_with_cut(20, 500, 0) == 10000);
		CHECK(calc_profit_with_cut(20, 500, 1) == 9000);
		CHECK(calc_profit_with_cut(20, 500, 2) == 8000);
		CHECK(calc_profit_with_cut(-5, 10, 0) == -50);
	}

	i64 calc_profit(const i64 buy_price, const i64 sell_price, const i32 buy_limit)
	{
		/* f64 is used to handle very large price values (over 32bit limit) with smaller
		 * floating point imprecisions */
		const f64 taxed_sell_price = (sell_price <= 50 ? sell_price : sell_price * tax_rate);
		return (taxed_sell_price - buy_price) * buy_limit;
	}

	i64 calc_profit(const flips::flip& flip)
	{
		constexpr f32 non_taxed_rate = 1.0f;
		const i64 non_taxed_sell_price = flip.done ? flip.sold_price : flip.sell_price;
		const f64 taxed_sell_price = non_taxed_sell_price * ( non_taxed_sell_price <= 50 ? non_taxed_rate : tax_rate );

		/* Items with a price at or below 50 won't be taxed */
		return (taxed_sell_price - flip.buy_price) * flip.buylimit;
	}

	i64 calc_profit_tax_free(const flips::flip& flip)
	{
		return (( flip.done ? flip.sold_price : flip.sell_price ) - flip.buy_price) * flip.buylimit;
	}

	TEST_CASE("Calculate profit from a flip")
	{
		const auto check_flip_profit = [](const flips::flip& flip, const i64 expected_profit)
		{
			CHECK(calc_profit(flip) == expected_profit);
			CHECK(calc_profit(flip.buy_price, flip.sell_price, flip.buylimit) == expected_profit);
		};

		SUBCASE("Some item") { check_flip_profit(flips::flip("Some item", 10, 20, 50), 500); }
		SUBCASE("Another item") { check_flip_profit(flips::flip("Another item", 30, 15, 90), -1350); }
		SUBCASE("Fancy item") { check_flip_profit(flips::flip("Fancy item", 400, 950, 10000), 5310000); }
		SUBCASE("Cool hat") { check_flip_profit(flips::flip("Cool hat", 230'000'000, 320'000'000, 12), 1003200000); }
		SUBCASE("Very cool hat") { check_flip_profit(flips::flip("Very cool hat", 1'200'000'000, 1'900'000'000, 20), 13240000000); }
		SUBCASE("Yew logs") { check_flip_profit(flips::flip("Yew logs", 277, 276, 24999), -162993); }

		SUBCASE("Flip that is 'sold'")
		{
			constexpr i64 flip_f_profit = 332000;
			flips::flip flip_f("Some shoes", 256, 700, 1000);
			flip_f.done = true;
			flip_f.sold_price = 600;
			CHECK(calc_profit(flip_f) == flip_f_profit);
			CHECK(calc_profit(flip_f.buy_price, flip_f.sold_price, flip_f.buylimit) == flip_f_profit);
		}
	}

	void print_flip_estimation(const i32 insta_buy, const i32 insta_sell, const i32 buy_limit)
	{
		/* Multiply the sell price by 0.98 to account for the 2% tax */
		const i64 margin = calc_margin((insta_buy - 1) * 0.98f, insta_sell + 1);
		const i64 cut_profit = calc_profit_with_cut(margin, buy_limit, 0);	/* The cut has already been taken into account in margin */
																 			/* We can't use CalcProfit here though because it also */
																 			/* calculates the taxes */
		const std::string roi = flip_utils::round(((double)margin / insta_sell) * 100, 2) + "%";
		const std::string required_capital = flip_utils::round_big_numbers(insta_sell * buy_limit);

		/* Green color by default */
		i32 color_code = 32;

		/* Set red color when you'd be making a loss */
		if (cut_profit < 0)
			color_code = 31;

		/* Color coded string for the profit text */
		const std::string profit_str = "\033[" + std::to_string(color_code) + "m" + flip_utils::round_big_numbers(cut_profit) + "\033[0m";

		table stat_table({"Stat", "Value"});
		stat_table.add_row({"Margin", std::to_string(margin)});
		stat_table.add_row({"ROI-%", roi});
		stat_table.add_row({"Cost", required_capital});
		stat_table.add_row({"Profit", profit_str});

		table price_table({"Offer", "Price"});
		price_table.add_row({"Buy", std::to_string(insta_sell + 1)});
		price_table.add_row({"Sell", std::to_string(insta_buy - 1)});

		stat_table.print();
		std::cout << '\n';
		price_table.print();
	}
}
