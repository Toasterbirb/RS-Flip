#pragma once
#include "Flips.hpp"

namespace margin
{
	int calc_margin(const int insta_buy, const int insta_sell);
	int calc_profit_with_cut(const int margin, const int buy_limit, const int price_cut);
	int calc_profit(const int buy_price, const int sell_price, const int buy_limit);
	int calc_profit(const flips::flip& flip);
	int calc_profit_tax_free(const flips::flip& flip);
	void print_flip_estimation(const int insta_buy, const int insta_sell, const int buy_limit);
}
