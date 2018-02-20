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
  VALUE(TEST_CASE_FILE, std::string, "testcases.csv", "..."),
  VALUE(ANCESTOR_FPATH, std::string, "ancestor.gp", "Ancestor program file"),
  GROUP(SELECTION_GROUP, "Selection Settings"),
  VALUE(SELECTION_METHOD, size_t, 0, "Which selection method are we using 0: ..."),
  VALUE(TOURNAMENT_SIZE, size_t, 4, "How big are tournaments when using tournament selection?"),
  GROUP(OTHELLO_GROUP, "Othello-specific Settings"),
  VALUE(OTHELLO_BOARD_WIDTH, size_t, 8, "Othello board will be SIZE X SIZE."),
  VALUE(OTHELLO_HW_BOARDS, size_t, 1, "How many dream boards are given to agents for them to manipulate?"),
  GROUP(SGP_HARDWARE_GROUP, "SignalGP Hardware Settings"),
  VALUE(SGP_HW_MAX_CORES, size_t, 16, "Max number of hardware cores; i.e., max number of simultaneous threads of execution hardware will support."),
  VALUE(SGP_HW_MAX_CALL_DEPTH, size_t, 128, "Max call depth of hardware unit"),
  VALUE(SGP_HW_MIN_BIND_THRESH, double, 0.5, "Hardware minimum binding threshold"),
  GROUP(SGP_MUTATION_GROUP, "SignalGP Mutation Settings"),
  VALUE(SGP_PROG_MAX_ARG_VAL, int, 16, "Maximum argument value for instructions."),
  VALUE(SGP_PER_BIT__TAG_BFLIP_RATE, double, 0.05, "Per-bit mutation rate of affinity bit flips."),
  VALUE(SGP_PER_INST__SUB_RATE, double, 0.005, "Per-instruction subsitution mutation rate."),
  GROUP(DATA_GROUP, "Data Collection Settings"),
  VALUE(SYSTEMATICS_INTERVAL, size_t, 100, "Interval to record systematics summary stats."),
  VALUE(FITNESS_INTERVAL, size_t, 100, "Interval to record fitness summary stats."),
  VALUE(POP_SNAPSHOT_INTERVAL, size_t, 100, "Interval to take a full snapshot of the population."),
  VALUE(DATA_DIRECTORY, std::string, "./", "Location to dump data output."),
)

#endif
