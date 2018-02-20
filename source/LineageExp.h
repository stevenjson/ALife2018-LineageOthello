#ifndef LINEAGE_EXP_H
#define LINEAGE_EXP_H

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
#include "games/Othello.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/AvidaGP.h"
#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

#include "TestcaseSet.h"
#include "OthelloHW.h"
#include "lineage-config.h"

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

class LineageExp {
public:
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

  struct Agent {
    emp::vector<double> scores_by_testcase;

    Agent() : scores_by_testcase() { ; }
    Agent(const Agent && in) : scores_by_testcase(in.scores_by_testcase) { ; }
    Agent(const Agent & in) : scores_by_testcase(in.scores_by_testcase) { ; }

  };

  struct SignalGPAgent : Agent {
    using Agent::scores_by_testcase;
    SGP__program_t program;

    SignalGPAgent(const SGP__program_t & _p)
    : Agent(), program(_p)
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
    // TODO (@steven)
    emp::vector<bool> place_holder;

    emp::vector<bool> & GetGenome() { return place_holder; }
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
    emp::BitVector move_validity;   ///< BoardSize length bitvector describing what moves are valid on the associated testcase?
  };

  // More aliases
  using SGP__world_t = emp::World<SignalGPAgent>;
  using AGP__world_t = emp::World<AvidaGPAgent>;

protected:
  // == Configurable experiment parameters ==
  // General parameters
  size_t RUN_MODE;
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t EVAL_TIME;
  size_t REPRESENTATION;
  std::string TEST_CASE_FILE;
  std::string ANCESTOR_FPATH;
  // Selection Group parameters
  size_t SELECTION_METHOD;
  size_t TOURNAMENT_SIZE;
  // Othello Group parameters
  size_t OTHELLO_BOARD_WIDTH;
  size_t OTHELLO_HW_BOARDS;
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

  // Experiment variables.
  emp::Ptr<emp::Random> random;

  TestcaseSet<TestcaseInput,TestcaseOutput> testcases; ///< Test cases are OthelloBoard ==> Expert move
  using test_case_t = typename TestcaseSet<TestcaseInput, TestcaseOutput>::test_case_t;

  emp::Ptr<OthelloHardware> othello_dreamware; ///< Othello game board dreamware!

  // SignalGP-specifics.
  emp::Ptr<SGP__world_t> sgp_world;         ///< World for evolving SignalGP agents.
  emp::Ptr<SGP__inst_lib_t> sgp_inst_lib;   ///< SignalGP instruction library.
  emp::Ptr<SGP__event_lib_t> sgp_event_lib; ///< SignalGP event library.
  emp::Ptr<SGP__hardware_t> sgp_eval_hw;    ///< Hardware used to evaluate SignalGP programs during evolution/analysis.

  // AvidaGP-specifics.
  emp::Ptr<AGP__world_t> agp_world;       ///< World for evolving AvidaGP agents.
  // TODO (@steven)

  // --- Experiment signals ---
  // - DoBeginRun: this is where you'll setup systematics, fitness file, etc. with the world.
  emp::Signal<void(void)> do_begin_run_setup_sig;
  // - DoEvaluation: evaluate e'rybody!
  emp::Signal<void(void)> do_evaluation_sig;
  // - DoSelection: Do selection.
  emp::Signal<void(void)> do_selection_sig;
  // - DoUpdate: Responsible for calling world->Update()
  emp::Signal<void(void)> do_world_update_sig;
  // - DoMutation:
  emp::Signal<void(void)> do_mutation_sig;
  // - DoAnalysis
  emp::Signal<void(void)> do_analysis_sig;
  // - Snapshot?
  emp::Signal<void(size_t)> do_pop_snapshot_sig;

  // --- Agent evaluation signals ---
  // othello_begin_turn
  emp::Signal<void(const emp::Othello &)> begin_turn_sig;
  emp::Signal<void(void)> agent_advance_sig;

