#pragma once

#include <queue>
#include <vector>
#include <unordered_map>
#include <list>

#include "priority-queue.h"
#include "proc-gen.h"

// Basing implementation on
// https://www.redblobgames.com/pathfinding/a-star/implementation.html
// but adapted to array based grids and the 4 directional nature of grid neighbors

using namespace std;

i32 Index(Vector2 xy, i32 width)
{
  return xy.y * width + xy.x;
}

i32 UpNeighbor(i32 index, i32 width){
  return  index - width;
}

i32 DownNeighbor(i32 index, i32 width){
  return index + width;
}

i32 LeftNeighbor(i32 index){
  return index - 1;
}

i32 RightNeighbor(i32 index){
  return index + 1;
}

bool IsForestedOrWater(i32 index, GameState *gs){
  if(gs->heightmap[index] < 200){
   return true;
  }
  if(gs->forestmap[index] == 1){
    return true;
  }
  return false;
}

double Weight(i32 curI, i32 nextI, GameState *gs){
  // using the difference height map
  return abs(gs->heightmap[nextI] - gs->heightmap[curI]);
}

Vector2 Vector(i32 index, GameState *gs){
  Vector2 tmp = Vector2();
  tmp.x = index % gs->map_width;
  tmp.y = index / gs->map_width;
  return tmp;
}

double Heuristic(i32 index, Vector2 goal, GameState *gs){
  Vector2 cur = Vector(index, gs);
  return (abs(cur.x - goal.x) + abs(cur.y - goal.y));
}

void AStar(GameState *gs)
{
  PriorityQueue<int, double> frontier; //= PriorityQueue<int, double>();  // Priority Queue to traverse grid
  unordered_map<int, int> from = unordered_map<int, int>();        // A map containing each node's previous path node
  unordered_map<int, double> pathCost = unordered_map<int, double>(); // a map containing the lowest cost to get to any discovered grid node
  gs->path = new list<int>();
  bool goalFound = false;

  int startI = Index(gs->player_pos, gs->map_width); // index of character's startng positin
  int goalI = Index(gs->target_pos, gs->map_width);

  // initialize starting position values
  frontier.put(startI, 0.0);
  from[startI] = startI;
  pathCost[startI] = 0.0;

  printf("Start: %d, %d, %d\n", startI, startI % gs->map_width, startI / gs->map_width);
  printf("Target: %d, %d, %d\n", goalI, goalI % gs->map_width, goalI / gs->map_width);

  while(!frontier.empty()) // while we have more nodes to check / traverse
  {
    int curI = frontier.get();

    //printf("Current: %d\n", curI);

    if(curI == goalI) { // success case
      goalFound = true;
      printf("GOAL FOUND!\n");
      break;
    }
    //printf("Visiting: %d\n", current);
    i32 leftI = LeftNeighbor(curI);
    i32 RightI = RightNeighbor(curI);
    i32 UpI = UpNeighbor(curI,  gs->map_width);
    i32 DownI = DownNeighbor(curI,  gs->map_width);

    //printf("lefti: %d, righti: %d, upi: %d, downi: %d\n", leftI, RightI, UpI, DownI);

    // if height map area > 200 then it's land
    // if we divide height map by 10 then it's > 20
    // forested: 0 = nonforested 1 = forested

    if(!IsForestedOrWater(leftI, gs))
    {
      // calculate cost by adding up the path with the new Weight
      double newCost = pathCost[curI] + Weight(curI, leftI, gs);

      // if the index hasn't been discovered or if we found a shorter path
      if(pathCost.find(leftI) == pathCost.end() || newCost < pathCost[leftI])
      {

          // initialize everything to represent the new calculated numbers
          pathCost[leftI] = newCost;
          double priority = newCost + Heuristic(leftI, gs->target_pos, gs);
          frontier.put(leftI, priority);
          from[leftI] = curI;
      }
    }

    if(!IsForestedOrWater(RightI, gs))
    {
      // calculate cost by adding up the path with the new Weight
      double newCost = pathCost[curI] + Weight(curI, RightI, gs);

      // if the index hasn't been discovered or if we found a shorter path
      if(pathCost.find(RightI) == pathCost.end() || newCost < pathCost[RightI])
      {

          // initialize everything to represent the new calculated numbers
          pathCost[RightI] = newCost;
          double priority = newCost + Heuristic(RightI, gs->target_pos, gs);
          frontier.put(RightI, priority);
          from[RightI] = curI;
      }
    }

    if(!IsForestedOrWater(UpI, gs))
    {
      // calculate cost by adding up the path with the new Weight
      double newCost = pathCost[curI] + Weight(curI, UpI, gs);

      // if the index hasn't been discovered or if we found a shorter path
      if(pathCost.find(UpI) == pathCost.end() || newCost < pathCost[UpI])
      {
          // initialize everything to represent the new calculated numbers
          pathCost[UpI] = newCost;
          double priority = newCost + Heuristic(UpI, gs->target_pos, gs);
          frontier.put(UpI, priority);
          from[UpI] = curI;
      }
    }

    if(!IsForestedOrWater(DownI, gs))
    {
      // calculate cost by adding up the path with the new Weight
      double newCost = pathCost[curI] + Weight(curI, DownI, gs);

      // if the index hasn't been discovered or if we found a shorter path
      if(pathCost.find(DownI) == pathCost.end() || newCost < pathCost[DownI])
      {
          // initialize everything to represent the new calculated numbers
          pathCost[DownI] = newCost;
          double priority = newCost + Heuristic(DownI, gs->target_pos, gs);
          frontier.put(DownI, priority);
          from[DownI] = curI;
      }
    }
  }

  if(!goalFound){
    return;
  }

  // finally put together the final path to return it
  i32 tmp = Index(gs->target_pos, gs->map_width);
  gs->path->push_front(tmp);
  while(from.find(tmp)->first != Index(gs->player_pos, gs->map_width))
  {
    tmp = from[tmp];
    gs->path->push_front(tmp);
  }
  return;
}
