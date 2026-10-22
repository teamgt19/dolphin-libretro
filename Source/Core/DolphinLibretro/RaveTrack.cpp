// RAVE (TeamGT): reader of Local\RAVE_TRACK_V1 - see rave_track.h for the layout, frames and units.
#include "DolphinLibretro/RaveTrack.h"

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Libretro
{
namespace RaveTrack
{
#ifdef _WIN32
static HANDLE s_mapping = nullptr;
static const volatile rave_track_block* s_block = nullptr;
static bool s_tried = false;
static ULONGLONG s_next_try = 0;

static bool Open()
{
  if (s_block)
    return true;
  // not there yet: look again now and then (RAVE makes it when it starts in VR)
  const ULONGLONG now = GetTickCount64();
  if (s_tried && now < s_next_try)
    return false;
  s_tried = true;
  s_next_try = now + 2000;
  s_mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, RAVE_TRACK_NAME_W);
  if (!s_mapping)
    return false;
  void* view = MapViewOfFile(s_mapping, FILE_MAP_READ, 0, 0, RAVE_TRACK_SIZE);
  if (!view)
  {
    CloseHandle(s_mapping);
    s_mapping = nullptr;
    return false;
  }
  s_block = static_cast<const volatile rave_track_block*>(view);
  if (s_block->magic != RAVE_TRACK_MAGIC || s_block->major != RAVE_TRACK_MAJOR || s_block->size < RAVE_TRACK_SIZE)
  {
    Close();
    return false;
  }
  return true;
}

bool Present()
{
  return Open();
}

bool ReadHand(unsigned hand, rave_track_hand& out)
{
  if (hand >= RAVE_TRACK_HANDS || !Open())
    return false;
  for (int attempt = 0; attempt < 2; attempt++)
  {
    const uint32_t before = s_block->sequence;
    MemoryBarrier();
    if (before & 1u)
      continue;  // RAVE is writing
    rave_track_block copy;
    std::memcpy(&copy, const_cast<const rave_track_block*>(s_block), sizeof(copy));
    MemoryBarrier();
    if (s_block->sequence != before)
      continue;  // torn
    if (!(copy.flags & RAVE_TRACK_ACTIVE) || GetTickCount64() >= copy.written_ms + copy.lease_ms)
      return false;
    out = copy.hands[hand];
    return (out.flags & RAVE_HAND_TRACKED) != 0;
  }
  return false;
}

void Close()
{
  if (s_block)
    UnmapViewOfFile(const_cast<rave_track_block*>(s_block));
  if (s_mapping)
    CloseHandle(s_mapping);
  s_block = nullptr;
  s_tried = false;
  s_mapping = nullptr;
}
#else
bool Present() { return false; }
bool ReadHand(unsigned, rave_track_hand&) { return false; }
void Close() {}
#endif
}  // namespace RaveTrack
}  // namespace Libretro