  // Functors!
  // - Get move
  std::function<size_t(void)> get_eval_agent_move;
  // - Get done
  std::function<bool(void)> get_eval_agent_done;
  // - Get player ID
  std::function<size_t(void)> get_eval_agent_playerID;

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
    for (size_t i = 0; i < EVAL_TIME && !get_eval_agent_done(); ++i) {
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
    std::cout << "Generate test case!" << std::endl;
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
    output.move_validity.Resize(game.GetBoardSize());   // Resize bit vector to match board size.
    output.move_validity.Clear();                       // Set bits to 0.
    for (size_t i = 0; i < valid_moves.size(); ++i) {
      output.move_validity.Set(valid_moves[i], true);
    }
    return test_case_t(input, output);
  }

public:
  LineageExp(const LineageConfig & config)
    : testcases()
  {
    RUN_MODE = config.RUN_MODE();
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    EVAL_TIME = config.EVAL_TIME();
    REPRESENTATION = config.REPRESENTATION();
    TEST_CASE_FILE = config.TEST_CASE_FILE();
    ANCESTOR_FPATH = config.ANCESTOR_FPATH();
    SELECTION_METHOD = config.SELECTION_METHOD();
    TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    OTHELLO_BOARD_WIDTH = config.OTHELLO_BOARD_WIDTH();
    OTHELLO_HW_BOARDS = config.OTHELLO_HW_BOARDS();
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

    // Make a random number generator.
    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    // Load test cases.
    testcases.RegisterTestcaseReader([this](emp::vector<std::string> & strs) { return this->GenerateTestcase(strs); });
    testcases.LoadTestcases(TEST_CASE_FILE);

    // for (size_t i = 0; i < testcases.GetSize(); ++i) {
    //   std::cout << "============= Test case: " << i << " =============" << std::endl;
    //   std::cout << "ID: " << testcases[i].id << std::endl;
    //   std::cout << "Input" << std::endl;
    //   // Board
    //   testcases[i].GetInput().game.Print();
    //   std::cout << "Round: " << testcases[i].GetInput().round << std::endl;
    //   std::cout << "PlayerID: " << testcases[i].GetInput().playerID << std::endl;
    //   std::cout << "Output" << std::endl;
    //   std::cout << "Expert move: " << testcases[i].GetOutput().expert_move << std::endl;
    // }

    // Configure the dreamware!
    othello_dreamware = emp::NewPtr<OthelloHardware>(OTHELLO_BOARD_WIDTH, 1);

    // Make the world(s)!
    // - SGP World -
    sgp_world = emp::NewPtr<SGP__world_t>(random, "SGP-LineageAnalysis-World");
    agp_world = emp::NewPtr<AGP__world_t>(random, "AGP-LineageAnalysis-World");

    // Configure instruction/event libraries.
    sgp_inst_lib = emp::NewPtr<SGP__inst_lib_t>();
    sgp_event_lib = emp::NewPtr<SGP__event_lib_t>();
    // TODO (@steven): agp inst lib.

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
    exit(-1);
  }

  ~LineageExp() {
    random.Delete();
    othello_dreamware.Delete();
    sgp_world.Delete();
    sgp_inst_lib.Delete();
    sgp_event_lib.Delete();
    sgp_eval_hw.Delete();
    agp_world.Delete();
    // TODO: clean up whatever Avida-specific dynamically allocated memory we end up using.
  }

  void ConfigSGP();
  void ConfigAGP();

  void Run() {
    do_begin_run_setup_sig.Trigger();
    for (size_t ud = 0; ud < GENERATIONS; ++ud) {
      RunStep();
      if (ud % POP_SNAPSHOT_INTERVAL == 0) do_pop_snapshot_sig.Trigger(ud);
    }
  }

  void RunStep() {
    do_evaluation_sig.Trigger();
    do_selection_sig.Trigger();
    do_world_update_sig.Trigger();
    do_mutation_sig.Trigger();
  }

  // Mutation functions
  size_t SGP__Mutate(SignalGPAgent & agent, emp::Random & rnd); // TODO
  size_t AGP__Mutate(AvidaGPAgent & agent, emp::Random & rnd);  // TODO

  // Population snapshot functions
  void SGP_Snapshot_SingleFile(size_t update); // TODO

  // SignalGP utility functions.
  void SGP__ResetHW(const SGP__memory_t & main_in_mem=SGP__memory_t());

  // -- AvidaGP Instructions --
  // TODO (@steven)

