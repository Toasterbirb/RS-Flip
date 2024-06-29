#pragma once

#include "Types.hpp"

namespace flips
{
	struct flip;
}

namespace margin
{
	__attribute__((const, warn_unused_result))
	i64 calc_margin(const int insta_buy, const int insta_sell);

	__attribute__((const, warn_unused_result))
	i64 calc_profit_with_cut(const int margin, const int buy_limit, const int price_cut);

	__attribute__((const, warn_unused_result))
	i64 calc_profit(const int buy_price, const int sell_price, const int buy_limit);

	__attribute__((const, warn_unused_result))
	i64 calc_profit(const flips::flip& flip);

	__attribute__((const, warn_unused_result))
	i64 calc_profit_tax_free(const flips::flip& flip);
	void print_flip_estimation(const int insta_buy, const int insta_sell, const int buy_limit);
}
