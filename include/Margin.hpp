#pragma once
#include "Flips.hpp"

namespace Margin
{
	int CalcMargin(int instaBuy, int instaSell);
	int CalcProfitWithCut(int margin, int buylimit, int pricecut);
	int CalcProfit(const int& buy_price, const int& sell_price, const int& buy_limit);
	int CalcProfit(Flips::Flip flip);
	int CalcProfitTaxFree(Flips::Flip flip);
	void PrintFlipEstimation(int instaBuy, int instaSell, int buylimit);
}