  // -- SignalGP Instructions --
  // TODO: actual instruction implementations
  // Fork
  static void SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // BoardWidth
  void SGP_Inst_GetBoardWidth(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // EndTurn
  void SGP_Inst_EndTurn(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // SetMoveXY
  void SGP__Inst_SetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_SetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetMoveXY
  void SGP__Inst_GetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_GetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsValidXY
  void SGP__Inst_IsValidXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_IsValidID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // AdjacentXY
  void SGP__Inst_AdjacentXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_AdjacentID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidMovesCnt
  void SGP_Inst_ValidMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidOppMovesCnt
  void SGP_Inst_ValidOppMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetBoardValue
  void SGP_Inst_GetBoardValueXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_GetBoardValueID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // PlaceXY
  void SGP_Inst_PlaceDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_PlaceDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // PlaceOppXY
  void SGP_Inst_PlaceOppDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_PlaceOppDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FlipCntXY
  void SGP_Inst_FlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_FlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // OppFlipCntXY
  void SGP_Inst_OppFlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_OppFlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FrontierCntXY
  void SGP_Inst_FrontierCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP_Inst_FrontierCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ResetBoard
  void SGP_Inst_ResetBoard_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsOver
  void SGP_Inst_IsOver_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
};

void LineageExp::SGP__ResetHW(const SGP__memory_t & main_in_mem) {
  sgp_eval_hw->ResetHardware();
  sgp_eval_hw->SetTrait(TRAIT_ID__MOVE, -1);
  sgp_eval_hw->SetTrait(TRAIT_ID__DONE, 0);
  sgp_eval_hw->SetTrait(TRAIT_ID__PLAYER_ID, -1);
  sgp_eval_hw->SpawnCore(0, main_in_mem, true);
}

void LineageExp::ConfigSGP() {
  // Configure the world.
  sgp_world->Reset();
  sgp_world->SetWellMixed(true);

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
  // sgp_inst_lib->AddInst("Fork", SGP__Inst_Fork, 0, "...");
  // sgp_inst_lib->AddInst("GetBoardWidth",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_GetBoardWidth(hw, inst); },
  //                       1, "...");
  // sgp_inst_lib->AddInst("EndTurn",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_EndTurn(hw, inst); },
  //                       0, "...");
  // sgp_inst_lib->AddInst("SetMoveXY",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveXY(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("SetMoveID",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveID(hw, inst); },
  //                       1, "...");
  // sgp_inst_lib->AddInst("GetMoveXY",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveXY(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("GetMoveID",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveID(hw, inst); },
  //                       1, "...");
  // sgp_inst_lib->AddInst("IsValidXY",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidXY(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("IsValidID",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidID(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("AdjacentXY",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentXY(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("AdjacentID",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentID(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("ValidMoveCnt-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_ValidMoveCnt_HW(hw, inst); },
  //                       1, "...");
  // sgp_inst_lib->AddInst("ValidOppMoveCnt-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_ValidOppMoveCnt_HW(hw, inst); },
  //                       1, "...");
  // sgp_inst_lib->AddInst("GetBoardValueXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_GetBoardValueXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("GetBoardValueID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_GetBoardValueID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("PlaceDiskXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_PlaceDiskXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("PlaceDiskID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_PlaceDiskID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("PlaceOppDiskXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_PlaceOppDiskXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("PlaceOppDiskID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_PlaceOppDiskID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("FlipCntXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_FlipCntXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("FlipCntID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_FlipCntID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("OppFlipCntXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_OppFlipCntXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("OppFlipCntID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_OppFlipCntID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("FrontierCntXY-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_FrontierCntXY_HW(hw, inst); },
  //                       3, "...");
  // sgp_inst_lib->AddInst("FrontierCntID-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_FrontierCntID_HW(hw, inst); },
  //                       2, "...");
  // sgp_inst_lib->AddInst("ResetBoard-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_ResetBoard_HW(hw, inst); },
  //                       0, "...");
  // sgp_inst_lib->AddInst("IsOver-HW",
  //                       [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_IsOver_HW(hw, inst); },
  //                       1, "...");

  // Setup triggers!
  // Configure initial run setup
  // - QUESTION: Do we want ancestors to be NOP ancestors or randomly generated programs?
  do_begin_run_setup_sig.AddAction([this]() {
    // Setup systematics/fitness tracking.
    auto & sys_file = sgp_world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
    sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
    auto & fit_file = sgp_world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
    fit_file.SetTimingRepeat(FITNESS_INTERVAL);
    // 1) Load/configure ancestor, fill out population.
    // TODO
  });

  // - Configure evaluation
  do_evaluation_sig.AddAction([this]() {
    for (size_t id = 0; id < sgp_world->GetSize(); ++id) {
      // Evaluate agent given by id.
      SignalGPAgent & our_hero = sgp_world->GetOrg(id);
      our_hero.scores_by_testcase.resize(testcases.GetSize(), 0);
      sgp_eval_hw->SetProgram(our_hero.GetGenome());
      // Evaluate agent on all test cases.
      for (size_t testID = 0; testID < testcases.GetSize(); ++testID) {
        // TODO: evaluate agent on test case.
        // size_t move = EvalMove__GP();
      }
    }
  });

  // - Configure selection
  do_selection_sig.AddAction([this]() {
    emp::EliteSelect(*sgp_world, 1, 1);
    emp::TournamentSelect(*sgp_world, TOURNAMENT_SIZE, POP_SIZE - 1);
  });

  // - Configure world upate.
  do_world_update_sig.AddAction([this]() { sgp_world->Update(); });

  // do_mutation
  do_mutation_sig.AddAction([this]() { sgp_world->DoMutations(1); });

  // do_pop_snapshot
  // do_pop_snapshot_sig.AddAction([this](size_t update) { this->SGP_Snapshot_SingleFile(update); });

  // do_analysis --> TODO
}

void LineageExp::ConfigAGP() {
  // TODO (@steven)
}

#endif
