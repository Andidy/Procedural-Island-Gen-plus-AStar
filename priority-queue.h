#pragma once

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <queue>
#include <tuple>
#include <algorithm>

using namespace std;

template<typename T, typename priority_t>
struct PriorityQueue{
  typedef pair<priority_t, T> PQElement;
  priority_queue<PQElement, vector<PQElement>, greater<PQElement>> elements;

  inline bool empty() const { return elements.empty(); }
  inline void put(T item, priority_t priority){ elements.emplace(priority, item); }
  T get() {
    T top = elements.top().second;
    elements.pop();
    return top;
  }
};

#endif
