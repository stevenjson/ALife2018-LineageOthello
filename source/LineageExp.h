#ifndef LINEAGE_EXP_H
#define LINEAGE_EXP_H

// @includes
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <functional>

#include "base/Ptr.h"
#include "base/vector.h"
#include "control/Signal.h"
#include "Evolve/World.h"
#include "Evolve/Resource.h"
#include "games/Othello.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/AvidaGP.h"
#include "hardware/AvidaCPU_InstLib.h"
#include "hardware/InstLib.h"
#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"


#include "TestcaseSet.h"
#include "OthelloHW.h"
#include "lineage-config.h"

// @constants
constexpr int TESTCASE_FILE__DARK_ID = 1;
constexpr int TESTCASE_FILE__LIGHT_ID = -1;
constexpr int TESTCASE_FILE__OPEN_ID = 0;

constexpr size_t SGP__TAG_WIDTH = 16;

constexpr size_t REPRESENTATION_ID__AVIDAGP = 0;
constexpr size_t REPRESENTATION_ID__SIGNALGP = 1;

constexpr size_t TRAIT_ID__MOVE = 0;
constexpr size_t TRAIT_ID__DONE = 1;
constexpr size_t TRAIT_ID__PLAYER_ID = 2;

constexpr size_t RUN_ID__EXP = 0;
constexpr size_t RUN_ID__ANALYSIS = 1;

constexpr size_t ANALYSIS_TYPE_ID__DEBUGGING = 0;

constexpr int AGENT_VIEW__ILLEGAL_ID = -1;
constexpr int AGENT_VIEW__OPEN_ID = 0;
constexpr int AGENT_VIEW__SELF_ID = 1;
constexpr int AGENT_VIEW__OPP_ID = 2;

constexpr size_t SELECTION_METHOD_ID__TOURNAMENT = 0;
constexpr size_t SELECTION_METHOD_ID__LEXICASE = 1;
constexpr size_t SELECTION_METHOD_ID__ECOEA = 2;
constexpr size_t SELECTION_METHOD_ID__MAPELITES = 3;

constexpr size_t POP_INITIALIZATION_METHOD_ID__ANCESTOR_FILE = 0;
constexpr size_t POP_INITIALIZATION_METHOD_ID__RANDOM_POP = 1;

class LineageExp {
public:
  // @aliases
  // SignalGP-specific type aliases:
  using SGP__hardware_t = emp::EventDrivenGP_AW<SGP__TAG_WIDTH>;
  using SGP__program_t = SGP__hardware_t::Program;
  using SGP__state_t = SGP__hardware_t::State;
  using SGP__inst_t = SGP__hardware_t::inst_t;
  using SGP__inst_lib_t = SGP__hardware_t::inst_lib_t;
  using SGP__event_t = SGP__hardware_t::event_t;
  using SGP__event_lib_t = SGP__hardware_t::event_lib_t;
  using SGP__memory_t = SGP__hardware_t::memory_t;
  using SGP__tag_t = SGP__hardware_t::affinity_t;

  // AvidaGP-specific type aliases:
  using AGP__hardware_t = emp::AvidaGP;
  using AGP__program_t = AGP__hardware_t::genome_t;
  using AGP__inst_t = AGP__hardware_t::inst_t;
  using AGP__inst_lib_t = AGP__hardware_t::inst_lib_t;

  struct Agent {
    size_t agent_id;
    size_t GetID() const { return agent_id; }
    void SetID(size_t id) { agent_id = id; }
  };

  struct SignalGPAgent : Agent {
    using Agent::agent_id;
    SGP__program_t program;

    SignalGPAgent(const SGP__program_t & _p)
      : program(_p)
    { ; }

    SignalGPAgent(const SignalGPAgent && in)
      : Agent(in), program(in.program)
    { ; }

    SignalGPAgent(const SignalGPAgent & in)
      : Agent(in), program(in.program)
    { ; }

    SGP__program_t & GetGenome() { return program; }

  };

  struct AvidaGPAgent : Agent {
    using Agent::agent_id;
    AGP__program_t program;

    AvidaGPAgent(const AGP__program_t & _p)
    : Agent(), program(_p)
    { ; }

    AvidaGPAgent(const AvidaGPAgent && in)
      : Agent(in), program(in.program)
    { ; }

    AvidaGPAgent(const AvidaGPAgent & in)
      : Agent(in), program(in.program)
    { ; }

    AGP__program_t & GetGenome() { return program; }
  };

  struct TestcaseInput {
    emp::Othello game;  ///< Othello game board.
    size_t playerID;    ///< From what player is the testcase move made?
    size_t round;       ///< What round is this testcase?

    TestcaseInput(size_t board_width, size_t pID=0) : game(board_width), playerID(pID) { ; }
    TestcaseInput(const TestcaseInput &) = default;
    TestcaseInput(TestcaseInput &&) = default;
  };

  struct TestcaseOutput {
    size_t expert_move;             ///< What move did the expert make on the associated testcase?
    emp::vector<size_t> move_valid;
    // emp::BitVector move_validity;   ///< BoardSize length bitvector describing what moves are valid on the associated testcase?
  };

  // More aliases
  using phenotype_t = emp::vector<double>;
  using data_t = emp::mut_landscape_info<phenotype_t>;
  using mut_count_t = std::unordered_map<std::string, int>;
  using SGP__world_t = emp::World<SignalGPAgent, data_t>;
  using AGP__world_t = emp::World<AvidaGPAgent, data_t>;

protected:
  // == Configurable experiment parameters ==
  // @config_declarations
  // General parameters
  size_t RUN_MODE;
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t EVAL_TIME;
  size_t REPRESENTATION;
  std::string TEST_CASE_FILE;
  std::string ANCESTOR_FPATH;
  size_t POP_INITIALIZATION_METHOD;
  // Selection Group parameters
  size_t SELECTION_METHOD;
  size_t TOURNAMENT_SIZE;
  double RESOURCE_SELECT__RES_AMOUNT;
  double RESOURCE_SELECT__RES_INFLOW;
  double RESOURCE_SELECT__OUTFLOW;
  double RESOURCE_SELECT__FRAC;
  double RESOURCE_SELECT__MAX_BONUS;
  double RESOURCE_SELECT__COST;
  size_t RESOURCE_SELECT__GAME_PHASE_LEN;
  // Scoring Group parameters
  double SCORE_MOVE__ILLEGAL_MOVE_VALUE;
  double SCORE_MOVE__LEGAL_MOVE_VALUE;
  double SCORE_MOVE__EXPERT_MOVE_VALUE;
  // Othello Group parameters
  size_t OTHELLO_BOARD_WIDTH;
  size_t OTHELLO_HW_BOARDS;
  // SignalGP program group parameters
  size_t SGP_FUNCTION_LEN;
  size_t SGP_FUNCTION_CNT;
  // SignalGP Hardware Group parameters
  size_t SGP_HW_MAX_CORES;
  size_t SGP_HW_MAX_CALL_DEPTH;
  double SGP_HW_MIN_BIND_THRESH;
  // SignalGP Mutation Group parameters
  int SGP_PROG_MAX_ARG_VAL;
  double SGP_PER_BIT__TAG_BFLIP_RATE;
  double SGP_PER_INST__SUB_RATE;
  // Data Collection parameters
  size_t SYSTEMATICS_INTERVAL;
  size_t FITNESS_INTERVAL;
  size_t POP_SNAPSHOT_INTERVAL;
  std::string DATA_DIRECTORY;
  // Analysis parameters
  size_t ANALYSIS_TYPE;
  std::string ANALYZE_PROGRAM_FPATH;

  // Experiment variables.
  emp::Ptr<emp::Random> random;

  size_t update;
  size_t eval_time;
  size_t OTHELLO_MAX_ROUND_CNT;

