#include "Margin.hpp"
#include "FlipUtils.hpp"
#include "Table.hpp"

namespace Margin
{
	int CalcMargin(const int insta_buy, const int insta_sell)
	{
		return insta_buy - insta_sell;
	}

	TEST_CASE("Calculate margin")
	{
		CHECK(CalcMargin(2000, 1000) == 1000);
		CHECK(CalcMargin(4000, 5000) == -1000);
	}

	int CalcProfitWithCut(const int margin, const int buy_limit, const int price_cut)
	{
		return buy_limit * (margin - (price_cut * 2));
	}

	TEST_CASE("Calculate profit")
	{
		CHECK(CalcProfitWithCut(20, 500, 0) == 10000);
		CHECK(CalcProfitWithCut(20, 500, 1) == 9000);
		CHECK(CalcProfitWithCut(20, 500, 2) == 8000);
		CHECK(CalcProfitWithCut(-5, 10, 0) == -50);
	}

	int CalcProfit(const int buy_price, const int sell_price, const int buy_limit)
	{
		if (sell_price <= 50)
			return (sell_price - buy_price) * buy_limit;
		else
			return ((sell_price * 0.98f) - buy_price) * buy_limit;
	}

	int CalcProfit(const Flips::Flip& flip)
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

	int CalcProfitTaxFree(const Flips::Flip& flip)
	{
		if (flip.done)
			return (flip.sold_price - flip.buy_price) * flip.buylimit;
		else
			return (flip.sell_price - flip.buy_price) * flip.buylimit;
	}

	TEST_CASE("Calculate profit from a flip")
	{
		Flips::Flip flipA("Some item", 10, 20, 50);
		CHECK(CalcProfit(flipA) == 500);

		Flips::Flip flipB("Another item", 30, 15, 90);
		CHECK(CalcProfit(flipB) == -1350);

		Flips::Flip yewLogs("Yew logs", 277, 276, 24999);
		CHECK(CalcProfit(yewLogs) == -162993);
	}

	void PrintFlipEstimation(const int insta_buy, const int insta_sell, const int buy_limit)
	{
		/* Multiply the sell price by 0.98 to account for the 2% tax */
		int margin = CalcMargin((insta_buy - 1) * 0.98f, insta_sell + 1);
		int cut_profit = CalcProfitWithCut(margin, buy_limit, 0); /* The cut has already been taken into account in margin */
																 /* We can't use CalcProfit here though because it also */
																 /* calculates the taxes */
		std::string roi = FlipUtils::Round(((double)margin / insta_sell) * 100, 2) + "%";
		std::string required_capital = FlipUtils::RoundBigNumbers(insta_sell * buy_limit);

		/* Green color by default */
		int color_code = 32;

		/* Set red color when you'd be making a loss */
		if (cut_profit < 0)
			color_code = 31;

		/* Color coded string for the profit text */
		std::string profit_str = "\033[" + std::to_string(color_code) + "m" + FlipUtils::RoundBigNumbers(cut_profit) + "\033[0m";

		Table stat_table({"Stat", "Value"});
		stat_table.add_row({"Margin", std::to_string(margin)});
		stat_table.add_row({"ROI-%", roi});
		stat_table.add_row({"Cost", required_capital});
		stat_table.add_row({"Profit", profit_str});

		Table price_table({"Offer", "Price"});
		price_table.add_row({"Buy", std::to_string(insta_sell + 1)});
		price_table.add_row({"Sell", std::to_string(insta_buy - 1)});

		stat_table.print();
		std::cout << '\n';
		price_table.print();
	}
}
