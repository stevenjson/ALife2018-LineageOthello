#ifndef LINEAGE_EXP_H
#define LINEAGE_EXP_H

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

constexpr size_t SGP__TAG_WIDTH = 16;

constexpr size_t REPRESENTATION_ID__AVIDAGP = 0;
constexpr size_t REPRESENTATION_ID__SIGNALGP = 1;

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
    emp::Othello game;
    size_t playerID;

    TestcaseInput(size_t board_width, size_t pID=0) : game(board_width), playerID(pID) { ; }
    TestcaseInput(const TestcaseInput &) = default;
    TestcaseInput(TestcaseInput &&) = default;
  };

  struct TestcaseOutput {
    size_t expert_move;
    emp::BitVector move_validity;
  };

  // More aliases
  using SGP__world_t = emp::World<SignalGPAgent>;
  using AGP__world_t = emp::World<AvidaGPAgent>;

  using testcase_t = std::pair<TestcaseInput,TestcaseOutput>;

protected:
  // == Configurable experiment parameters ==
  // General parameters
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t EVAL_TIME;
  size_t REPRESENTATION;

  size_t TOURNAMENT_SIZE;

  size_t BOARD_WIDTH;

  size_t POP_SNAPSHOT_INTERVAL;

  // Experiment variables.
  emp::Ptr<emp::Random> random;

  // QUESTION: Do we want to also have a Testcase input struct? (like our test case output struct)
  TestcaseSet<TestcaseInput,TestcaseOutput> testcases; ///< Test cases are OthelloBoard ==> Expert move

  emp::Ptr<OthelloHardware> othello_dreamware;

  // SignalGP-specifics.
  emp::Ptr<SGP__world_t> sgp_world;
  emp::Ptr<SGP__inst_lib_t> sgp_inst_lib;
  emp::Ptr<SGP__event_lib_t> sgp_event_lib;
  emp::Ptr<SGP__hardware_t> sgp_eval_hw;

  // AvidaGP-specifics.
  emp::Ptr<AGP__world_t> agp_world;
  // TODO (@steven)

  // Signals!
  // --- Experiment signals ---
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

  /// NOTE: at the moment, the last thing in game_board is used to indicate playerID
  /// TODO: clean up testcase set.
  testcase_t GenerateTestcase(std::vector<std::string> game_board, std::string expert_move) {
    TestcaseInput input(BOARD_WIDTH);
    emp::Othello & game = input.game;
    emp_assert(game_board.size() == (game.GetBoardSize() + 1));
    size_t playerID = std::atoi(game_board.back().c_str());
    game_board.resize(game_board.size() - 1);
    // Fill out game board.
    for (size_t i = 0; i < game_board.size(); ++i) {
      int board_space = std::atoi(game_board[i].c_str());
      switch (board_space) {
        case emp::Othello::DarkPlayerID():
          game.SetPos(i, emp::Othello::DarkDisk());
          break;
        case emp::Othello::LightPlayerID():
          game.SetPos(i, emp::Othello::LightDisk());
          break;
        default:
          game.SetPos(i, emp::Othello::OpenSpace());
          break;
      }
    }
    // Fill out testcase output.
    TestcaseOutput output;
    output.expert_move = std::atoi(expert_move.c_str());
    emp::vector<size_t> valid_moves = game.GetMoveOptions(playerID);
    output.move_validity.Resize(game.GetBoardSize());   // Resize bit vector to match board size.
    output.move_validity.Clear();                       // Set bits to 0.
    for (size_t i = 0; i < valid_moves.size(); ++i) {
      output.move_validity.Set(valid_moves[i], true);
    }
    return std::make_pair(input, output);
  }

public:
  LineageExp(const LineageConfig & config)
  {
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    EVAL_TIME = config.EVAL_TIME();
    REPRESENTATION = config.REPRESENTATION();
    POP_SNAPSHOT_INTERVAL = 100;
    TOURNAMENT_SIZE = 4;
    BOARD_WIDTH = 8;

    // Setup output directory.
    // TODO
    // Make a random number generator.
    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    // Load test cases.
    // TODO

    // Configure the dreamware!
    othello_dreamware = emp::NewPtr<OthelloHardware>(BOARD_WIDTH, 1);

    // Make the world(s)!
    // - SGP World -
    sgp_world = emp::NewPtr<SGP__world_t>(random, "SGP-LineageAnalysis-World");
    agp_world = emp::NewPtr<AGP__world_t>(random, "AGP-LineageAnalysis-World");

    // Configure instruction/event libraries.
    sgp_inst_lib = emp::NewPtr<SGP__inst_lib_t>();
    sgp_event_lib = emp::NewPtr<SGP__event_lib_t>();
    // TODO (@steven): agp inst lib.

    // TODO: setup data-recording file.

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
    // TODO
  }

  void ConfigSGP();
  void ConfigAGP();

  void Run() {
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

  // -- AvidaGP Instructions --
  // TODO (@steven)

  // -- SignalGP Instructions --
  // TODO: actual instruction implementations
  // Fork
  void SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst);
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

void LineageExp::ConfigSGP() {
  // Configure the world.
  sgp_world->Reset();
  sgp_world->SetWellMixed(true);

  // Configure the instruction set.
  // TODO

  // Setup triggers!
  // - Configure evaluation
  do_evaluation_sig.AddAction([this]() {
    for (size_t id = 0; id < sgp_world->GetSize(); ++id) {
      // ...TODO...
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

  // do_analysis --> TODO

  // do_pop_snapshot
}

void LineageExp::ConfigAGP() {
  // TODO (@steven)
}

#endif
