#include "raylib.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "raymath.h"

#include <vector>
#include <list>

#include "proc-gen.h"
#include "simplex.h"
#include "priority-queue.h"
#include "astar.h"

// Map Functions ---------------------------------------------------------------
u32 ValidNeighbor(i32 neighbor, u32 width, u32 height)
{
  if (neighbor < 0) return false;
  if (neighbor >= (i32)(width * height)) return false;

  i32 x = neighbor % (i32)width;
  i32 y = neighbor / (i32)width;

  if (x < 0 || x == (i32)width)
    return false;
  else if (y < 0 || y == (i32)height)
    return false;
  else
    return true;
}
// End Map Funcs ---------------------------------------------------------------

// Procedural Generation -------------------------------------------------------
f32 ridgenoise(f64 x, f64 y)
{
  return 2.0 * (1.0 - fabs(1.0 - noise(x, y)));
}

void GenerateHeightMap(GameState *gs)
{
  // Generate a random offset based on seed
  srand(gs->seed);
  i32 xoffset = rand() % 2048;
  i32 yoffset = rand() % 2048;

  // loop through every location
  for(u32 y = 0; y < gs->map_height; y++)
  {
    for(u32 x = 0; x < gs->map_width; x++)
    {
      // generate inital noise layer
      f32 frequency = 2.0;
      u32 octaves = 3;
      f32 amplitude = 1.0;
      f32 range = 1.0;

      f32 posx = ((x / (f32)gs->map_width) - 0.5) * frequency;
      f32 posy = ((y / (f32)gs->map_height) - 0.5) * frequency;

      f32 n = noise(posx + xoffset, posy + yoffset);

      // layer more noise onto the inital noise to create a more organic image
      for(u32 o = 0; o < octaves; o++)
      {
        frequency = frequency * 2.0;
        amplitude = amplitude * 0.5;
        range = range + amplitude;
        n = n + 0.5 * ridgenoise(posx * frequency, posy * frequency)
          * amplitude * n;
        n = n + 0.5 * noise(posx * frequency, posy * frequency)
          * amplitude;
      }
      n = n / range;

      // use upper and lower bounding functions to further shape noise
      // d = normalizeDistance(x, y, width / 2, height / 2);
      f32 d = sqrt(pow((gs->map_width / 2.0) - x, 2.0)
          + pow((gs->map_height / 2.0) - y, 2.0))
        / (gs->map_width / 2.0);

      // n = n * (upper(d) - lower(d)) + lower(d);
      //n = n * ((1 - pow(d, 3.5)) - (1 - fabs(d))) + 0.4 * (1 - fabs(d));
      n = n * ((1 - pow(d, 3.5)) - (1 - pow(d, 1.5))) + 0.4 * (1 - pow(d, 1.5));


      n = Clamp(n, 0, 1);
      n = pow(n, 1.5);
      n *= 2550.0;

      // assign the generated noise data to its tile
      gs->heightmap[y * gs->map_width + x] = n;
} } }

