#pragma once
#include "Flips.hpp"

namespace Margin
{
	int CalcMargin(const int instaBuy, const int instaSell);
	int CalcProfitWithCut(const int margin, const int buylimit, const int pricecut);
	int CalcProfit(const int buy_price, const int sell_price, const int buy_limit);
	int CalcProfit(const Flips::Flip& flip);
	int CalcProfitTaxFree(const Flips::Flip& flip);
	void PrintFlipEstimation(const int instaBuy, const int instaSell, const int buylimit);
}