  // Testcases
  TestcaseSet<TestcaseInput,TestcaseOutput> testcases; ///< Test cases are OthelloBoard ==> Expert move
  using test_case_t = typename TestcaseSet<TestcaseInput, TestcaseOutput>::test_case_t;
  size_t cur_testcase;
  // Fitness function sets.
  emp::vector<std::function<double(SignalGPAgent &)>> sgp_lexicase_fit_set; ///< Fit set for SGP lexicase selection.
  emp::vector<std::function<double(AvidaGPAgent &)>> agp_lexicase_fit_set;  ///< Fit set for AGP lexicase selection.
  emp::vector<std::function<double(SignalGPAgent &)>> sgp_resource_fit_set; ///< Fit set for SGP resource selection.
  emp::vector<std::function<double(AvidaGPAgent &)>> agp_resource_fit_set;  ///< Fit set for AGP resource selection.

  // emp::BitVector testcase_eval_cache;
  emp::vector<size_t> testcase_eval_cach;
  emp::vector<double> testcase_score_cache;
  emp::vector<double> agent_score_cache;

  emp::vector<emp::vector<size_t>> testcases_by_phase;
  emp::vector<emp::Resource> resources;

  emp::Ptr<OthelloHardware> othello_dreamware; ///< Othello game board dreamware!

  // SignalGP-specifics.
  emp::Ptr<SGP__world_t> sgp_world;         ///< World for evolving SignalGP agents.
  emp::Ptr<SGP__inst_lib_t> sgp_inst_lib;   ///< SignalGP instruction library.
  emp::Ptr<SGP__event_lib_t> sgp_event_lib; ///< SignalGP event library.
  emp::Ptr<SGP__hardware_t> sgp_eval_hw;    ///< Hardware used to evaluate SignalGP programs during evolution/analysis.

  // AvidaGP-specifics.
  emp::Ptr<AGP__world_t> agp_world;         ///< World for evolving AvidaGP agents.
  emp::Ptr<AGP__inst_lib_t> agp_inst_lib;   ///< AvidaGP instruction library.
  emp::Ptr<AGP__hardware_t> agp_eval_hw;    ///< Hardware used to evaluate AvidaGP programs during evolution/analysis.

  // --- Experiment signals ---
  // All of these are hardware-specific.
  // - DoBeginRun: this is where you'll setup systematics, fitness file, etc. with the world.
  emp::Signal<void(void)> do_begin_run_setup_sig; ///< Shared between AGP and SGP
  emp::Signal<void(void)> do_pop_init_sig;
  // - DoEvaluation: evaluate e'rybody!
  emp::Signal<void(void)> do_evaluation_sig;
  // - DoSelection: Do selection.
  emp::Signal<void(void)> do_selection_sig;
  // - DoUpdate: Responsible for calling world->Update()
  emp::Signal<void(void)> do_world_update_sig;
  // - DoAnalysis
  emp::Signal<void(void)> do_analysis_sig;
  // - Snapshot?
  emp::Signal<void(size_t)> do_pop_snapshot_sig;

  //
  emp::Signal<void(mut_count_t)> on_mutate_sig;                        ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, double)> record_fit_sig;                ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, phenotype_t)> record_phen_sig;  ///< Trigger signal before organism gives birth.

  // --- Agent evaluation signals ---
  // othello_begin_turn
  emp::Signal<void(const emp::Othello &)> begin_turn_sig;
  emp::Signal<void(void)> agent_advance_sig;

  // Functors!
  // - Get move
  std::function<size_t(void)> get_eval_agent_move;              ///< Hardware-specific!
  // - Get done
  std::function<bool(void)> get_eval_agent_done;                ///< Hardware-specific!
  // - Get player ID
  std::function<size_t(void)> get_eval_agent_playerID;          ///< Hardware-specific!
  // - Given player move and test case, calculate agent score.
  std::function<double(test_case_t &, size_t)> calc_test_score; ///< Shared between hardware types.

  // Protected functions.
  /// Evaluate GP move (hardware-agnostic).
  /// Requires that the following signals/functors be setup:
  ///  - begin_turn_sig
  ///  - agent_advance_sig
  ///  - get_eval_agent_done
  ///  - get_eval_agent_move
  ///  - get_eval_agent_playerID
  size_t EvalMove__GP(emp::Othello & game, bool promise_validity=false) {
    // Signal begin_turn
    begin_turn_sig.Trigger(game);
    // Run agent until time is up or until agent indicates it is done evaluating.
    for (eval_time = 0; eval_time < EVAL_TIME && !get_eval_agent_done(); ++eval_time) {
      agent_advance_sig.Trigger();
    }
    // Extract agent's move.
    size_t move = get_eval_agent_move();
    // Did we promise a valid move?
    if (promise_validity) {
      // Double-check move validity.
      const size_t playerID = get_eval_agent_playerID();
      if (!game.IsMoveValid(playerID, move)) {
        // Move is not valid. Needs to be fixed, so set it to the nearest valid move.
        emp::vector<size_t> valid_moves = game.GetMoveOptions(playerID);
        const size_t move_x = game.GetPosX(move);
        const size_t move_y = game.GetPosY(move);
        size_t new_move_x = 0;
        size_t new_move_y = 0;
        size_t sq_move_dist = game.GetBoard().size() * game.GetBoard().size();
        for (size_t i = 0; i < valid_moves.size(); ++i) {
          const size_t proposed_x = game.GetPosX(valid_moves[i]);
          const size_t proposed_y = game.GetPosY(valid_moves[i]);
          const size_t proposed_dist = std::pow(proposed_x - move_x, 2) + std::pow(proposed_y - move_y, 2);
          if (proposed_dist < sq_move_dist) {
            new_move_x = proposed_x; new_move_y = proposed_y; sq_move_dist = proposed_dist;
          }
        }
        move = game.GetPosID(new_move_x, new_move_y);
      }
    }
    return move;
  }

  /// Returns a random valid move.
  size_t EvalMove__Random(emp::Othello & game, size_t playerID, bool promise_validity=false) {
    emp::vector<size_t> options = game.GetMoveOptions(playerID);
    return options[random->GetUInt(0, options.size())];
  }

  /// From vector of strings pulled from a single line of a testcases file, generate a single
  /// test case.
  /// Expected line contents: game board positions, player ID, expert move, round
  test_case_t GenerateTestcase(emp::vector<std::string> & test_case_strings) {
    // Build test case input.
    TestcaseInput input(OTHELLO_BOARD_WIDTH);
    emp::Othello & game = input.game;
    // test_case_strings expectation: game_board_positions, round, playerID, expert_move
    emp_assert(test_case_strings.size() == (game.GetBoardSize() + 3));

    // Get the round.
    size_t rnd = std::atoi(test_case_strings.back().c_str());
    test_case_strings.resize(test_case_strings.size() - 1);

    // Get the expert move.
    size_t expert_move = std::atoi(test_case_strings.back().c_str());
    test_case_strings.resize(test_case_strings.size() - 1);

    // Get the playerID.
    int id = std::atoi(test_case_strings.back().c_str());
    size_t playerID = (id == TESTCASE_FILE__DARK_ID) ? emp::Othello::DarkPlayerID() : emp::Othello::LightPlayerID();
    test_case_strings.resize(test_case_strings.size() - 1);

    for (size_t i = 0; i < test_case_strings.size(); ++i) {
      int board_space = std::atoi(test_case_strings[i].c_str());
      switch (board_space) {
        case TESTCASE_FILE__DARK_ID:
          game.SetPos(i, emp::Othello::DarkDisk());
          break;
        case TESTCASE_FILE__LIGHT_ID:
          game.SetPos(i, emp::Othello::LightDisk());
          break;
        case TESTCASE_FILE__OPEN_ID:
          game.SetPos(i, emp::Othello::OpenSpace());
          break;
        default:
          std::cout << "Unrecognized board tile! Exiting..." << std::endl;
          exit(-1);
      }
    }
    input.round = rnd;
    input.playerID = playerID;

    // Fill out testcase output.
    TestcaseOutput output;
    output.expert_move = expert_move;
    emp::vector<size_t> valid_moves = game.GetMoveOptions(playerID);
    output.move_valid.resize(game.GetBoardSize());   // Resize bit vector to match board size.
    for (size_t i = 0; i < output.move_valid.size(); ++i) output.move_valid[i] = 0;
    for (size_t i = 0; i < valid_moves.size(); ++i) {
      output.move_valid[valid_moves[i]] = 1;
    }
    return test_case_t(input, output);
  }

  /// Get appropriate score cache ID.
  size_t GetCacheID(size_t agentID, size_t testID) {
    return (agentID * testcases.GetSize()) + testID;
  }
  /// Run all tests on current eval hardware.
  void Evaluate(Agent & agent) {
    const size_t id = agent.GetID();
    // Evaluate agent on all test cases.
    double score = 0.0;
    for (cur_testcase = 0; cur_testcase < testcases.GetSize(); ++cur_testcase) {
      score += this->RunTest(id, cur_testcase);
    }
    agent_score_cache[id] = score;
  }
  /// Run test return test score.
  double RunTest(size_t agentID, size_t testID) {
    const size_t cID = GetCacheID(agentID, testID);
    // If we haven't cached this test case yet, go ahead and calculate it.
    if (!testcase_eval_cach[cID]) {
      test_case_t & test = testcases[testID];
      size_t move = EvalMove__GP(test.GetInput().game, false);
      double score = calc_test_score(test, move);
      testcase_eval_cach[cID] = 1;
      testcase_score_cache[cID] = score;
    }
    return testcase_score_cache[cID];
  }
  /// Clear cache.
  void ClearCache() {
    for (size_t i = 0; i < testcase_eval_cach.size(); ++i) { testcase_eval_cach[i] = 0; }
  }


