#ifndef LINEAGE_INST_IMPL_H
#define LINEAGE_INST_IMPL_H

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


// AGP_Inst_GetBoardWidth
void LineageExp::AGP__Inst_GetBoardWidth(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  hw.regs[inst.args[0]] = OTHELLO_BOARD_WIDTH;
}
// AGP_Inst_EndTurn
void LineageExp::AGP__Inst_EndTurn(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  hw.SetTrait(TRAIT_ID__DONE, 1);
}
// AGP__Inst_SetMoveXY
void LineageExp::AGP__Inst_SetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const size_t move = dreamboard.GetPosID(move_x, move_y);
  hw.SetTrait(TRAIT_ID__MOVE, move);
}
// AGP__Inst_SetMoveID
void LineageExp::AGP__Inst_SetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const size_t move_id = hw.regs[inst.args[0]];
  hw.SetTrait(TRAIT_ID__MOVE, move_id);
}
// AGP__Inst_GetMoveXY
void LineageExp::AGP__Inst_GetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  const size_t move_x = dreamboard.GetPosX(move_id);
  const size_t move_y = dreamboard.GetPosY(move_id);
  hw.regs[inst.args[0]] =  move_x;
  hw.regs[inst.args[1]] =  move_y;
}
// AGP__Inst_GetMoveID
void LineageExp::AGP__Inst_GetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  hw.regs[inst.args[0]] =  move_id;
}
// AGP__Inst_IsValidXY
void LineageExp::AGP__Inst_IsValidXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_x, move_y);
  hw.regs[inst.args[2]] = valid;
}
// AGP__Inst_IsValidID_HW
void LineageExp::AGP__Inst_IsValidID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t move_id = hw.regs[inst.args[0]];
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_id);
  hw.regs[inst.args[1]] = valid;
}
// AGP__Inst_IsValidXY
void LineageExp::AGP__Inst_IsValidOppXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = dreamboard.GetOpponentID(hw.GetTrait(TRAIT_ID__PLAYER_ID));
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_x, move_y);
  hw.regs[inst.args[2]] = valid;
}
// AGP__Inst_IsValidID_HW
void LineageExp::AGP__Inst_IsValidOppID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = dreamboard.GetOpponentID(hw.GetTrait(TRAIT_ID__PLAYER_ID));
  const size_t move_id = hw.regs[inst.args[0]];
  const int valid = (int)dreamboard.IsMoveValid(playerID, move_id);
  hw.regs[inst.args[1]] = valid;
}
// AGP__Inst_AdjacentXY
void LineageExp::AGP__Inst_AdjacentXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const size_t dir = emp::Mod((int)inst.args[2], 8);
  const int nID = dreamboard.GetNeighbor(move_x, move_y, dir);
  if (nID == -1)
  {
    hw.regs[inst.args[0]] = AGENT_VIEW__ILLEGAL_ID;
    hw.regs[inst.args[1]] = AGENT_VIEW__ILLEGAL_ID;
  }
  else
  {
    hw.regs[inst.args[0]] = dreamboard.GetPosX(nID);
    hw.regs[inst.args[1]] = dreamboard.GetPosY(nID);
  }
}
// AGP__Inst_AdjacentID
void LineageExp::AGP__Inst_AdjacentID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = hw.regs[inst.args[0]];
  const size_t dir = emp::Mod((int)hw.regs[inst.args[1]], 8);
  const int nID = dreamboard.GetNeighbor(move_id, dir);
  (nID == -1) ? hw.regs[inst.args[0]] = AGENT_VIEW__ILLEGAL_ID : hw.regs[inst.args[0]] = nID;
}
// AGP_Inst_ValidMoveCnt_HW
void LineageExp::AGP__Inst_ValidMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  hw.regs[inst.args[0]] = dreamboard.GetMoveOptions(playerID).size();
}
// AGP_Inst_ValidOppMoveCnt_HW
void LineageExp::AGP__Inst_ValidOppMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  hw.regs[inst.args[0]] = dreamboard.GetMoveOptions(oppID).size();
}
// AGP_Inst_GetBoardValueXY_HW
void LineageExp::AGP__Inst_GetBoardValueXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  // If inputs are garbage, let the caller know.
  if (dreamboard.IsValidPos(move_x, move_y))
  {
    const size_t owner = dreamboard.GetPosOwner(move_x, move_y);
    if (owner == playerID)
    {
      hw.regs[inst.args[2]] = AGENT_VIEW__SELF_ID;
    }
    else if (owner == oppID)
    {
      hw.regs[inst.args[2]] = AGENT_VIEW__OPP_ID;
    }
    else
    {
      hw.regs[inst.args[2]] = AGENT_VIEW__OPEN_ID;
    }
  }
  else
  {
    hw.regs[inst.args[2]] = AGENT_VIEW__ILLEGAL_ID;
  }
}
// AGP_Inst_GetBoardValueID_HW
void LineageExp::AGP__Inst_GetBoardValueID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = hw.regs[inst.args[0]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  // If inputs are garbage, let the caller know.
  if (dreamboard.IsValidPos(move_id))
  {
    const size_t owner = dreamboard.GetPosOwner(move_id);
    if (owner == playerID)
    {
      hw.regs[inst.args[1]] = AGENT_VIEW__SELF_ID;
    }
    else if (owner == oppID)
    {
      hw.regs[inst.args[1]] = AGENT_VIEW__OPP_ID;
    }
    else
    {
      hw.regs[inst.args[1]] = AGENT_VIEW__OPEN_ID;
    }
  }
  else
  {
    hw.regs[inst.args[1]] = AGENT_VIEW__ILLEGAL_ID;
  }
}
// AGP_Inst_PlaceDiskXY_HW
void LineageExp::AGP__Inst_PlaceDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_x, move_y))
  {
    dreamboard.DoMove(playerID, move_x, move_y);
    hw.regs[inst.args[2]] = 1;
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_PlaceDiskID_HW
void LineageExp::AGP__Inst_PlaceDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = hw.regs[inst.args[0]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id))
  {
    dreamboard.DoMove(playerID, move_id);
    hw.regs[inst.args[1]] = 1;
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_PlaceOppDiskXY_HW
void LineageExp::AGP__Inst_PlaceOppDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_x, move_y))
  {
    dreamboard.DoMove(oppID, move_x, move_y);
    hw.regs[inst.args[2]] = 1;
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_PlaceOppDiskID_HW
void LineageExp::AGP__Inst_PlaceOppDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)hw.regs[inst.args[0]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id))
  {
    dreamboard.DoMove(oppID, move_id);
    hw.regs[inst.args[1]] = 1;
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_FlipCntXY_HW
void LineageExp::AGP__Inst_FlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const size_t move_id = dreamboard.GetPosID(move_x, move_y);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id))
  {
    hw.regs[inst.args[2]] = dreamboard.GetFlipList(playerID, move_id).size();
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_FlipCntID_HW
void LineageExp::AGP__Inst_FlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)hw.regs[inst.args[0]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  if (dreamboard.IsMoveValid(playerID, move_id))
  {
    hw.regs[inst.args[1]] = dreamboard.GetFlipList(playerID, move_id).size();
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_OppFlipCntXY_HW
void LineageExp::AGP__Inst_OppFlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const size_t move_id = dreamboard.GetPosID(move_x, move_y);
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id))
  {
    hw.regs[inst.args[2]] = dreamboard.GetFlipList(oppID, move_id).size();
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_OppFlipCntID_HW
void LineageExp::AGP__Inst_OppFlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)hw.regs[inst.args[0]];
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  const size_t oppID = dreamboard.GetOpponentID(playerID);
  if (dreamboard.IsMoveValid(oppID, move_id))
  {
    hw.regs[inst.args[1]] = dreamboard.GetFlipList(oppID, move_id).size();
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_FrontierCnt_HW
void LineageExp::AGP__Inst_FrontierCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t playerID = hw.GetTrait(TRAIT_ID__PLAYER_ID);
  hw.regs[inst.args[0]] = dreamboard.GetFrontierPosCnt(playerID);
}
// AGP_Inst_ResetBoard_HW
void LineageExp::AGP__Inst_ResetBoard_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_dreamware->ResetActive(testcases[cur_testcase].GetInput().game);
}
// AGP_Inst_IsOver_HW
void LineageExp::AGP__Inst_IsOver_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  emp::Othello &dreamboard = othello_dreamware->GetActiveDreamOthello();
  hw.regs[inst.args[0]] = (int)dreamboard.IsOver();
}

#endif
