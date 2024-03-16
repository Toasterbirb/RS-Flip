#pragma once

#include <random>

class random
{
public:
	/**
	 * @brief Construct a random number generator with a random seed
	 *
	 * On linux this will get the seed by reading a few bytes from
	 * /dev/urandom. If this file cannot be opened (or we are not
	 * on linux) time will be used as the seed instead.
	 */
	random();

	/**
	 * @brief Change the seed of the random number engine
	 *
	 * @param seed
	 */
	void seed(unsigned int seed);

	/**
	 * @brief Get the next random number from the random number engine
	 */
	unsigned long next();

	/**
	 * @brief Generate a random integer value between min and max (inclusive)
	 */
	template<typename T>
	T range(T min, T max)
	{
		return rng_engine() % (max + 1 - min) + min;
	}

	/**
	 * @brief Generate a random floating point value between min and max
	 *
	 * @warning Using this method with integer values will return non-random values. Only use this method with floating point ranges
	 */
	template<typename T>
	T range_float(T min, T max)
	{
		T multiplier = (static_cast<T>(rng_engine())) / rng_engine.max();
		return (multiplier * (max - min)) + min;
	}

private:
	std::mt19937_64 rng_engine;
};