public:
  // @constructor
  LineageExp(const LineageConfig & config)
    : update(0), eval_time(0), OTHELLO_MAX_ROUND_CNT(0), testcases(), cur_testcase(0)
  {
    RUN_MODE = config.RUN_MODE();
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    EVAL_TIME = config.EVAL_TIME();
    REPRESENTATION = config.REPRESENTATION();
    TEST_CASE_FILE = config.TEST_CASE_FILE();
    ANCESTOR_FPATH = config.ANCESTOR_FPATH();
    POP_INITIALIZATION_METHOD = config.POP_INITIALIZATION_METHOD();
    SELECTION_METHOD = config.SELECTION_METHOD();
    TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    RESOURCE_SELECT__RES_AMOUNT = config.RESOURCE_SELECT__RES_AMOUNT();
    RESOURCE_SELECT__RES_INFLOW = config.RESOURCE_SELECT__RES_INFLOW();
    RESOURCE_SELECT__OUTFLOW = config.RESOURCE_SELECT__OUTFLOW();
    RESOURCE_SELECT__FRAC = config.RESOURCE_SELECT__FRAC();
    RESOURCE_SELECT__MAX_BONUS = config.RESOURCE_SELECT__MAX_BONUS();
    RESOURCE_SELECT__COST = config.RESOURCE_SELECT__COST();
    RESOURCE_SELECT__GAME_PHASE_LEN = config.RESOURCE_SELECT__GAME_PHASE_LEN();
    SCORE_MOVE__ILLEGAL_MOVE_VALUE = config.SCORE_MOVE__ILLEGAL_MOVE_VALUE();
    SCORE_MOVE__LEGAL_MOVE_VALUE = config.SCORE_MOVE__LEGAL_MOVE_VALUE();
    SCORE_MOVE__EXPERT_MOVE_VALUE = config.SCORE_MOVE__EXPERT_MOVE_VALUE();
    OTHELLO_BOARD_WIDTH = config.OTHELLO_BOARD_WIDTH();
    OTHELLO_HW_BOARDS = config.OTHELLO_HW_BOARDS();
    SGP_FUNCTION_LEN = config.SGP_FUNCTION_LEN();
    SGP_FUNCTION_CNT = config.SGP_FUNCTION_CNT();
    SGP_HW_MAX_CORES = config.SGP_HW_MAX_CORES();
    SGP_HW_MAX_CALL_DEPTH = config.SGP_HW_MAX_CALL_DEPTH();
    SGP_HW_MIN_BIND_THRESH = config.SGP_HW_MIN_BIND_THRESH();
    SGP_PROG_MAX_ARG_VAL = config.SGP_PROG_MAX_ARG_VAL();
    SGP_PER_BIT__TAG_BFLIP_RATE = config.SGP_PER_BIT__TAG_BFLIP_RATE();
    SGP_PER_INST__SUB_RATE = config.SGP_PER_INST__SUB_RATE();
    SYSTEMATICS_INTERVAL = config.SYSTEMATICS_INTERVAL();
    FITNESS_INTERVAL = config.FITNESS_INTERVAL();
    POP_SNAPSHOT_INTERVAL = config.POP_SNAPSHOT_INTERVAL();
    DATA_DIRECTORY = config.DATA_DIRECTORY();
    ANALYSIS_TYPE = config.ANALYSIS_TYPE();
    ANALYZE_PROGRAM_FPATH = config.ANALYZE_PROGRAM_FPATH();

    // Make a random number generator.
    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    // What is the maximum number of rounds for an othello game?
    OTHELLO_MAX_ROUND_CNT = (OTHELLO_BOARD_WIDTH * OTHELLO_BOARD_WIDTH) - 4;

    // Load test cases.
    testcases.RegisterTestcaseReader([this](emp::vector<std::string> & strs) { return this->GenerateTestcase(strs); });
    testcases.LoadTestcases(TEST_CASE_FILE);


    // Setup testcase cache.
    testcase_eval_cach.resize(POP_SIZE * testcases.GetSize(), 0);
    testcase_score_cache.resize(POP_SIZE * testcases.GetSize(), 0.0);
    agent_score_cache.resize(POP_SIZE, 0.0);

    // Organize testcase IDs into phases.
    // - How many phases are we working with?
    int bucket_cnt = std::ceil(((double)OTHELLO_MAX_ROUND_CNT)/((double)RESOURCE_SELECT__GAME_PHASE_LEN));
    testcases_by_phase.resize(bucket_cnt);
    // - What phase does each test case belong to?
    for (size_t i = 0; i < testcases.GetSize(); ++i) {
      size_t rd = testcases[i].GetInput().round;
      size_t phase = emp::Min(rd/RESOURCE_SELECT__GAME_PHASE_LEN, testcases_by_phase.size() - 1);
      testcases_by_phase[phase].emplace_back(i);
    }
    // Fill out resources.
    for (size_t i = 0; i < bucket_cnt; ++i) {
      resources.emplace_back(RESOURCE_SELECT__RES_AMOUNT, RESOURCE_SELECT__RES_INFLOW, RESOURCE_SELECT__OUTFLOW);
    }

    // Print out test case distribution.
    std::cout << "Test case phase distribution (phase:size):";
    for (size_t i = 0; i < testcases_by_phase.size(); ++i) {
      std::cout << " " << i << ":" << testcases_by_phase[i].size();
    } std::cout << std::endl;

    // Given a test case and a move, how are we scoring an agent?
    calc_test_score = [this](test_case_t & test, size_t move) {
      // Score move given test.
      if (move == test.GetOutput().expert_move) {
        return SCORE_MOVE__EXPERT_MOVE_VALUE;
      } else if (move >= test.GetOutput().move_valid.size()) {
        return SCORE_MOVE__ILLEGAL_MOVE_VALUE;
      } else {
        // std::cout << "Move is in a valid position. Is it a valid move? " << test.GetOutput().move_valid[move] << std::endl;
        return (test.GetOutput().move_valid[move]) ? SCORE_MOVE__LEGAL_MOVE_VALUE : SCORE_MOVE__ILLEGAL_MOVE_VALUE;
      }
    };

    for (size_t i = 0; i < testcases.GetSize(); ++i) {
      std::cout << "============= Test case: " << i << " =============" << std::endl;
      std::cout << "ID: " << testcases[i].id << std::endl;
      std::cout << "Input" << std::endl;
      // Board
      testcases[i].GetInput().game.Print();
      auto options = testcases[i].GetInput().game.GetMoveOptions(testcases[i].GetInput().playerID);
      std::cout << "Board options: ";
      for (size_t j = 0; j < options.size(); ++j) {
        std::cout << " " << options[j];
      } std::cout << std::endl;
      std::cout << "Valid options: ";
      for (size_t j = 0; j < testcases[i].GetOutput().move_valid.size(); ++j) {
        std::cout << " " << testcases[i].GetOutput().move_valid[j];
      } std::cout << std::endl;
      std::cout << "Board width: " << testcases[i].GetInput().game.GetBoardWidth() << std::endl;
      std::cout << "Round: " << testcases[i].GetInput().round << std::endl;
      std::cout << "PlayerID: " << testcases[i].GetInput().playerID << std::endl;
      std::cout << "Output" << std::endl;
      std::cout << "Expert move: " << testcases[i].GetOutput().expert_move << std::endl;
    }

    // Configure the dreamware!
    othello_dreamware = emp::NewPtr<OthelloHardware>(OTHELLO_BOARD_WIDTH, 1);

    // Make the world(s)!
    // - SGP World -
    sgp_world = emp::NewPtr<SGP__world_t>(random, "SGP-LineageAnalysis-World");
    agp_world = emp::NewPtr<AGP__world_t>(random, "AGP-LineageAnalysis-World");

    // Configure instruction/event libraries.
    sgp_inst_lib = emp::NewPtr<SGP__inst_lib_t>();
    sgp_event_lib = emp::NewPtr<SGP__event_lib_t>();
    agp_inst_lib = emp::NewPtr<AGP__inst_lib_t>();

    if (RUN_MODE == RUN_ID__EXP) {
      // Make data directory.
      mkdir(DATA_DIRECTORY.c_str(), ACCESSPERMS);
      if (DATA_DIRECTORY.back() != '/') DATA_DIRECTORY += '/';
    }

    // Config experiment based on representation type.
    switch (REPRESENTATION) {
      case REPRESENTATION_ID__AVIDAGP:
        ConfigAGP();
        break;
      case REPRESENTATION_ID__SIGNALGP:
        ConfigSGP();
        break;
      default:
        std::cout << "Unrecognized representation configuration setting (" << REPRESENTATION << "). Exiting..." << std::endl;
        exit(-1);
    }
  }

  ~LineageExp() {
    random.Delete();
    othello_dreamware.Delete();
    sgp_world.Delete();
    agp_world.Delete();
    sgp_inst_lib.Delete();
    agp_inst_lib.Delete();
    sgp_event_lib.Delete();
    sgp_eval_hw.Delete();
    agp_eval_hw.Delete();

    // TODO: clean up whatever Avida-specific dynamically allocated memory we end up using.
  }

  void ConfigSGP();
  void ConfigAGP();

  void Run() {
    switch (RUN_MODE) {
      case RUN_ID__EXP: {
        do_begin_run_setup_sig.Trigger();
        for (update = 0; update <= GENERATIONS; ++update) {
          RunStep();
          if (update % POP_SNAPSHOT_INTERVAL == 0) do_pop_snapshot_sig.Trigger(update);
        }
        break;
      }
      case RUN_ID__ANALYSIS:
        do_analysis_sig.Trigger();
        break;
      default:
        std::cout << "Unrecognized run mode! Exiting..." << std::endl;
        exit(-1);
    }
  }

  void RunStep() {
    do_evaluation_sig.Trigger();    // Update agent scores.
    do_selection_sig.Trigger();     // Do selection (selection, reproduction, mutation).
    do_world_update_sig.Trigger();  // Do world update (population turnover, clear score caches).
  }

  // Fitness functions
  // -- Also, probably want to cache this value as well.
  double CalcFitness(Agent & agent) {
    return agent_score_cache[agent.GetID()];
  }

  // Mutation functions
  size_t SGP__Mutate(SignalGPAgent & agent, emp::Random & rnd);
  size_t AGP__Mutate(AvidaGPAgent & agent, emp::Random & rnd);  // TODO

  // Population snapshot functions
  void SGP_Snapshot_SingleFile(size_t update);

  // SignalGP utility functions.
  void SGP__InitPopulation_Random();
  void SGP__InitPopulation_FromAncestorFile();
  void SGP__ResetHW(const SGP__memory_t & main_in_mem=SGP__memory_t());

  // SignalGP Analysis functions.
  void SGP__Debugging_Analysis();

  // -- AvidaGP Instructions --
  // BoardWidth
  void AGP_Inst_GetBoardWidth(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // EndTurn
  void AGP_Inst_EndTurn(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // SetMove
  void AGP__Inst_SetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_SetMoveID(AGP__hardware_t &hw, const SGP__inst_t &inst);
  // GetMove
  void AGP__Inst_GetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_GetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // IsValid
  void AGP__Inst_IsValidXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_IsValidID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // Adjacent
  void AGP__Inst_AdjacentXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_AdjacentID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ValidMovesCnt
  void AGP_Inst_ValidMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ValidOppMovesCnt
  void AGP_Inst_ValidOppMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // GetBoardValue
  void AGP_Inst_GetBoardValueXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_GetBoardValueID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // Place
  void AGP_Inst_PlaceDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_PlaceDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // PlaceOpp
  void AGP_Inst_PlaceOppDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_PlaceOppDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // FlipCnt
  void AGP_Inst_FlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_FlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // OppFlipCnt
  void AGP_Inst_OppFlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_OppFlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // FrontierCnt
  void AGP_Inst_FrontierCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP_Inst_FrontierCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ResetBoard
  void AGP_Inst_ResetBoard_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // IsOver
  void AGP_Inst_IsOver_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);

  // -- SignalGP Instructions --
  // Fork
  void SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // BoardWidth
  void SGP_Inst_GetBoardWidth(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // EndTurn
  void SGP_Inst_EndTurn(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // SetMove
  void SGP__Inst_SetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_SetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetMove
  void SGP__Inst_GetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_GetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // Adjacent
  void SGP__Inst_AdjacentXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_AdjacentID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsValid
  void SGP__Inst_IsValidXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_IsValidID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsValidOpp
  void SGP__Inst_IsValidOppXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_IsValidOppID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidMovesCnt
  void SGP__Inst_ValidMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidOppMovesCnt
  void SGP__Inst_ValidOppMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetBoardValue
  void SGP__Inst_GetBoardValueXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_GetBoardValueID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // Place
  void SGP__Inst_PlaceDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_PlaceDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // PlaceOpp
  void SGP__Inst_PlaceOppDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_PlaceOppDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FlipCnt
  void SGP__Inst_FlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_FlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // OppFlipCnt
  void SGP__Inst_OppFlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_OppFlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FrontierCnt
  void SGP__Inst_FrontierCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ResetBoard
  void SGP__Inst_ResetBoard_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsOver
  void SGP__Inst_IsOver_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
};

// SignalGP Functions
/// Reset the SignalGP evaluation hardware, setting input memory of
/// main thread to be equal to main_in_mem.
void LineageExp::SGP__ResetHW(const SGP__memory_t & main_in_mem) {
  sgp_eval_hw->ResetHardware();
  sgp_eval_hw->SetTrait(TRAIT_ID__MOVE, -1);
  sgp_eval_hw->SetTrait(TRAIT_ID__DONE, 0);
  sgp_eval_hw->SetTrait(TRAIT_ID__PLAYER_ID, -1);
  sgp_eval_hw->SpawnCore(0, main_in_mem, true);
}

void LineageExp::SGP__InitPopulation_Random() {
  std::cout << "Initializing population randomly!" << std::endl;
  for (size_t p = 0; p < POP_SIZE; ++p) {
    SGP__program_t prog(sgp_inst_lib);
    for (size_t f = 0; f < SGP_FUNCTION_CNT; ++f) {
      prog.PushFunction();
      prog[f].affinity.Randomize(*random);
      for (size_t i = 0; i < SGP_FUNCTION_LEN; ++i) {
        const size_t instID = random->GetUInt(sgp_inst_lib->GetSize());
        const size_t a0 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        const size_t a1 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        const size_t a2 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        SGP__inst_t inst(instID, a0, a1, a2);
        inst.affinity.Randomize(*random);
        prog[f].PushInst(inst);
      }
    }
    sgp_world->Inject(prog,1);
  }
}

void LineageExp::SGP__InitPopulation_FromAncestorFile() {
  std::cout << "Initializing population from ancestor file!" << std::endl;
  // Configure the ancestor program.
  SGP__program_t ancestor_prog(sgp_inst_lib);
  std::ifstream ancestor_fstream(ANCESTOR_FPATH);
  if (!ancestor_fstream.is_open()) {
    std::cout << "Failed to open ancestor program file(" << ANCESTOR_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  ancestor_prog.Load(ancestor_fstream);
  std::cout << " --- Ancestor program: ---" << std::endl;
  ancestor_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;
  sgp_world->Inject(ancestor_prog, POP_SIZE);    // Inject a bunch of ancestors into the population.
}

/// Analysis mode used for tracing program.
/// Primary purpose of this analysis type is to help debug this experiment. Woo!
void LineageExp::SGP__Debugging_Analysis() {
  std::cout << "\nRunning DEBUGGING analysis...\n" << std::endl;
  // Configure analysis program.
  SGP__program_t analyze_prog(sgp_inst_lib);
  std::ifstream analyze_fstream(ANALYZE_PROGRAM_FPATH);
  if (!analyze_fstream.is_open()) {
    std::cout << "Failed to open analysis program file(" << ANALYZE_PROGRAM_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  analyze_prog.Load(analyze_fstream);
  std::cout << " --- Analysis program: ---" << std::endl;
  analyze_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;

  ClearCache();

  // Load program onto agent.
  SignalGPAgent our_hero(analyze_prog);
  our_hero.SetID(0);
  sgp_eval_hw->SetProgram(our_hero.GetGenome());
  // this->Evaluate(our_hero);
  double score = 0.0;
  for (cur_testcase = 0; cur_testcase < testcases.GetSize(); ++cur_testcase) {
    double test_score = RunTest(our_hero.GetID(), cur_testcase);
    std::cout << "TEST CASE " << cur_testcase << " SCORE: " << test_score << std::endl;
    score += test_score;
    // How did it do?
  }
  agent_score_cache[our_hero.GetID()] = score;

  std::cout << "\n\nFINAL SCORE (total): " << score << std::endl;
  std::cout << "Test case scores: {";
  for (size_t i = 0; i < testcases.GetSize(); ++i) {
    std::cout << "Test " << i << ": " << testcase_score_cache[GetCacheID(our_hero.GetID(), i)] << ", ";
  } std::cout << "}" << std::endl;
}

/// Mutate an SGP agent's program. Only does tag mutations, instruction substitutions, and
/// argument substitutions. (maintains constand-length genomes)
size_t LineageExp::SGP__Mutate(SignalGPAgent & agent, emp::Random & rnd) {
  SGP__program_t & program = agent.GetGenome();
  size_t mut_cnt = 0;
  // For each function:
  for (size_t fID = 0; fID < program.GetSize(); ++fID) {
    // Mutate affinity.
    for (size_t i = 0; i < program[fID].GetAffinity().GetSize(); ++i) {
      SGP__tag_t & aff = program[fID].GetAffinity();
      if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
        ++mut_cnt;
        aff.Set(i, !aff.Get(i));
      }
    }
    // Substitutions?
    for (size_t i = 0; i < program[fID].GetSize(); ++i) {
      SGP__inst_t & inst = program[fID][i];
      // Mutate affinity (even if it doesn't have one).
      for (size_t k = 0; k < inst.affinity.GetSize(); ++k) {
        if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
          ++mut_cnt;
          inst.affinity.Set(k, !inst.affinity.Get(k));
        }
      }
      // Mutate instruction.
      if (rnd.P(SGP_PER_INST__SUB_RATE)) {
        ++mut_cnt;
        inst.id = rnd.GetUInt(program.GetInstLib()->GetSize());
      }
      // Mutate arguments (even if they aren't relevent to instruction).
      for (size_t k = 0; k < SGP__hardware_t::MAX_INST_ARGS; ++k) {
        if (rnd.P(SGP_PER_INST__SUB_RATE)) {
          ++mut_cnt;
          inst.args[k] = rnd.GetInt(SGP_PROG_MAX_ARG_VAL);
        }
      }
    }
  }
  return mut_cnt;
}

void LineageExp::SGP_Snapshot_SingleFile(size_t update) {
  std::string snapshot_dir = DATA_DIRECTORY + "pop_" + emp::to_string((int)update);
  mkdir(snapshot_dir.c_str(), ACCESSPERMS);
  // For each program in the population, dump the full program description in a single file.
  std::ofstream prog_ofstream(snapshot_dir + "/pop_" + emp::to_string((int)update) + ".pop");
  for (size_t i = 0; i < sgp_world->GetSize(); ++i) {
    if (i) prog_ofstream << "===\n";
    SignalGPAgent & agent = sgp_world->GetOrg(i);
    agent.program.PrintProgramFull(prog_ofstream);
  }
  prog_ofstream.close();
}

// --- SGP instruction implementations ---
// SGP__Inst_Fork
void LineageExp::SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  hw.SpawnCore(inst.affinity, hw.GetMinBindThresh(), state.local_mem);
}
// SGP_Inst_GetBoardWidth
void LineageExp::SGP_Inst_GetBoardWidth(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], OTHELLO_BOARD_WIDTH);
}
// SGP_Inst_EndTurn
void LineageExp::SGP_Inst_EndTurn(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  hw.SetTrait(TRAIT_ID__DONE, 1);
}
// SGP__Inst_SetMoveXY
void LineageExp::SGP__Inst_SetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t move = dreamboard.GetPosID(move_x, move_y);
  hw.SetTrait(TRAIT_ID__MOVE, move);
}
// SGP__Inst_SetMoveID
void LineageExp::SGP__Inst_SetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  hw.SetTrait(TRAIT_ID__MOVE, move_id);
}
// SGP__Inst_GetMoveXY
void LineageExp::SGP__Inst_GetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  const size_t move_x = dreamboard.GetPosX(move_id);
  const size_t move_y = dreamboard.GetPosY(move_id);
  state.SetLocal(inst.args[0], move_x);
  state.SetLocal(inst.args[1], move_y);
}
// SGP__Inst_GetMoveID
void LineageExp::SGP__Inst_GetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  state.SetLocal(inst.args[0], move_id);
}
// SGP__Inst_IsValidXY
void LineageExp::SGP__Inst_IsValidXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_x, move_y);
  state.SetLocal(inst.args[2], valid);
}
// SGP__Inst_IsValidID_HW
void LineageExp::SGP__Inst_IsValidID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t move_id = state.GetLocal(inst.args[0]);
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_id);
  state.SetLocal(inst.args[1], valid);
}
// SGP__Inst_IsValidOppXY
void LineageExp::SGP__Inst_IsValidOppXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const int valid = (int)dreamboard.IsMoveValid(oppID, move_x, move_y);
  state.SetLocal(inst.args[2], valid);
}
// SGP__Inst_IsValidOppID
void LineageExp::SGP__Inst_IsValidOppID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  const size_t move_id = state.GetLocal(inst.args[0]);
  const int valid = (int)dreamboard.IsMoveValid(oppID, move_id);
  state.SetLocal(inst.args[1], valid);
}
// SGP__Inst_AdjacentXY
void LineageExp::SGP__Inst_AdjacentXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t dir    = emp::Mod((int)state.GetLocal(inst.args[2]), 8);
  const int nID = dreamboard.GetNeighbor(move_x, move_y, dir);
  if (nID == -1) {
    state.SetLocal(inst.args[0], AGENT_VIEW__ILLEGAL_ID);
    state.SetLocal(inst.args[1], AGENT_VIEW__ILLEGAL_ID);
  } else {
    state.SetLocal(inst.args[0], dreamboard.GetPosX(nID));
    state.SetLocal(inst.args[1], dreamboard.GetPosY(nID));
  }
}
// SGP__Inst_AdjacentID
void LineageExp::SGP__Inst_AdjacentID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  const size_t dir     = emp::Mod((int)state.GetLocal(inst.args[2]), 8);
  const int nID        = dreamboard.GetNeighbor(move_id, dir);
  state.SetLocal(inst.args[0], (nID == -1) ? AGENT_VIEW__ILLEGAL_ID : nID);
}
// SGP_Inst_ValidMoveCnt_HW
void LineageExp::SGP__Inst_ValidMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  state.SetLocal(inst.args[0], dreamboard.GetMoveOptions(playerID).size());
}
// SGP_Inst_ValidOppMoveCnt_HW
void LineageExp::SGP__Inst_ValidOppMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  state.SetLocal(inst.args[0], dreamboard.GetMoveOptions(oppID).size());
}
// SGP_Inst_GetBoardValueXY_HW
void LineageExp::SGP__Inst_GetBoardValueXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  // If inputs are garbage, let the caller know.
  if (dreamboard.IsValidPos(move_x, move_y)) {
    const size_t owner = dreamboard.GetPosOwner(move_x, move_y);
    if (owner == playerID) { state.SetLocal(inst.args[2], AGENT_VIEW__SELF_ID); }
    else if (owner == oppID) { state.SetLocal(inst.args[2], AGENT_VIEW__OPP_ID); }
    else { state.SetLocal(inst.args[2], AGENT_VIEW__OPEN_ID); }
  } else {
    state.SetLocal(inst.args[2], AGENT_VIEW__ILLEGAL_ID);
  }
}
// SGP_Inst_GetBoardValueID_HW
void LineageExp::SGP__Inst_GetBoardValueID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = state.GetLocal(inst.args[0]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  // If inputs are garbage, let the caller know.
  if (dreamboard.IsValidPos(move_id)) {
    const size_t owner = dreamboard.GetPosOwner(move_id);
    if (owner == playerID) { state.SetLocal(inst.args[1], AGENT_VIEW__SELF_ID); }
    else if (owner == oppID) { state.SetLocal(inst.args[1], AGENT_VIEW__OPP_ID); }
    else { state.SetLocal(inst.args[1], AGENT_VIEW__OPEN_ID); }
  } else {
    state.SetLocal(inst.args[1], AGENT_VIEW__ILLEGAL_ID);
  }
}
// SGP_Inst_PlaceDiskXY_HW
void LineageExp::SGP__Inst_PlaceDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_x, move_y)) {
    dreamboard.DoMove(playerID, move_x, move_y);
    state.SetLocal(inst.args[2], 1);
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_PlaceDiskID_HW
void LineageExp::SGP__Inst_PlaceDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = state.GetLocal(inst.args[0]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id)) {
    dreamboard.DoMove(playerID, move_id);
    state.SetLocal(inst.args[1], 1);
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_PlaceOppDiskXY_HW
void LineageExp::SGP__Inst_PlaceOppDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_x, move_y)) {
    dreamboard.DoMove(oppID, move_x, move_y);
    state.SetLocal(inst.args[2], 1);
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_PlaceOppDiskID_HW
void LineageExp::SGP__Inst_PlaceOppDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = state.GetLocal(inst.args[0]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id)) {
    dreamboard.DoMove(oppID, move_id);
    state.SetLocal(inst.args[1], 1);
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_FlipCntXY_HW
void LineageExp::SGP__Inst_FlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t move_id = dreamboard.GetPosID(move_x, move_y);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id)) {
    state.SetLocal(inst.args[2], dreamboard.GetFlipList(playerID, move_id).size());
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_FlipCntID_HW
void LineageExp::SGP__Inst_FlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id)) {
    state.SetLocal(inst.args[1], dreamboard.GetFlipList(playerID, move_id).size());
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_OppFlipCntXY_HW
void LineageExp::SGP__Inst_OppFlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const size_t move_id = dreamboard.GetPosID(move_x, move_y);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id)) {
    state.SetLocal(inst.args[2], dreamboard.GetFlipList(oppID, move_id).size());
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_OppFlipCntID_HW
void LineageExp::SGP__Inst_OppFlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id)) {
    state.SetLocal(inst.args[1], dreamboard.GetFlipList(oppID, move_id).size());
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_FrontierCnt_HW
void LineageExp::SGP__Inst_FrontierCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  state.SetLocal(inst.args[0], dreamboard.GetFrontierPosCnt(playerID));
}
// SGP_Inst_ResetBoard_HW
void LineageExp::SGP__Inst_ResetBoard_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  othello_dreamware->ResetActive(testcases[cur_testcase].GetInput().game);
}
// SGP_Inst_IsOver_HW
void LineageExp::SGP__Inst_IsOver_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  emp::Othello & dreamboard = othello_dreamware->GetActiveDreamOthello();
  state.SetLocal(inst.args[0], (int)dreamboard.IsOver());
}

