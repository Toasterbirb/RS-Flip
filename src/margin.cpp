#include "FlipUtils.hpp"
#include "Flips.hpp"
#include "Margin.hpp"
#include "Table.hpp"

#include <doctest/doctest.h>
#include <iostream>

namespace margin
{
	i64 calc_margin(const int insta_buy, const int insta_sell)
	{
		return insta_buy - insta_sell;
	}

	TEST_CASE("Calculate margin")
	{
		CHECK(calc_margin(2000, 1000) == 1000);
		CHECK(calc_margin(4000, 5000) == -1000);
	}

	i64 calc_profit_with_cut(const int margin, const int buy_limit, const int price_cut)
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

	i64 calc_profit(const int buy_price, const int sell_price, const int buy_limit)
	{
		if (sell_price <= 50)
			return (sell_price - buy_price) * buy_limit;
		else
			return ((sell_price * 0.98f) - buy_price) * buy_limit;
	}

	i64 calc_profit(const flips::flip& flip)
	{
		if (flip.done)
			if (flip.sold_price <= 50)
				return (flip.sold_price - flip.buy_price) * flip.buylimit;
			else
				return ((flip.sold_price * 0.98) - flip.buy_price) * flip.buylimit;
		else
			if (flip.sell_price <= 50)
				return (flip.sell_price - flip.buy_price) * flip.buylimit;
			else
				return ((flip.sell_price * 0.98) - flip.buy_price) * flip.buylimit;
	}

	i64 calc_profit_tax_free(const flips::flip& flip)
	{
		if (flip.done)
			return (flip.sold_price - flip.buy_price) * flip.buylimit;
		else
			return (flip.sell_price - flip.buy_price) * flip.buylimit;
	}

	TEST_CASE("Calculate profit from a flip")
	{
		flips::flip flip_a("Some item", 10, 20, 50);
		CHECK(calc_profit(flip_a) == 500);

		flips::flip flip_b("Another item", 30, 15, 90);
		CHECK(calc_profit(flip_b) == -1350);

		flips::flip yew_logs("Yew logs", 277, 276, 24999);
		CHECK(calc_profit(yew_logs) == -162993);
	}

	void print_flip_estimation(const int insta_buy, const int insta_sell, const int buy_limit)
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
		std::string profit_str = "\033[" + std::to_string(color_code) + "m" + flip_utils::round_big_numbers(cut_profit) + "\033[0m";

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
