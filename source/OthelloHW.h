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
  using player_t = emp::Othello::Player;
  emp::vector<emp::Othello> dreams; ///< Let's lean into that whole 'othello dream' terminology...
  size_t active_dream;
  player_t playerID;

public:
  OthelloHardware(size_t dream_cnt, player_t pID=player_t::DARK)
  : dreams(dream_cnt), active_dream(0), playerID(pID)
  { emp_assert(dream_cnt > 0); }

  emp::Othello & GetActiveDreamOthello() { return dreams[active_dream]; }

  void SetActiveDream(size_t id) {
    emp_assert(id < dreams.size());
    active_dream = id;
  }

  void SetPlayerID(player_t pID) { playerID = pID; }
  player_t GetPlayerID() const { return playerID; }

  void Reset() {
    for (size_t i = 0; i < dreams.size(); ++i) dreams[i].Reset();
  }

  void Reset(const emp::Othello & other) {
    for (size_t i = 0; i < dreams.size(); ++i) {
      dreams[i].Reset();
      dreams[i].SetBoard(other.GetBoard());
    }
  }

  void ResetActive() {
    dreams[active_dream].Reset();
  }

  void ResetActive(const emp::Othello & other) {
    dreams[active_dream].Reset();
    dreams[active_dream].SetBoard(other.GetBoard());
  }

};


#endif