void GenerateSlopeMap(GameState *gs)
{
  f32 radtopi = 180.0 / 3.14159265;

  for (u32 i = 0; i < gs->map_width * gs->map_height; i++)
  {
    f32 nw = 0.0;
    f32 n = 0.0;
    f32 ne = 0.0;
    f32 e = 0.0;
    f32 se = 0.0;
    f32 s = 0.0;
    f32 sw = 0.0;
    f32 w = 0.0;
    u32 num_neighbors = 0.0;

    if (ValidNeighbor(i - 1 - gs->map_width, gs->map_width, gs->map_height))
    {
      nw = atan(fabs(gs->heightmap[i] - gs->heightmap[i - 1 - gs->map_width]) / 10.0)
        * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i - gs->map_width, gs->map_width, gs->map_height))
    {
      n = atan(fabs(gs->heightmap[i] - gs->heightmap[i - gs->map_width]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i + 1 - gs->map_width, gs->map_width, gs->map_height))
    {
      ne = atan(fabs(gs->heightmap[i] - gs->heightmap[i + 1 - gs->map_width]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i + 1, gs->map_width, gs->map_height))
    {
      e = atan(fabs(gs->heightmap[i] - gs->heightmap[i + 1]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i + 1 + gs->map_width, gs->map_width, gs->map_height))
    {
      se = atan(fabs(gs->heightmap[i] - gs->heightmap[i + 1 + gs->map_width]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i + gs->map_width, gs->map_width, gs->map_height))
    {
      s = atan(fabs(gs->heightmap[i] - gs->heightmap[i + gs->map_width]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i - 1 + gs->map_width, gs->map_width, gs->map_height))
    {
      sw = atan(fabs(gs->heightmap[i] - gs->heightmap[i - 1 + gs->map_width]) / 10.0) * radtopi;
      num_neighbors += 1;
    }
    if (ValidNeighbor(i - 1, gs->map_width, gs->map_height))
    {
      w = atan(fabs(gs->heightmap[i] - gs->heightmap[i - 1]) / 10.0) * radtopi;
      num_neighbors += 1;
    }

    gs->slopemap[i] = (nw + n + ne + e + se + s + sw + w) / (f32)num_neighbors;
} }

void GenerateWaterMap(GameState *gs)
{
  // Generate a random offset based on seed
  srand(gs->seed);
  srand(rand());
  i32 xoffset = rand() % 2048;
  i32 yoffset = rand() % 2048;

  // loop through every location
  for(u32 y = 0; y < gs->map_height; y++)
  {
    for(u32 x = 0; x < gs->map_width; x++)
    {
      // generate inital noise layer
      f32 frequency = 2.0;
      u32 octaves = 3;
      f32 amplitude = 1.0;
      f32 range = 1.0;

      f32 posx = ((x / (f32)gs->map_width) - 0.5) * frequency;
      f32 posy = ((y / (f32)gs->map_height) - 0.5) * frequency;

      f32 n = noise(posx + xoffset, posy + yoffset);

      // layer more noise onto the inital noise to create a more organic image
      for(u32 o = 0; o < octaves; o++)
      {
        frequency = frequency * 2.0;
        amplitude = amplitude * 0.5;
        range = range + amplitude;
        n = n + 0.5 * ridgenoise(posx * frequency, posy * frequency)
          * amplitude * n;
        n = n + 0.5 * noise(posx * frequency, posy * frequency)
          * amplitude;
      }
      n = n / range;

      n = pow(n, 1.5);

      /*
      // use upper and lower bounding functions to further shape noise
      // d = normalizeDistance(x, y, width / 2, height / 2);
      f32 d = sqrt(pow((width / 2.0) - x, 2.0) + pow((height / 2.0) - y, 2.0))
        / (width / 2.0);

      // n = n * (upper(d) - lower(d)) + lower(d);
      n = n * ((1 - pow(d, 3.5)) - (1 - fabs(d))) + 0.4 * (1 - fabs(d));
      */
      n = Clamp(n, 0, 1);
      n = pow(n, 3.0f);
      n *= 255.0;

      // assign the generated noise data to its tile
      //u8 adjusted = 255 - (u8)n;
      u8 adjusted = (u8)n;
      gs->watermap[y * gs->map_width + x] = adjusted;
} } }

void GenerateForestMap(GameState *gs)
{
  for(u32 y = 0; y < gs->map_height; y++)
  {
    for(u32 x = 0; x < gs->map_width; x++)
    {
      i32 index = y * gs->map_width + x;
      f32 e = gs->heightmap[index] / 10.0;
      if(e > 25 && e < 70)
      {
        if(gs->watermap[index] > 55)
        {
          gs->forestmap[index] = 1;
        }
        else
        {
          gs->forestmap[index] = 0;
} } } } }
// End Proc Gen ----------------------------------------------------------------

void UpdateMapDrawData(GameState *gs)
{
  for (u32 i = 0; i < gs->map_height * gs->map_width; i++)
  {
    f32 e = gs->heightmap[i] / 10.0;

    if (gs->mapmode == HEIGHTMAP)
    {
      if (e > 20)
        gs->map_data[i] = /*(Color)*/{(u8)e, (u8)e, (u8)e, 255};
      else
        gs->map_data[i] = /*(Color)*/{(u8)e, (u8)e, 255, 255};
    }
    else if (gs->mapmode == SLOPEMAP)
    {
      u32 s = gs->slopemap[i];

      if (e > 60)
      {
        if (s > 70) gs->map_data[i] = MAROON;
        else if (s > 50) gs->map_data[i] = ORANGE;
        else if (s > 30) gs->map_data[i] = DARKGREEN;
        else /*(s >= 0)*/ gs->map_data[i] = DARKPURPLE;
      }
      else if (e > 40)
      {
        if (s > 70) gs->map_data[i] = RED;
        else if (s > 50) gs->map_data[i] = GOLD;
        else if (s > 30) gs->map_data[i] = LIME;
        else /*(s >= 0)*/ gs->map_data[i] = VIOLET;
      }
      else if (e > 20)
      {
        if (s > 70) gs->map_data[i] = PINK;
        else if (s > 50) gs->map_data[i] = YELLOW;
        else if (s > 30) gs->map_data[i] = GREEN;
        else /*(s >= 0)*/ gs->map_data[i] = PURPLE;
      }
      else
      {
        gs->map_data[i] = BLUE;
    } }
    else if (gs->mapmode == SIMPLESLOPEMAP)
    {
      u32 s = gs->slopemap[i];

      if (e > 20)
      {
        if (s > 60) gs->map_data[i] = RAYWHITE;
        else if (s > 40) gs->map_data[i] = LIGHTGRAY;
        else if (s > 20) gs->map_data[i] = BEIGE;
        else /*(s >= 0)*/ gs->map_data[i] = GREEN;
      }
      else
      {
        gs->map_data[i] = BLUE;
    } }
    else if (gs->mapmode == WATERMAP)
    {
      u8 w = gs->watermap[i];

      if (w >= 188) gs->map_data[i] = DARKBLUE;
      else if (w >= 125) gs->map_data[i] = BLUE;
      else if (w >= 55) gs->map_data[i] = SKYBLUE;
      else /*(w >= 0)*/ gs->map_data[i] = YELLOW;
    }
    else if (gs->mapmode == FORESTMAP)
    {
      u8 f = gs->forestmap[i];
      f32 e = gs->heightmap[i] / 10.0;

      if(f && (e > 20)) gs->map_data[i] = DARKGREEN;
      else if(!f && (e > 20)) gs->map_data[i] = GREEN;
      else gs->map_data[i] = BLUE;
    }
    else if (gs->mapmode == THEGOODONE)
    {
      u8 f = gs->forestmap[i];
      u32 s = gs->slopemap[i];
      f32 e = gs->heightmap[i] / 10.0f;

      if(e > 20)
      {
        if (s > 75) gs->map_data[i] = RAYWHITE;
        else if (s > 63) gs->map_data[i] = LIGHTGRAY;
        else if (s > 45) gs->map_data[i] = BEIGE;
        else if (s > 20) gs->map_data[i] = LIME;
        else /*(s >= 0)*/ gs->map_data[i] = GREEN;

        if (f) gs->map_data[i] = DARKGREEN;
      }
      else
      {
        gs->map_data[i] = BLUE;
  } } }

  gs->map_data_img = LoadImageEx(gs->map_data, gs->map_width, gs->map_height);
}

int main(void)
{
  // Create Window
  const i32 screenWidth = 1920;
  const i32 screenHeight = 1000;
  InitWindow(screenWidth, screenHeight, "PROC-GEN");
  SetTargetFPS(60);

  Vector2 origin;
  origin.x = 0.0;
  origin.y = 0.0;

  f32 scale = 3;
  Vector2 cursorposition;

  //Camera2D camera = {0};
  //camera.target = origin;
  //camera.offset = (Vector2){screenWidth / 2, screenHeight / 2};
  //camera.rotation = 0.0f;
  //camera.zoom = 1.0f;

  // Initialize State -----------------------------------------------------
  GameState *gs = (GameState *)malloc(sizeof(GameState));
  gs->mapmode = 5;
  gs->mapmode_new = 0;
  gs->map_reset = 0;
  gs->map_height = 256;
  gs->map_width = 256;
  gs->seed = 1234;
  gs->new_target_set = false;
  gs->invalid_player_pos = false;
  gs->player_pos = /*(Vector2)*/{128, 128};
  gs->target_pos = /*(Vector2)*/{0, 0};

  // Generate World ------------------------------------------------------------

  // type arr_name[width * height]; is equivalent to:
  // type *arr_name = malloc(width * height * sizeof(type));
  gs->heightmap = (f32 *)malloc(gs->map_width * gs->map_height * sizeof(f32));
  gs->slopemap = (u32 *)malloc(gs->map_width * gs->map_height * sizeof(u32));
  gs->watermap = (u8 *)malloc(gs->map_width * gs->map_height * sizeof(u8));
  gs->forestmap = (u8 *)malloc(gs->map_width * gs->map_height * sizeof(u8));

  GenerateHeightMap(gs);
  GenerateSlopeMap(gs);
  GenerateWaterMap(gs);
  GenerateForestMap(gs);

  // To display world
  gs->map_data = (Color *)malloc(gs->map_width * gs->map_height * sizeof(Color));
  Texture2D map_tex = {0};

  UpdateMapDrawData(gs);
  map_tex = LoadTextureFromImage(gs->map_data_img);
  //UnloadImage(gs->map_data_img);

  while (!WindowShouldClose())
  {
    // Update Game State -------------------------------------------------------
    if (IsKeyDown(KEY_A))
    {
      gs->mapmode_new = 0;
    }
    if (IsKeyDown(KEY_S))
    {
      gs->mapmode_new = 1;
    }
    if (IsKeyDown(KEY_D))
    {
      gs->mapmode_new = 3;
    }
    if (IsKeyDown(KEY_F))
    {
      gs->mapmode_new = 4;
    }
    if (IsKeyDown(KEY_G))
    {
      gs->mapmode_new = 5;
    }
    if (IsKeyDown(KEY_R))
    {
      gs->seed = rand();
      GenerateHeightMap(gs);
      GenerateSlopeMap(gs);
      GenerateWaterMap(gs);
      GenerateForestMap(gs);
      gs->map_reset = 1;
      if((gs->heightmap[Index(gs->player_pos, gs->map_width)] / 10.0f) <= 20.0f)
      {
        gs->invalid_player_pos = true;
      }
    }

    Vector2 pos = GetMousePosition();
    //printf("---------------------------------------------------------------\n");
    //printf("Debug Mouse: %f, %f\n", pos.x, pos.y);
    pos.x = pos.x / (f32)scale;
    pos.y = pos.y / (f32)scale;
    pos.x = floor(pos.x);
    pos.y = floor(pos.y);
    //printf("Debug Mouse: %f, %f\n", pos.x, pos.y);
    if ((pos.x < 0 || pos.x >= gs->map_width) ||
        (pos.y < 0 || pos.y >= gs->map_height))
    {
      cursorposition = origin;
    }
    else
    {
      cursorposition = pos;
      if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
      {
        // Set Pathfinding Target
        gs->target_pos = pos;
        AStar(gs);
        if(!gs->path->empty()){
          gs->new_target_set = true;
        }
      }
      else if(IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
      {
        // Set Player Position
        if(gs->forestmap[Index(pos, gs->map_width)] ||
         ((gs->heightmap[Index(pos, gs->map_width)] / 10.0f) <= 20))
        {
          gs->invalid_player_pos = true;
        }
        else
        {
          gs->player_pos = pos;
          gs->invalid_player_pos = false;
        }
      }
    }

    // Render ------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(DARKGRAY);

    if((gs->mapmode_new != gs->mapmode) || gs->map_reset)
    {
      gs->mapmode = gs->mapmode_new;
      gs->map_reset = 0;
      UpdateMapDrawData(gs);
      UpdateTexture(map_tex, gs->map_data_img.data);
      UnloadImage(gs->map_data_img);
    }

    DrawTextureEx(map_tex, origin, 0, scale, WHITE);

    if(gs->new_target_set)
    {
      if((gs->player_pos.x != gs->target_pos.x) || (gs->player_pos.y != gs->target_pos.y))
      {
        i32 temp = gs->path->front();
        gs->player_pos.x = temp % gs->map_width;
        gs->player_pos.y = temp / gs->map_width;
        gs->path->pop_front();
      }
      else if((gs->player_pos.x == gs->target_pos.x) || (gs->player_pos.y == gs->target_pos.y))
      {
        gs->new_target_set = false;
      }

      for (std::list<int>::iterator it=gs->path->begin(); it != gs->path->end(); ++it)
      {
        DrawRectangleV(Vector2Scale(/*(Vector2)*/{(f32)(*it % gs->map_width), (f32)(*it / gs->map_width)}, scale), /*(Vector2)*/{scale, scale}, ORANGE);
      }
    }

    DrawRectangleV(Vector2Scale(gs->player_pos, scale), /*(Vector2)*/{scale, scale}, MAGENTA);

    char textstr[1024];
    i32 cursorx = (i32)trunc(cursorposition.x);
    i32 cursory = (i32)trunc(cursorposition.y);
    sprintf(
      textstr, "Mouse Pos: %d, %d || Elevation: %f, Slope: %d, Moisture: %d",
      cursorx,
      cursory,
      gs->heightmap[cursorx + cursory * gs->map_width],
      gs->slopemap[cursorx + cursory * gs->map_width],
      gs->watermap[cursorx + cursory * gs->map_width]);
    DrawText(
      textstr, origin.x + 10, origin.y + (scale * gs->map_height), 24, BLACK
    );


    char playerstr[1024];
    sprintf(
      playerstr, "Player Pos: %d, %d",
      (i32)(gs->player_pos.x),
      (i32)(gs->player_pos.y)
    );
    DrawText(
      playerstr, origin.x + 10 + (scale * gs->map_width),
      origin.y + 10, 24, BLACK
    );

    char targetstr[1024];
    sprintf(
      targetstr, "Target Pos: %d, %d",
      (i32)(gs->target_pos.x),
      (i32)(gs->target_pos.y)
    );
    DrawText(
      targetstr, origin.x + 10 + (scale * gs->map_width),
      origin.y + 34, 24, BLACK
    );

    if(gs->invalid_player_pos)
    {
      char errorstr[1024];
      sprintf(
        errorstr, "You are trying to place the agent\non a forest or in the ocean\nwhich is not a valid position"
      );
      DrawText(
        errorstr, origin.x + 10 + (scale * gs->map_width),
        origin.y + 58, 24, RED
      );
    }

    char helpstr[1024];
    sprintf(
      helpstr, "Mapmodes and other buttons:\nLeftClick: pathfind to clicked grid cell\nRightClick: move agent to clicked grid cell\nA: Heightmap\nS: Slopemap\nD: Watermap\nF: Forestmap\nG: Fancymap"
    );
    DrawText(
      helpstr, origin.x + 10 + (scale * gs->map_width),
      origin.y + 200, 24, BLACK
    );


    char legendstr[1024];
    sprintf(
      legendstr, "Legend:"
    );
    DrawText(
      legendstr, origin.x + 10 + (scale * gs->map_width),
      origin.y + 600, 24, YELLOW
    );

    char legend2str[1024];

    switch(gs->mapmode)
    {
      case HEIGHTMAP:
      sprintf(
        legend2str, "Heightmap: Black = Low Elevation, White = High Elevation, Blue = Water"
      );break;
      case SLOPEMAP:
      sprintf(
        legend2str, "Slopemap: Darker Color = Higher Elevation, Lighter Color = Lower Elevation\nPurple->Green->Yellow->Red : Slope increases left to right"
      );break;
      case WATERMAP:
      sprintf(
        legend2str, "Watermap: Darker BLUE = More Rain"
      );break;
      case FORESTMAP:
      sprintf(
        legend2str, "ForestMap: Light Green = No forest, Dark Green = forest"
      );break;
      case THEGOODONE:
      sprintf(
        legend2str, "Fancymap: It just looks pretty"
      );break;
    }


    DrawText(
      legend2str, origin.x + 10 + (scale * gs->map_width),
      origin.y + 624, 24, YELLOW
    );

    DrawFPS(1840, 10);
    EndDrawing();
    // End Render --------------------------------------------------------------
  }

  CloseWindow();
  return 0;
}
