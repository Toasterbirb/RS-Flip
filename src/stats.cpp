#include "Flips.hpp"
#include "Stats.hpp"

namespace Stats
{
	/* Internal function declarations */
	double CalcFlipROI(const nlohmann::json& flip);


	double CalcFlipROI(const nlohmann::json& flip)
	{
		return CalcROI(flip["buy"], flip["sold"]);
	}

	double CalcROI(const nlohmann::json& flip)
	{
		return CalcROI(flip["buy"], flip["sold"]);
	}

	double CalcROI(const int buy_price, const int sell_price)
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

	std::vector<AvgStat> SortFlipsByROI(std::vector<AvgStat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const AvgStat& a, const AvgStat& b) {
			return a.AvgROI() > b.AvgROI();
		});

		return flips;
	}

	std::vector<AvgStat> SortFlipsByProfit(std::vector<AvgStat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const AvgStat& a, const AvgStat& b) {
			return a.AvgProfit() > b.AvgProfit();
		});

		return flips;
	}

	std::vector<AvgStat> SortFlipsByRecommendation(std::vector<AvgStat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const AvgStat& a, const AvgStat& b) {
			return a.FlipRecommendation() > b.FlipRecommendation();
		});

		return flips;
	}
}
