#pragma once
#include "Flips.hpp"

namespace Margin
{
	int CalcMargin(const int insta_buy, const int insta_sell);
	int CalcProfitWithCut(const int margin, const int buy_limit, const int price_cut);
	int CalcProfit(const int buy_price, const int sell_price, const int buy_limit);
	int CalcProfit(const Flips::Flip& flip);
	int CalcProfitTaxFree(const Flips::Flip& flip);
	void PrintFlipEstimation(const int insta_buy, const int insta_sell, const int buy_limit);
}
