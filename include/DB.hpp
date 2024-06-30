#pragma once

#include "AvgStat.hpp"
#include "Types.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace flips
{
	struct flip;
}

class db
{
public:
	db();

	/* Allow custom json data for testing purposes */
	__attribute__((cold))
	explicit db(const nlohmann::json& json_data);

	enum class flip_key
	{
		account, item, buy, sell, sold, limit, cancelled, done
	};

	enum class stat_key
	{
		flips_done, profit
	};

	void add_flip(const flips::flip& flip); /* Add a new flip */

	size_t total_flip_count() const; /* Total amount of flips (done and not done) */

	template<typename T>
	__attribute__((hot, warn_unused_result))
	T get_flip(const u32 index, const flip_key key) const
	{
		return json_data["flips"][index].at(flip_key_to_str.at(key));
	}

	template<typename T>
	__attribute__((warn_unused_result))
	f64 get_flip_average(const std::vector<u32>& indices, const flip_key key) const
	{
		T total = std::accumulate(indices.begin(), indices.end(), 0, [&](T total, u32 flip_index){
			const T json_value = json_data["flips"][flip_index].at(flip_key_to_str.at(key));
			return json_value + total;
		});

		return total / static_cast<f64>(indices.size());
	}

	template<typename T>
	__attribute__((warn_unused_result))
	T get_flip_min(const std::vector<u32>& indices, const flip_key key) const
	{
		std::vector<T> values = get_flip_values<T>(indices, key);
		return *std::min_element(values.begin(), values.end());
	}

	template<typename T>
	__attribute__((warn_unused_result))
	T get_flip_max(const std::vector<u32>& indices, const flip_key key) const
	{
		std::vector<T> values = get_flip_values<T>(indices, key);
		return *std::max_element(values.begin(), values.end());
	}

	template<typename T>
	__attribute__((hot))
	void set_flip(const u32 index, const flip_key key, const T data)
	{
		assert(index < json_data["flips"].size());
		json_data["flips"][index][flip_key_to_str.at(key)] = data;
	}

	__attribute__((warn_unused_result))
	flips::flip get_flip_obj(const u32 index) const;

	__attribute__((warn_unused_result))
	std::vector<stats::avg_stat> get_flip_avg_stats() const;

	__attribute__((warn_unused_result))
	std::vector<u32> find_flips_by_name(const std::string& item_name) const;

	__attribute__((warn_unused_result))
	std::vector<u32> find_flips_by_count(const u32 flip_count) const;

	__attribute__((warn_unused_result))
	i64 get_stat(const stat_key key) const;
	void set_stat(const stat_key key, const i64 data);

	void write(); /* Write the DB to disk */

private:
	nlohmann::json json_data;

	template<typename T>
	__attribute__((warn_unused_result))
	std::vector<T> get_flip_values(const std::vector<u32>& indices, const flip_key key) const
	{
		std::vector<T> values;
		for (u32 index : indices)
			values.push_back(json_data["flips"][index].at(flip_key_to_str.at(key)));

		return values;
	}

	void create_default_data_file();

	const static inline std::unordered_map<flip_key, std::string> flip_key_to_str = {
		{ flip_key::account,	"account" },
		{ flip_key::item,		"item" },
		{ flip_key::buy,		"buy" },
		{ flip_key::sell,		"sell" },
		{ flip_key::sold,		"sold" },
		{ flip_key::limit,		"limit" },
		{ flip_key::cancelled,	"cancelled" },
		{ flip_key::done,		"done" },
	};

	const static inline std::unordered_map<stat_key, std::string> stat_key_to_str = {
		{ stat_key::flips_done,	"flips_done" },
		{ stat_key::profit,		"profit" },
	};
};
