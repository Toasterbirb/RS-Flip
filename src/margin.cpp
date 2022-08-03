#include "Margin.hpp"
#include "Utils.hpp"
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

	int CalcProfit(int margin, int buylimit, int pricecut)
	{
		return buylimit * (margin - (pricecut * 2));
	}

	TEST_CASE("Calculate profit")
	{
		CHECK(CalcProfit(20, 500, 0) == 10000);
		CHECK(CalcProfit(20, 500, 1) == 9000);
		CHECK(CalcProfit(20, 500, 2) == 8000);
	}

	int CalcProfit(Flips::Flip flip)
	{
		return (flip.sell_price - flip.buy_price) * flip.buylimit;
	}

	TEST_CASE("Calculate profit from a flip")
	{
		Flips::Flip flipA("Some item", 10, 20, 50);
		CHECK(CalcProfit(flipA) == 500);

		Flips::Flip flipB("Another item", 30, 15, 90);
		CHECK(CalcProfit(flipB) == -1350);
	}

	void PrintLine()
	{
		std::cout << "-------------------------------\n";
	}

	void PrintFlipEstimation(int instaBuy, int instaSell, int buylimit)
	{
		int margin = CalcMargin(instaBuy, instaSell);
		int profit = CalcProfit(margin, buylimit, 0);
		int cut_profit = CalcProfit(margin, buylimit, 1);
		int big_cut_profit = CalcProfit(margin, buylimit, 5);

		std::cout << "Margin: " << margin << std::endl;
		std::cout << "Required capital: " << Utils::RoundBigNumbers(instaSell * (buylimit - 1)) << std::endl;
		std::cout << "ROI: " << ((double)margin / instaSell) * 100 << "%\n";

		PrintLine();

		std::cout << "Profit: " << profit << " (" << Utils::RoundBigNumbers(profit) << ")" << std::endl;
		std::cout << "Profit (after price check): " << profit - margin << " (" << Utils::RoundBigNumbers(profit - margin) << ")" << std::endl;

		PrintLine();

		std::cout << "\e[31mProfit (1 cut): " << cut_profit << " (" << Utils::RoundBigNumbers(cut_profit) << ")\e[0m" << std::endl;
		std::cout << "Profit (5 cut): " << big_cut_profit << " (" << Utils::RoundBigNumbers(big_cut_profit) << ")" << std::endl;

		PrintLine();

		std::cout << "Buy for " << instaSell + 1 << std::endl;
		std::cout << "Sell for " << instaBuy - 1 << std::endl;
	}
}
