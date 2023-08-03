#include "Margin.hpp"
#include "FlipUtils.hpp"
#include "doctest/doctest.h"

namespace Margin
{
	int CalcMargin(int instaBuy, int instaSell)
	{
		return instaBuy - instaSell;
	}

	TEST_CASE("Calculate margin")
	{
		CHECK(CalcMargin(2000, 1000) == 1000);
		CHECK(CalcMargin(4000, 5000) == -1000);
	}

	int CalcProfitWithCut(int margin, int buylimit, int pricecut)
	{
		return buylimit * (margin - (pricecut * 2));
	}

	TEST_CASE("Calculate profit")
	{
		CHECK(CalcProfitWithCut(20, 500, 0) == 10000);
		CHECK(CalcProfitWithCut(20, 500, 1) == 9000);
		CHECK(CalcProfitWithCut(20, 500, 2) == 8000);
		CHECK(CalcProfitWithCut(-5, 10, 0) == -50);
	}

	int CalcProfit(const int& buy_price, const int& sell_price, const int& buy_limit)
	{
		if (sell_price <= 50)
			return (sell_price - buy_price) * buy_limit;
		else
			return ((sell_price * 0.98f) - buy_price) * buy_limit;
	}

	int CalcProfit(Flips::Flip flip)
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

	int CalcProfitTaxFree(Flips::Flip flip)
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
		CHECK(CalcProfit(yewLogs) == -24999);
	}

	void PrintLine()
	{
		std::cout << "-------------------------------\n";
	}

	void PrintFlipEstimation(int instaBuy, int instaSell, int buylimit)
	{
		/* Multiply the sell price by 0.98 to account for the 2% tax */
		int margin = CalcMargin(instaBuy * 0.98f, instaSell);
		int profit = CalcProfitWithCut(margin, buylimit, 0);
		int cut_profit = CalcProfitWithCut(margin, buylimit - 1, 1);
		int big_cut_profit = CalcProfitWithCut(margin, buylimit - 1, 5);

		std::cout << "Margin: " << margin << std::endl;
		std::cout << "Required capital: " << FlipUtils::RoundBigNumbers(instaSell * (buylimit - 1)) << std::endl;
		std::cout << "ROI: " << ((double)margin / instaSell) * 100 << "%\n";

		PrintLine();

		std::cout << "Profit: " << profit << " (" << FlipUtils::RoundBigNumbers(profit) << ")" << std::endl;
		std::cout << "Profit (after price check): " << profit - margin << " (" << FlipUtils::RoundBigNumbers(profit - margin) << ")" << std::endl;

		PrintLine();

		/* Green color by default */
		int color_code = 32;

		/* Set red color when you'd be making a loss */
		if (cut_profit < 0)
			color_code = 31;

		std::cout << "\e[" << color_code << "mProfit (1 cut): " << cut_profit << " (" << FlipUtils::RoundBigNumbers(cut_profit) << ")\e[0m" << std::endl;
		std::cout << "Profit (5 cut): " << big_cut_profit << " (" << FlipUtils::RoundBigNumbers(big_cut_profit) << ")" << std::endl;

		PrintLine();

		std::cout << "Buy for " << instaSell + 1 << std::endl;
		std::cout << "Sell for " << instaBuy - 1 << std::endl;
	}
}