#pragma once
#include "Flips.hpp"

namespace Margin
{
	int CalcMargin(int instaBuy, int instaSell);
	int CalcProfit(int margin, int buylimit, int pricecut);
	int CalcProfit(Flips::Flip flip);
	void PrintFlipEstimation(int instaBuy, int instaSell, int buylimit);
}