void LineageExp::ConfigSGP() {
  // Configure the world.
  sgp_world->Reset();
  sgp_world->SetWellMixed(true);
  // NOTE: second argument specifies that we're not mutating the first thing int the pop (we're doing elite selection in all of our stuff).
  sgp_world->SetMutFun([this](SignalGPAgent & agent, emp::Random & rnd) { return this->SGP__Mutate(agent, rnd); }, 1);
  sgp_world->SetFitFun([this](SignalGPAgent & agent) { return this->CalcFitness(agent); });

  // Configure the instruction set.
  // - Default instruction set.
  sgp_inst_lib->AddInst("Inc", SGP__hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  sgp_inst_lib->AddInst("Dec", SGP__hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  sgp_inst_lib->AddInst("Not", SGP__hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  sgp_inst_lib->AddInst("Add", SGP__hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  sgp_inst_lib->AddInst("Sub", SGP__hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  sgp_inst_lib->AddInst("Mult", SGP__hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  sgp_inst_lib->AddInst("Div", SGP__hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  sgp_inst_lib->AddInst("Mod", SGP__hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  sgp_inst_lib->AddInst("TestEqu", SGP__hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  sgp_inst_lib->AddInst("TestNEqu", SGP__hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  sgp_inst_lib->AddInst("TestLess", SGP__hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  sgp_inst_lib->AddInst("If", SGP__hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("While", SGP__hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("Countdown", SGP__hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("Close", SGP__hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  sgp_inst_lib->AddInst("Break", SGP__hardware_t::Inst_Break, 0, "Break out of current block.");
  sgp_inst_lib->AddInst("Call", SGP__hardware_t::Inst_Call, 0, "Call function that best matches call affinity.", emp::ScopeType::BASIC, 0, {"affinity"});
  sgp_inst_lib->AddInst("Return", SGP__hardware_t::Inst_Return, 0, "Return from current function if possible.");
  sgp_inst_lib->AddInst("SetMem", SGP__hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  sgp_inst_lib->AddInst("CopyMem", SGP__hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  sgp_inst_lib->AddInst("SwapMem", SGP__hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  sgp_inst_lib->AddInst("Input", SGP__hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  sgp_inst_lib->AddInst("Output", SGP__hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  sgp_inst_lib->AddInst("Commit", SGP__hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  sgp_inst_lib->AddInst("Pull", SGP__hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  sgp_inst_lib->AddInst("Nop", SGP__hardware_t::Inst_Nop, 0, "No operation.");
  // - Non-default instruction set.
  // TODO (@amlalejini): Fill out instruction descriptions.
  sgp_inst_lib->AddInst("Fork",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_Fork(hw, inst); },
                        0, "...");
  sgp_inst_lib->AddInst("GetBoardWidth",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_GetBoardWidth(hw, inst); },
                        1, "...");
  sgp_inst_lib->AddInst("EndTurn",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_EndTurn(hw, inst); },
                        0, "...");
  sgp_inst_lib->AddInst("SetMoveXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveXY(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("SetMoveID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveID(hw, inst); },
                        1, "...");
  sgp_inst_lib->AddInst("GetMoveXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveXY(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("GetMoveID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveID(hw, inst); },
                        1, "...");
  sgp_inst_lib->AddInst("IsValidXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("IsValidID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("IsValidOppXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidOppXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("IsValidOppID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidOppID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("AdjacentXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentXY(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("AdjacentID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentID(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("ValidMoveCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ValidMoveCnt_HW(hw, inst); },
                        1, "...");
  sgp_inst_lib->AddInst("ValidOppMoveCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ValidOppMoveCnt_HW(hw, inst); },
                        1, "...");
  sgp_inst_lib->AddInst("GetBoardValueXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetBoardValueXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("GetBoardValueID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetBoardValueID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("PlaceDiskXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceDiskXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("PlaceDiskID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceDiskID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("PlaceOppDiskXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceOppDiskXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("PlaceOppDiskID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceOppDiskID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("FlipCntXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FlipCntXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("FlipCntID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FlipCntID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("OppFlipCntXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_OppFlipCntXY_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("OppFlipCntID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_OppFlipCntID_HW(hw, inst); },
                        2, "...");
  sgp_inst_lib->AddInst("FrontierCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FrontierCnt_HW(hw, inst); },
                        3, "...");
  sgp_inst_lib->AddInst("ResetBoard-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ResetBoard_HW(hw, inst); },
                        0, "...");
  sgp_inst_lib->AddInst("IsOver-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsOver_HW(hw, inst); },
                        1, "...");

  sgp_eval_hw = emp::NewPtr<SGP__hardware_t>(sgp_inst_lib, sgp_event_lib, random);
  sgp_eval_hw->SetMinBindThresh(SGP_HW_MIN_BIND_THRESH);
  sgp_eval_hw->SetMaxCores(SGP_HW_MAX_CORES);
  sgp_eval_hw->SetMaxCallDepth(SGP_HW_MAX_CALL_DEPTH);

  // - Setup move evaluation signals/functors -
  // Setup begin_turn_signal action:
  //  - Reset the evaluation hardware. Give hardware accurate playerID, and update the dreamboard.
  get_eval_agent_done = [this]() {
    return (bool)sgp_eval_hw->GetTrait(TRAIT_ID__DONE);
  };

  get_eval_agent_playerID = [this]() {
    return (size_t)sgp_eval_hw->GetTrait(TRAIT_ID__PLAYER_ID);
  };

  // Setup triggers!
  // Configure initial run setup
  switch (POP_INITIALIZATION_METHOD) {
    case POP_INITIALIZATION_METHOD_ID__RANDOM_POP:
      do_pop_init_sig.AddAction([this]() {
        this->SGP__InitPopulation_Random();
      });
      break;
    case POP_INITIALIZATION_METHOD_ID__ANCESTOR_FILE:
      do_pop_init_sig.AddAction([this]() {
        this->SGP__InitPopulation_FromAncestorFile();
      });
      break;
    default:
      std::cout << "Unrecognized population initialization method! Exiting..." << std::endl;
      exit(-1);
  }

  do_begin_run_setup_sig.AddAction([this]() {
    std::cout << "Doing initial run setup." << std::endl;
    // Setup systematics/fitness tracking.
    auto & sys_file = sgp_world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
    sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
    auto & fit_file = sgp_world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
    fit_file.SetTimingRepeat(FITNESS_INTERVAL);
    // Generate the initial population.
    do_pop_init_sig.Trigger();
  });

  // - Configure evaluation
  do_evaluation_sig.AddAction([this]() {
    double best_score = -32767;
    for (size_t id = 0; id < sgp_world->GetSize(); ++id) {
      // std::cout << "Evaluating agent: " << id << std::endl;
      // Evaluate agent given by id.
      SignalGPAgent & our_hero = sgp_world->GetOrg(id);
      our_hero.SetID(id);
      sgp_eval_hw->SetProgram(our_hero.GetGenome());
      this->Evaluate(our_hero);
      if (agent_score_cache[id] > best_score) best_score = agent_score_cache[id];
    }
    std::cout << "Update: " << update << " Max score: " << best_score << std::endl;
  });

  // - Configure world upate.
  do_world_update_sig.AddAction([this]() { sgp_world->Update(); this->ClearCache(); });

  // do_pop_snapshot
  do_pop_snapshot_sig.AddAction([this](size_t update) { this->SGP_Snapshot_SingleFile(update); });

  // - Configure selection
  // TODO: configure different based on selection method settings.
  switch (SELECTION_METHOD) {
    case SELECTION_METHOD_ID__TOURNAMENT:
      do_selection_sig.AddAction([this]() {
        emp::EliteSelect(*sgp_world, 1, 1);
        emp::TournamentSelect(*sgp_world, TOURNAMENT_SIZE, POP_SIZE - 1);
      });
      break;
    case SELECTION_METHOD_ID__LEXICASE: {
      sgp_lexicase_fit_set.resize(0);
      for (size_t i = 0; i < testcases.GetSize(); ++i) {
        sgp_lexicase_fit_set.push_back([i, this](SignalGPAgent & agent) {
          return testcase_score_cache[this->GetCacheID(agent.GetID(), i)];
        });
      }
      do_selection_sig.AddAction([this]() {
        emp::EliteSelect(*sgp_world, 1, 1);
        emp::LexicaseSelect(*sgp_world, sgp_lexicase_fit_set, POP_SIZE - 1);
      });
      break;
    }
    case SELECTION_METHOD_ID__ECOEA: {
      sgp_resource_fit_set.resize(0);
      // Add a resource fit function for each game phase.
      for (size_t i = 0; i < testcases_by_phase.size(); ++i) {
        sgp_resource_fit_set.push_back([i, this](SignalGPAgent & agent) {
          double score = 0;
          emp::vector<size_t> & phasecases = testcases_by_phase[i];
          // Aggregate all relevant test scores.
          for (size_t j = 0; j < phasecases.size(); ++j) {
            score += testcase_score_cache[this->GetCacheID(agent.GetID(), phasecases[j])];
          }
          return score;
        });
      }
      // Setup the do selection signal action.
      do_selection_sig.AddAction([this]() {
        emp::EliteSelect(*sgp_world, 1, 1);
        emp::ResourceSelect(*sgp_world, sgp_resource_fit_set, resources,
                            TOURNAMENT_SIZE, POP_SIZE - 1, RESOURCE_SELECT__FRAC,
                            RESOURCE_SELECT__MAX_BONUS, RESOURCE_SELECT__COST);
      });
      break;
    }
    case SELECTION_METHOD_ID__MAPELITES:
      std::cout << "Map-Elites selection not setup yet..." << std::endl;
      exit(-1);
      break;
    default:
      std::cout << "Unrecognized selection method! Exiting..." << std::endl;
      exit(-1);
  }

  // ANALYSIS_TYPE ANALYSIS_TYPE_ID__DEBUGGING
  switch (RUN_MODE) {
    case RUN_ID__EXP: {
      // Setup run-mode agent advance signal response.
      agent_advance_sig.AddAction([this]() {
        sgp_eval_hw->SingleProcess();
      });
      // Setup run-mode begin turn signal response.
      begin_turn_sig.AddAction([this](const emp::Othello & game) {
        const size_t playerID = testcases[cur_testcase].GetInput().playerID;
        SGP__ResetHW();
        sgp_eval_hw->SetTrait(TRAIT_ID__PLAYER_ID, playerID);
        othello_dreamware->Reset(game);
        othello_dreamware->SetActiveDream(0);
        othello_dreamware->SetPlayerID(playerID);
      });
      // Setup non-verbose get move.
      get_eval_agent_move = [this]() {
        return (size_t)sgp_eval_hw->GetTrait(TRAIT_ID__MOVE);
      };

      break;
    }

    case RUN_ID__ANALYSIS: {
      switch (ANALYSIS_TYPE) {

        case ANALYSIS_TYPE_ID__DEBUGGING: {
          // Debugging analysis signal response.
          do_analysis_sig.AddAction([this]() { this->SGP__Debugging_Analysis(); });
          // Setup a verbose agent_advance_sig
          agent_advance_sig.AddAction([this]() {
            std::cout << "----- EVAL STEP: " << eval_time << " -----" << std::endl;
            sgp_eval_hw->SingleProcess();
            sgp_eval_hw->PrintState();
            std::cout << "--- DREAMBOARD STATE ---" << std::endl;
            othello_dreamware->GetActiveDreamOthello().Print();
          });
          // Setup a verbose begin_turn_sig response.
          begin_turn_sig.AddAction([this](const emp::Othello & game) {
            const size_t playerID = testcases[cur_testcase].GetInput().playerID;
            std::cout << "===============================================" << std::endl;
            std::cout << "TEST CASE: " << cur_testcase << std::endl;
            std::cout << "ID: " << testcases[cur_testcase].id << std::endl;
            std::cout << " ----- Input ----- " << std::endl;
            // Board
            testcases[cur_testcase].GetInput().game.Print();
            auto options = testcases[cur_testcase].GetInput().game.GetMoveOptions(testcases[cur_testcase].GetInput().playerID);
            std::cout << "Board width: " << testcases[cur_testcase].GetInput().game.GetBoardWidth() << std::endl;
            std::cout << "Round: " << testcases[cur_testcase].GetInput().round << std::endl;
            std::cout << "PlayerID: " << testcases[cur_testcase].GetInput().playerID << std::endl;
            std::cout << " ----- Output ----- " << std::endl;
            std::cout << "Expert move: " << testcases[cur_testcase].GetOutput().expert_move << std::endl;
            std::cout << "Board options: ";
            for (size_t j = 0; j < options.size(); ++j) {
              std::cout << " " << options[j];
            } std::cout << std::endl;
            std::cout << "Valid options: ";
            for (size_t j = 0; j < testcases[cur_testcase].GetOutput().move_valid.size(); ++j) {
              std::cout << " " << testcases[cur_testcase].GetOutput().move_valid[j];
            } std::cout << std::endl;

            SGP__ResetHW();
            sgp_eval_hw->SetTrait(TRAIT_ID__PLAYER_ID, playerID);
            othello_dreamware->Reset(game);
            othello_dreamware->SetActiveDream(0);
            othello_dreamware->SetPlayerID(playerID);
          });

          get_eval_agent_move = [this]() {
            size_t move = (size_t)sgp_eval_hw->GetTrait(TRAIT_ID__MOVE);
            std::cout << "SELECTED MOVE: " << move << std::endl;
            return move;
          };

          break;
        }
        default:
          std::cout << "Unrecognized analysis type. Exiting..." << std::endl;
          exit(-1);
      }
      break;
    }

    default:
      std::cout << "Unrecognized run mode! Exiting..." << std::endl;
      exit(-1);
  }

}

void LineageExp::ConfigAGP() {
  // TODO (@steven)
  agp_world->Reset();
  agp_world->SetWellMixed(true);

  agp_inst_lib->AddInst("Dec", AGP__inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
  agp_inst_lib->AddInst("Not", AGP__inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
  agp_inst_lib->AddInst("SetReg", AGP__inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
  agp_inst_lib->AddInst("Add", AGP__inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
  agp_inst_lib->AddInst("Sub", AGP__inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
  agp_inst_lib->AddInst("Mult", AGP__inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
  agp_inst_lib->AddInst("Div", AGP__inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
  agp_inst_lib->AddInst("Inc", AGP__inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
  agp_inst_lib->AddInst("Mod", AGP__inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
  agp_inst_lib->AddInst("TestEqu", AGP__inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
  agp_inst_lib->AddInst("TestNEqu", AGP__inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
  agp_inst_lib->AddInst("TestLess", AGP__inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
  agp_inst_lib->AddInst("If", AGP__inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
  agp_inst_lib->AddInst("While", AGP__inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
  agp_inst_lib->AddInst("Countdown", AGP__inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
  agp_inst_lib->AddInst("Break", AGP__inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
  agp_inst_lib->AddInst("Scope", AGP__inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
  agp_inst_lib->AddInst("Define", AGP__inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
  agp_inst_lib->AddInst("Call", AGP__inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
  agp_inst_lib->AddInst("Push", AGP__inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
  agp_inst_lib->AddInst("Pop", AGP__inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
  agp_inst_lib->AddInst("Input", AGP__inst_lib_t::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
  agp_inst_lib->AddInst("Output", AGP__inst_lib_t::Inst_Output, 2, "Push reg Arg1 into output Arg2");
  agp_inst_lib->AddInst("CopyVal", AGP__inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
  agp_inst_lib->AddInst("ScopeReg", AGP__inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

  // - Non-default instruction set.
  // TODO (@steven): Fill out instruction descriptions.
  // agp_inst_lib->AddInst("GetBoardWidth",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_GetBoardWidth(hw, inst); },
  //                       1, "...");
  // agp_inst_lib->AddInst("EndTurn",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_EndTurn(hw, inst); },
  //                       0, "...");
  // agp_inst_lib->AddInst("SetMoveXY",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_SetMoveXY(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("SetMoveID",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_SetMoveID(hw, inst); },
  //                       1, "...");
  // agp_inst_lib->AddInst("GetMoveXY",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetMoveXY(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("GetMoveID",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetMoveID(hw, inst); },
  //                       1, "...");
  // agp_inst_lib->AddInst("IsValidXY",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_IsValidXY(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("IsValidID",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_IsValidID(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("AdjacentXY",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_AdjacentXY(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("AdjacentID",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_AdjacentID(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("ValidMoveCnt-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_ValidMoveCnt_HW(hw, inst); },
  //                       1, "...");
  // agp_inst_lib->AddInst("ValidOppMoveCnt-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_ValidOppMoveCnt_HW(hw, inst); },
  //                       1, "...");
  // agp_inst_lib->AddInst("GetBoardValueXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_GetBoardValueXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("GetBoardValueID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_GetBoardValueID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("PlaceDiskXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_PlaceDiskXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("PlaceDiskID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_PlaceDiskID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("PlaceOppDiskXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_PlaceOppDiskXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("PlaceOppDiskID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_PlaceOppDiskID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("FlipCntXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_FlipCntXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("FlipCntID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_FlipCntID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("OppFlipCntXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_OppFlipCntXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("OppFlipCntID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_OppFlipCntID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("FrontierCntXY-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_FrontierCntXY_HW(hw, inst); },
  //                       3, "...");
  // agp_inst_lib->AddInst("FrontierCntID-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_FrontierCntID_HW(hw, inst); },
  //                       2, "...");
  // agp_inst_lib->AddInst("ResetBoard-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_ResetBoard_HW(hw, inst); },
  //                       0, "...");
  // agp_inst_lib->AddInst("IsOver-HW",
  //                       [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP_Inst_IsOver_HW(hw, inst); },
  //                       1, "...");

  agp_eval_hw = emp::NewPtr<AGP__hardware_t>(agp_inst_lib);
  // TODO: whatever agp_eval_hw configs...

}

#endif
