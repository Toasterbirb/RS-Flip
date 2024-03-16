#include "Random.hpp"

#include <ctime>

random::random()
{
	unsigned int seed = time(0);
	rng_engine.seed(seed);
}

void random::seed(unsigned int seed)
{
	rng_engine.seed(seed);
}

unsigned long random::next()
{
	return rng_engine();
}
