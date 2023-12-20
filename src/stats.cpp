#include "Flips.hpp"
#include "Stats.hpp"

namespace Stats
{
	double CalcFlipROI(const nlohmann::json& flip)
	{
		return CalcROI(flip["buy"], flip["sold"]);
	}

	double CalcROI(const nlohmann::json& flip)
	{
		return CalcROI(flip["buy"], flip["sold"]);
	}

	double CalcROI(const int& buy_price, const int& sell_price)
	{
		/* Avoid division by zero
		 * If the item cost nothing, return ROI-% of 100% */
		if (buy_price == 0)
			return 100;

		return ((sell_price - buy_price) / (double)buy_price) * 100;
	}

	TEST_CASE("Calculate the return of investment")
	{
		CHECK(CalcROI(20, 30) == 50);
		CHECK(CalcROI(10, 20) == 100);
		CHECK(CalcROI(20, 10) == -50);
	}

	std::vector<AvgStat> SortFlipsByROI(const std::vector<AvgStat>& flips)
	{
		std::vector<AvgStat> result = flips;

		/* TODO: Switch to some other algorithm than bubblesort */

		bool had_swap = true;
		AvgStat placeholder;
		while (had_swap)
		{
			had_swap = false;
			for (size_t i = 0; i < flips.size() - 1; i++)
			{
				if (result[i].AvgROI() < result[i + 1].AvgROI())
				{
					placeholder = result[i];
					result[i] = result[i + 1];
					result[i + 1] = placeholder;
					had_swap = true;
				}
			}
		}

		return result;
	}

	std::vector<AvgStat> SortFlipsByProfit(const std::vector<AvgStat>& flips)
	{
		std::vector<AvgStat> result = flips;

		/* TODO: Switch to some other algorithm than bubblesort */

		bool had_swap = true;
		AvgStat placeholder;
		while (had_swap)
		{
			had_swap = false;
			for (size_t i = 0; i < flips.size() - 1; i++)
			{
				if (result[i].AvgProfit() < result[i + 1].AvgProfit())
				{
					placeholder = result[i];
					result[i] = result[i + 1];
					result[i + 1] = placeholder;
					had_swap = true;
				}
			}
		}

		return result;
	}

	std::vector<AvgStat> SortFlipsByRecommendation(const std::vector<AvgStat>& flips)
	{
		std::vector<AvgStat> result = flips;

		/* TODO: Switch to some other algorithm than bubblesort */

		bool had_swap = true;
		AvgStat placeholder;
		while (had_swap)
		{
			had_swap = false;
			for (size_t i = 0; i < result.size() - 1; i++)
			{
				/* Skip flips with only one trade done */
				if (result[i].FlipCount() == 1)
				{
					result.erase(result.begin() + i);
					i--;
					continue;
				}

				if (result[i].FlipRecommendation() < result[i + 1].FlipRecommendation())
				{
					placeholder = result[i];
					result[i] = result[i + 1];
					result[i + 1] = placeholder;
					had_swap = true;
				}
			}
		}

		return result;
	}
}
