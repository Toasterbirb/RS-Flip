#pragma once

#include "Types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace stats
{
	enum class recommendation_algorithm
	{
		v1 = 0,
		v2 = 1
	};

	class avg_stat
	{
	public:
		avg_stat();
		explicit avg_stat(const std::string& item_name);
		void add_data(const i64 profit, const f64 ROI, const u32 item_count, const u32 latest_trade_index = 0);
		void inc_cancel_count();
		f64 avg_profit() const;
		f64 normalized_avg_profit() const;
		f64 profit_standard_deviation() const;
		f64 rolling_avg_profit(const u32 window_size) const; /* Get the avg. profit of the latest flips */
		f64 avg_roi() const;
		f64 normalized_avg_roi() const;
		f64 avg_buy_limit() const;
		f64 normalized_avg_buy_limit() const;
		f64 flip_recommendation() const;
		u32 flip_count() const;
		u32 profitable_flip_count() const;
		u32 cancelled_flip_count() const;
		f64 cancellation_ratio() const;
		i32 latest_trade_index() const;
		const std::vector<i32>& profits() const;

		// how many flips have been done in total
		static u32 total_flip_count();

		static void set_recommendation_algorithm(const u8 algorithm);

		std::string name;

		// range of average profits
		static inline f64 min_avg_profit{0};
		static inline f64 max_avg_profit{0};

		// range of average buy limits
		static inline f64 min_avg_buy_limit{0};
		static inline f64 max_avg_buy_limit{0};

		// range of ROI-%
		static inline f64 min_avg_roi{0};
		static inline f64 max_avg_roi{0};

	private:
		std::vector<i32> profit_list;

		i64 total_profit = 0;
		f64 total_roi = 0;
		i32 total_item_count = 0;
		u32 _latest_trade_index = 0;
		u32 _cancelled_flip_count = 0;

		// the total amount (count) of flip data added to avgstats
		static inline u32 _total_flip_count{0};
	};

	std::vector<avg_stat> flips_to_avg_stats(const std::vector<nlohmann::json>& flips);
}
