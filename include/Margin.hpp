#pragma once

namespace Margin
{
	int CalcMargin(int instaBuy, int instaSell);
	int CalcProfit(int margin, int buylimit, int pricecut);
	void PrintFlipEstimation(int instaBuy, int instaSell, int buylimit);
}
