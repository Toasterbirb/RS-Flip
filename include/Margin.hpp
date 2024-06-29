#pragma once

#include "Types.hpp"

namespace flips
{
	struct flip;
}

namespace margin
{
	i64 calc_margin(const int insta_buy, const int insta_sell);
	i64 calc_profit_with_cut(const int margin, const int buy_limit, const int price_cut);
	i64 calc_profit(const int buy_price, const int sell_price, const int buy_limit);
	i64 calc_profit(const flips::flip& flip);
	i64 calc_profit_tax_free(const flips::flip& flip);
	void print_flip_estimation(const int insta_buy, const int insta_sell, const int buy_limit);
}
