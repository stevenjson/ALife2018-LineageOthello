#ifndef OTHELLO_LU_H
#define OTHELLO_LU_H

#include <unordered_map>

#include "base/vector.h"
#include "games/Othello8.h"
#include "tools/map_utils.h"


class OthelloLookup {
  using othello_t = emp::Othello8;
  using idx_t = othello_t::Index;
  using player_t = othello_t::Player;

public:
  struct OthelloInfo {

    emp::vector<emp::vector<idx_t>> dark_flip_list_by_pos;
    size_t dark_frontier_cnt;
    emp::vector<idx_t> dark_move_options;
    emp::vector<char> dark_is_valid_move_by_pos;

    emp::vector<emp::vector<idx_t>> light_flip_list_by_pos;
    size_t light_frontier_cnt;
    emp::vector<idx_t> light_move_options;
    emp::vector<char> light_is_valid_move_by_pos;


    size_t GetFrontierCnt(player_t player) {
      return (player == player_t::DARK) ? dark_frontier_cnt : light_frontier_cnt;
    }

    const emp::vector<idx_t> & GetFlipList(player_t player, idx_t index) {
      emp_assert(index < dark_flip_list_by_pos.size() && index < light_flip_list_by_pos.size());
      return (player == player_t::DARK) ? dark_flip_list_by_pos[index] : light_flip_list_by_pos[index];
    }

    const emp::vector<idx_t> & GetMoveOptions(player_t player) {
      return (player == player_t::DARK) ? dark_move_options : light_move_options;
    }

    bool IsValidMove(player_t player, idx_t index) {
      // emp_assert(index < dark_is_valid_move_by_pos.size() && index < light_is_valid_move_by_pos.size());
      if (index >= dark_is_valid_move_by_pos.size() && index >= light_is_valid_move_by_pos.size()) {
        std::cout << "Oh no! " << dark_is_valid_move_by_pos.size() << "; " << light_is_valid_move_by_pos.size() << "; " << (size_t)index << std::endl;
      }
      return (player == player_t::DARK) ? (bool)dark_is_valid_move_by_pos[index] : (bool)light_is_valid_move_by_pos[index];
    }

  };

protected:
  // o, then p
  std::unordered_map<uint64_t, std::unordered_map<uint64_t, OthelloInfo>> lookup;
  emp::vector<idx_t> empty_list;

public:
  OthelloLookup() : empty_list(0) { ; }

  void CacheBoard(othello_t & othello) {
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (!emp::Has(lookup, o)) {
      lookup[o] = std::unordered_map<uint64_t, OthelloInfo>();
    }
    if (!emp::Has(lookup[o], p)) {
      lookup[o][p] = OthelloInfo();
    }
    OthelloInfo & info = lookup[o][p];
    for (size_t i = 0; i < othello.GetNumCells(); ++i) {
      info.dark_flip_list_by_pos.emplace_back(othello.GetFlipList(player_t::DARK, i));
      info.light_flip_list_by_pos.emplace_back(othello.GetFlipList(player_t::LIGHT, i));

      info.dark_is_valid_move_by_pos.emplace_back(othello.IsValidMove(player_t::DARK, i));
      info.light_is_valid_move_by_pos.emplace_back(othello.IsValidMove(player_t::LIGHT, i));
    }
    info.dark_frontier_cnt = othello.CountFrontierPos(player_t::DARK);
    info.light_frontier_cnt = othello.CountFrontierPos(player_t::LIGHT);

    info.dark_move_options = othello.GetMoveOptions(player_t::DARK);
    info.light_move_options = othello.GetMoveOptions(player_t::LIGHT);
  }

  // CountFrontierPos
  size_t CountFrontierPos(othello_t & othello, player_t player) {
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (emp::Has(lookup, o)) {
      if (emp::Has(lookup[o], p)) {
        return lookup[o][p].GetFrontierCnt(player);
      }
    }
    CacheBoard(othello);
    return lookup[o][p].GetFrontierCnt(player);
  }

  // GetFlipList
  const emp::vector<idx_t> & GetFlipList(othello_t & othello, player_t player, idx_t index) {
    if (!index.IsValid()) return empty_list;
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (emp::Has(lookup, o)) {
      if (emp::Has(lookup[o], p)) {
        return lookup[o][p].GetFlipList(player, index);
      }
    }
    CacheBoard(othello);
    return lookup[o][p].GetFlipList(player, index);
  }

  // GetFlipCount
  size_t GetFlipCount(othello_t & othello, player_t player, idx_t index) {
    if (!index.IsValid()) return 0;
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (emp::Has(lookup, o)) {
      if (emp::Has(lookup[o], p)) {
        return lookup[o][p].GetFlipList(player, index).size();
      }
    }
    CacheBoard(othello);
    return lookup[o][p].GetFlipList(player, index).size();
  }

  // GetMoveOptions
  const emp::vector<idx_t> & GetMoveOptions(othello_t & othello, player_t player) {
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (emp::Has(lookup, o)) {
      if (emp::Has(lookup[o], p)) {
        return lookup[o][p].GetMoveOptions(player);
      }
    }
    CacheBoard(othello);
    return lookup[o][p].GetMoveOptions(player);
  }

  // IsValid
  bool IsValidMove(othello_t & othello, player_t player, idx_t index) {
    if (!index.IsValid()) return false;
    const uint64_t o = othello.GetBoard().occupied;
    const uint64_t p = othello.GetBoard().player;
    if (emp::Has(lookup, o)) {
      if (emp::Has(lookup[o], p)) {
        return lookup[o][p].IsValidMove(player, index);
      }
    }
    return othello.IsValidMove(player, index);
  }

};





#endif
