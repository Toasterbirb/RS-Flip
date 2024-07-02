#include "Stats.hpp"

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

namespace stats
{
	f64 calc_roi(const nlohmann::json& flip)
	{
		return calc_roi(flip["buy"], flip["sold"]);
	}

	f64 calc_roi(const i32 buy_price, const i32 sell_price)
	{
		/* Avoid division by zero
		 * If the item cost nothing, return ROI-% of 100% */
		if (buy_price == 0)
			return 100;

		return ((sell_price - buy_price) / (double)buy_price) * 100;
	}

	TEST_CASE("Calculate the return of investment")
	{
		CHECK(calc_roi(20, 30) == 50);
		CHECK(calc_roi(10, 20) == 100);
		CHECK(calc_roi(20, 10) == -50);
	}

	std::vector<avg_stat> sort_flips_by_roi(std::vector<avg_stat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const avg_stat& a, const avg_stat& b) {
			return a.avg_roi() > b.avg_roi();
		});

		return flips;
	}

	std::vector<avg_stat> sort_flips_by_profit(std::vector<avg_stat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const avg_stat& a, const avg_stat& b) {
			return a.avg_profit() > b.avg_profit();
		});

		return flips;
	}

	std::vector<avg_stat> sort_flips_by_recommendation(std::vector<avg_stat> flips)
	{
		std::sort(flips.begin(), flips.end(), [](const avg_stat& a, const avg_stat& b) {
			return a.flip_recommendation() > b.flip_recommendation();
		});

		return flips;
	}
}
