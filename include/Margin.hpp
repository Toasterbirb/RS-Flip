#pragma once

#include "Types.hpp"

namespace flips
{
	struct flip;
}

namespace margin
{
	__attribute__((const, warn_unused_result))
	i64 calc_margin(const i32 insta_buy, const i32 insta_sell);

	__attribute__((const, warn_unused_result))
	i64 calc_profit_with_cut(const i32 margin, const i32 buy_limit, const i32 price_cut);

	__attribute__((const, warn_unused_result))
	i64 calc_profit(const i32 buy_price, const i32 sell_price, const i32 buy_limit);

	__attribute__((const, warn_unused_result))
	i64 calc_profit(const flips::flip& flip);

	__attribute__((const, warn_unused_result))
	i64 calc_profit_tax_free(const flips::flip& flip);
	void print_flip_estimation(const i32 insta_buy, const i32 insta_sell, const i32 buy_limit);
}
