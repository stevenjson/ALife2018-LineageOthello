#ifndef TOY_CONFIG_H
#define TOY_CONFIG_H

#include "config/config.h"

EMP_BUILD_CONFIG( ToyConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(RUN_MODE, size_t, 0, "What mode are we running in? \n0: Native experiment\n1: Analyze mode"),
  VALUE(RANDOM_SEED, int, -1, "Random number seed (negative value for based on time)"),
  VALUE(POP_SIZE, size_t, 1000, "Total population size"),
  VALUE(GENERATIONS, size_t, 5000, "How many generations should we run evolution?")
)

#endif
