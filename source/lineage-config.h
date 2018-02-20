#ifndef LINEAGE_CONFIG_H
#define LINEAGE_CONFIG_H

#include "config/config.h"

EMP_BUILD_CONFIG( LineageConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(RANDOM_SEED, int, -1, "Random number seed (negative value for based on time)"),
  VALUE(POP_SIZE, size_t, 1000, "Total population size"),
  VALUE(GENERATIONS, size_t, 100, "How many generations should we run evolution?"),
  VALUE(EVAL_TIME, size_t, 256, "Agent evaluation time"),
  VALUE(REPRESENTATION, size_t, 0, "Which representation are we evolving? 0: ... "),
)

#endif
