// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "base/vector.h"

#include "config/command_line.h"
#include "config/ArgManager.h"

#include "../lineage-config.h"
#include "../LineageExp.h"
#include "../OthelloLookup.h"

int main(int argc, char* argv[])
{
  using player_t = emp::Othello8::Player;
  // Read configs.
  OthelloLookup lu;
  emp::Othello8 game;
  emp::Random random;

  size_t trials = 1000;

  int dummy_var = 0;

  for (size_t trialid = 0; trialid < trials; ++trialid) {
    std::cout << "Trial " << trialid << std::endl;
    // For each trial:
    // 1) Generate a random board via random moves.
    size_t num_moves = random.GetUInt(10,60);
    game.Reset();
    for (size_t i = 0; i < num_moves; ++i) {
      // Make a bunch of moves!
      auto moves = game.GetMoveOptions();
      if (moves.size() == 0) { break; }
      size_t next_move = moves[random.GetUInt(moves.size())];
      game.DoNextMove(next_move);
    }

    std::clock_t base_start_time = std::clock();
    // 2) Cache the board.
    lu.CacheBoard(game);
    // 3) Poke the cache a lot.
    for (size_t i = 0; i < game.GetNumCells() + 10; ++i) {
      dummy_var += lu.GetFlipList(game, player_t::DARK, i).size();
      dummy_var += lu.GetFlipList(game, player_t::LIGHT, i).size();
      dummy_var += lu.CountFrontierPos(game, player_t::DARK);
      dummy_var += lu.CountFrontierPos(game, player_t::LIGHT);
      dummy_var += lu.GetMoveOptions(game, player_t::DARK).size();
      dummy_var += lu.GetMoveOptions(game, player_t::LIGHT).size();
    }
    std::clock_t base_tot_time = std::clock() - base_start_time;
    std::cout << "  Lookup time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
              << " ms." << std::endl;
    // How fast is caching+a bunch of lookups compared to just raw requests?
    base_start_time = std::clock();
    for (size_t i = 0; i < game.GetNumCells() + 10; ++i) {
      dummy_var += game.GetFlipList(player_t::DARK, i).size();
      dummy_var += game.GetFlipList(player_t::LIGHT, i).size();
      dummy_var += game.CountFrontierPos(player_t::DARK);
      dummy_var += game.CountFrontierPos(player_t::LIGHT);
      dummy_var += game.GetMoveOptions(player_t::DARK).size();
      dummy_var += game.GetMoveOptions(player_t::LIGHT).size();
    }
     base_tot_time = std::clock() - base_start_time;
    std::cout << "  Raw time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
              << " ms." << std::endl;
  }
}
