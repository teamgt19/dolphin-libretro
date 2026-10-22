// RAVE (TeamGT): the VR hands as RAVE publishes them in Local\RAVE_TRACK_V1 (rave_track.h), for player 1's Wii Remote
// when this core runs inside RAVE. Windows only; elsewhere nothing is ever available.
#pragma once

#include "DolphinLibretro/rave_track.h"

namespace Libretro
{
namespace RaveTrack
{
// RAVE's block is there (RAVE runs in VR) - opened once and kept.
bool Present();
// A current copy of one hand (0 = main / right, 1 = other / left): false when RAVE isn't writing (lease run out),
// the hand isn't tracked, or the copy was torn twice in a row.
bool ReadHand(unsigned hand, rave_track_hand& out);
void Close();
}  // namespace RaveTrack
}  // namespace Libretro
