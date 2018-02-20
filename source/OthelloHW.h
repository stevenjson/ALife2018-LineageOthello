#ifndef OTHELLO_HW_H
#define OTHELLO_HW_H

#include "base/vector.h"
#include "games/Othello.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

// NOTE: we don't actually need this for test case evaluations...
class OthelloHardware {

protected:
  emp::vector<emp::Othello> dreams; ///< Let's lean into that whole 'othello dream' terminology...
  size_t playerID;
  size_t active_dream;

public:
  OthelloHardware(size_t board_size, size_t dream_cnt, size_t _playerID=0)
  : dreams(), playerID(_playerID), active_dream(0)
  {
    emp_assert(dream_cnt > 0);
    emp_assert(IsValidPlayer(playerID));

    for (size_t i = 0; i < dream_cnt; ++i) dreams.emplace_back(board_size);

  }

  emp::Othello & GetActiveDreamOthello() { return dreams[active_dream]; }

  bool IsValidPlayer(size_t id) { return id == 0 || id == 1; }

  void SetPlayerID(size_t id) {
    emp_assert(IsValidPlayer(id));
    playerID = id;
  }

  void SetActiveDream(size_t id) {
    emp_assert(id < dreams.size());
    active_dream = id;
  }

  void Reset() {
    for (size_t i = 0; i < dreams.size(); ++i) dreams[i].Reset();
  }

  void Reset(const emp::Othello & other) {
    for (size_t i = 0; i < dreams.size(); ++i) {
      dreams[i].Reset();
      dreams[i].SetBoard(other.GetBoard());
    }
  }

};


#endif
