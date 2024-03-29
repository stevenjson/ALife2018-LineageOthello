#ifndef LINEAGE_CONFIG_H
#define LINEAGE_CONFIG_H

#include "config/config.h"

EMP_BUILD_CONFIG( LineageConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(RUN_MODE, size_t, 0, "What mode are we running in? \n0: Native experiment\n1: Analyze mode"),
  VALUE(RANDOM_SEED, int, -1, "Random number seed (negative value for based on time)"),
  VALUE(POP_SIZE, size_t, 1000, "Total population size"),
  VALUE(GENERATIONS, size_t, 5000, "How many generations should we run evolution?"),
  VALUE(EVAL_TIME, size_t, 1000, "Agent evaluation time (how much time an agent has on a single turn)"),
  VALUE(REPRESENTATION, size_t, 0, "Which representation are we evolving?\n0: AvidaGP\n1: SignalGP "),
  VALUE(TEST_CASE_FILE, std::string, "testcases.csv", "From what file should we load testcases from?"),
  VALUE(ANCESTOR_FPATH, std::string, "ancestor.gp", "Ancestor program file"),
  VALUE(POP_INITIALIZATION_METHOD, size_t, 0, "How do we initialize the population? \n0: ancestor file\n1: randomly generated"),
  GROUP(SELECTION_GROUP, "Selection Settings"),
  VALUE(SELECTION_METHOD, size_t, 0, "Which selection method are we using? \n0: Tournament\n1: Lexicase\n2: Eco-EA (resource)\n3: MAP-Elites\n4: Roulette"),
  VALUE(ELITE_SELECT__ELITE_CNT, size_t, 1, "How many elites get free reproduction passes?"),
  VALUE(TOURNAMENT_SIZE, size_t, 4, "How big are tournaments when using tournament selection or any selection method that uses tournaments?"),
  VALUE(RESOURCE_SELECT__MODE, size_t, 1, "How are resources configured? \n0: Each game phase has an associated resource.\n1: Each individual test case has an associated resource."),
  VALUE(RESOURCE_SELECT__RES_AMOUNT, double, 50.0, "Initial resource amount (for all resources)"),
  VALUE(RESOURCE_SELECT__RES_INFLOW, double, 50.0, "Resource in-flow (amount)"),
  VALUE(RESOURCE_SELECT__OUTFLOW, double, 0.05, "Resource out-flow (rate)"),
  VALUE(RESOURCE_SELECT__FRAC, double, 0.0025, "Fraction of resource consumed."),
  VALUE(RESOURCE_SELECT__MAX_BONUS, double, 5.0, "What's the max bonus someone can get for consuming a resource?"),
  VALUE(RESOURCE_SELECT__COST, double, 0.0, "Cost of using a resource?"),
  VALUE(RESOURCE_SELECT__GAME_PHASE_LEN, size_t, 10, "Game phase interval (defines the number of rounds in each phase)"),
  GROUP(MOVE_SCORING_GROUP, "Move scoring group."),
  VALUE(SCORE_MOVE__ILLEGAL_MOVE_VALUE, double, -5.0, "Score for making illegal move"),
  VALUE(SCORE_MOVE__LEGAL_MOVE_VALUE, double, 1.0, "Score for making a legal move, but not the expert's move"),
  VALUE(SCORE_MOVE__EXPERT_MOVE_VALUE, double, 2.0, "Score for making an expert move"),
  GROUP(OTHELLO_GROUP, "Othello-specific Settings"),
  VALUE(OTHELLO_HW_BOARDS, size_t, 1, "How many dream boards are given to agents for them to manipulate?"),
  GROUP(AGP_PROGRAM_GROUP, "AvidaGP Program Settings"),
  VALUE(AGP_GENOME_SIZE, size_t, 200, "How long should genome be?"),
  GROUP(SGP_PROGRAM_GROUP, "SignalGP program Settings"),
  VALUE(SGP_FUNCTION_LEN, size_t, 50, "Used for generating SGP programs. How long are functions?"),
  VALUE(SGP_FUNCTION_CNT, size_t, 4, "Used for generating SGP programs. How many functions do we generate?"),
  VALUE(SGP_PROG_MAX_LENGTH, size_t, 200, "Maximum length of SGP program (used for variable length programs)"),
  GROUP(SGP_HARDWARE_GROUP, "SignalGP Hardware Settings"),
  VALUE(SGP_HW_MAX_CORES, size_t, 16, "Max number of hardware cores; i.e., max number of simultaneous threads of execution hardware will support."),
  VALUE(SGP_HW_MAX_CALL_DEPTH, size_t, 128, "Max call depth of hardware unit"),
  VALUE(SGP_HW_MIN_BIND_THRESH, double, 0.0, "Hardware minimum referencing threshold"),
  GROUP(AGP_MUTATION_GROUP, "AvidaGP Mutation Settings"),
  VALUE(AGP_PER_INST__SUB_RATE, double, 0.005, "Per-instruction subsitution mutation rate."),
  GROUP(SGP_MUTATION_GROUP, "SignalGP Mutation Settings"),
  VALUE(SGP_VARIABLE_LENGTH, bool, true, "Are SGP programs variable or fixed length?"),
  VALUE(SGP_PROG_MAX_ARG_VAL, int, 16, "Maximum argument value for instructions."),
  VALUE(SGP_PER_BIT__TAG_BFLIP_RATE, double, 0.005, "Per-bit mutation rate of tag bit flips."),
  VALUE(SGP_PER_INST__SUB_RATE, double, 0.005, "Per-instruction/argument subsitution rate."),
  VALUE(SGP_PER_INST__INS_RATE, double, 0.005, "Per-instruction insertion mutation rate."),
  VALUE(SGP_PER_INST__DEL_RATE, double, 0.005, "Per-instruction deletion mutation rate."),
  VALUE(SGP_PER_FUNC__FUNC_DUP_RATE, double, 0.05, "Per-function rate of function duplications."),
  VALUE(SGP_PER_FUNC__FUNC_DEL_RATE, double, 0.05, "Per-function rate of function deletions."),
  GROUP(DATA_GROUP, "Data Collection Settings"),
  VALUE(SYSTEMATICS_INTERVAL, size_t, 100, "Interval to record systematics summary stats."),
  VALUE(FITNESS_INTERVAL, size_t, 100, "Interval to record fitness summary stats."),
  VALUE(POP_SNAPSHOT_INTERVAL, size_t, 5000, "Interval to take a full snapshot of the population."),
  VALUE(DATA_DIRECTORY, std::string, "./", "Location to dump data output."),
  GROUP(ANALYSIS_GROUP, "Analysis settings"),
  VALUE(ANALYSIS_TYPE, size_t, 0, "Which analysis should we run?\n0: Debugging mode"),
  VALUE(ANALYZE_PROGRAM_FPATH, std::string, "analyze.gp", "Which program should we analyze?")
)

#endif
