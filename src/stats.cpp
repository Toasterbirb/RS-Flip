#include "Flips.hpp"
#include "Margin.hpp"
#include "Stats.hpp"
#include "doctest/doctest.h"

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
			for (int i = 0; i < flips.size() - 1; i++)
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
			for (int i = 0; i < flips.size() - 1; i++)
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

	std::vector<AvgStat> SortFlipsByStability(const std::vector<AvgStat>& flips)
	{
		std::vector<AvgStat> result = flips;

		/* TODO: Switch to some other algorithm than bubblesort */

		bool had_swap = true;
		AvgStat placeholder;
		while (had_swap)
		{
			had_swap = false;
			for (int i = 0; i < result.size() - 1; i++)
			{
				/* Erase a flip if it has only been done once */
				if (result[i].FlipCount() == 1)
				{
					result.erase(result.begin() + i);
					i--;
					continue;
				}

				if ((result[i].FlipStability() > result[i + 1].FlipStability())
						|| (result[i].FlipStability() == result[i + 1].FlipStability() && result[i].FlipCount() < result[i + 1].FlipCount())
						|| (result[i].FlipStability() == result[i + 1].FlipStability() && result[i].AvgProfit() < result[i + 1].AvgProfit()))
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
			for (int i = 0; i < result.size() - 1; i++)
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

	TEST_CASE("Sorting flips")
	{
		/* Create a few flips */
		Flips::Flip flipA("Item A", 15, 40, 100); /* ROI: 166,67% | Profit : 2500 */
		flipA.sold_price = flipA.sell_price;
		Flips::Flip flipAb("Item A", 15, 20, 100); /* ROI: 33,33% | Profit: 500 */
		flipAb.sold_price = flipAb.sell_price;
		Flips::Flip flipAc("Item A", 15, 22, 100); /* ROI: 46,67% | Profit: 700 */
		flipAc.sold_price = flipAc.sell_price;
		/* A average ROI = 82,2233% */
		/* A average Profit = 1233,33 */

		Flips::Flip flipB("Item B", 20, 30, 100); /* ROI: 50% | Profit: 1000 */
		flipB.sold_price = flipB.sell_price;

		Flips::Flip flipC("Item C", 10, 20, 3000); /* ROI: 100% | Profit: 30000 */
		flipC.sold_price = flipC.sell_price;

		Flips::Flip flipD("Item D", 20, 10, 100); /* ROI: -50% | Profit: -1000 */
		flipD.sold_price = flipD.sell_price;
		Flips::Flip flipDb("Item D", 20, 50, 150); /* ROI: 150% | Profit: 4500 */
		flipDb.sold_price = flipDb.sell_price;
		Flips::Flip flipDc("Item D", 20, 70, 700); /* ROI: 250% | Profit: 35000 */
		flipDc.sold_price = flipDc.sell_price;
		/* D average ROI = 116,67% */
		/* D average Profit = 12833,33 */

		/* Turn the flips into a json array */
		std::vector<nlohmann::json> flips;
		flips.push_back(flipA.ToJson());
		flips.push_back(flipAb.ToJson());
		flips.push_back(flipAc.ToJson());
		flips.push_back(flipB.ToJson());
		flips.push_back(flipC.ToJson());
		flips.push_back(flipD.ToJson());
		flips.push_back(flipDb.ToJson());
		flips.push_back(flipDc.ToJson());

		/* Set all flips as done */
		for (int i = 0; i < flips.size(); i++)
			flips[i]["done"] = true;

		std::vector<AvgStat> flipAvgStats = FlipsToAvgstats(flips);

		SUBCASE("Sort by average ROI")
		{
			std::vector<AvgStat> sortedList = SortFlipsByROI(flipAvgStats);

			CHECK(sortedList[0].name == "Item D");
			CHECK(sortedList[1].name == "Item C");
			CHECK(sortedList[2].name == "Item A");
			CHECK(sortedList[3].name == "Item B");
		}

		SUBCASE("Sort by profit")
		{
			std::vector<AvgStat> sortedList = SortFlipsByProfit(flipAvgStats);

			CHECK(sortedList[0].name == "Item C");
			CHECK(sortedList[1].name == "Item D");
			CHECK(sortedList[2].name == "Item A");
			CHECK(sortedList[3].name == "Item B");
		}

		SUBCASE("Sort by stability")
		{
			std::vector<AvgStat> sortedList = SortFlipsByStability(flipAvgStats);

			CHECK(sortedList[0].name == "Item A");
			CHECK(sortedList[1].name == "Item D");
		}
	}

}
