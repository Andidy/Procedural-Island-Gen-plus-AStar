#ifndef PROC_GEN_H
#define PROC_GEN_H

#include "typenames.h"

#define HEIGHTMAP      0
#define SLOPEMAP       1
#define SIMPLESLOPEMAP 2
#define WATERMAP       3
#define FORESTMAP      4
#define THEGOODONE     5
typedef struct GameState
{
  u32 seed;
  u32 map_height;
  u32 map_width;

  Vector2 player_pos;
  bool invalid_player_pos;
  bool new_target_set;
  Vector2 target_pos;
  std::list<int> *path;

  u32 mapmode;
  u32 mapmode_new;
  i32 map_reset;
  f32 *heightmap;
  u32 *slopemap;
  u8  *watermap;
  u8  *forestmap;

  Color *map_data;
  Image map_data_img;
} GameState;

#endif
