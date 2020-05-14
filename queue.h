#pragma once

#define MAX_QUEUE_SIZE 102400

typedef struct Queue
{
  i32 size;
  i32 front;
  i32 back;
  i32 queue[MAX_QUEUE_SIZE];
} Queue;

void InitPQ(Queue *q)
{
  q->size = 0;
  q->front = -1;
  q->back = -1;
}

void Enqueue(Queue *q, i32 index)
{
  if(q->size < MAX_QUEUE_SIZE)
  {
    if(q->size <= 0)
    {
      q->queue[0] = index;
    }
    else if(q->back == MAX_QUEUE_SIZE - 1)
    {
      q->queue[0] = index;
      q->back = 0;
      q->size += 1;
    }
    else
    {
      q->queue[q->back + 1] = index;
      q->back += 1;
      q->size += 1;
    }
  }
  else
  {
    printf("The Queue is full\n");
  }
}

void Dequeue(Queue *q)
{
  if(q->size == 0)
  {
    printf("The PathQueue is empty\n");
  }
  else
  {
    q->size -= 1;
    q->front += 1;
  }
}

i32 Peek(Queue *q)
{
  return q->queue[q->front];
}

i32 IsQueueEmpty(Queue *q)
{
  return q->size <= 0;
}

////////////////////////////////////////////////////

typedef struct PQueue
{
  i32 size;
  i32 queue[MAX_QUEUE_SIZE];
  f32 dist[MAX_QUEUE_SIZE];
} PQueue;

void InitPQueue(PQueue *pq)
{
  pq->size = 0;
}

void EnqueuePQueue(PQueue *pq, i32 index, f32 dist)
{
  if(pq->size < MAX_QUEUE_SIZE)
  {
    if(pq->size <= 0)
    {
      pq->queue[0] = index;
      pq->dist[0] = dist;
      pq->size = 1;
    }
    else
    {
      i32 i;
      for(i = pq->size - 1; i >= 0; i--)
      {
        if(dist > pq->dist[i])
        {
          pq->queue[i + 1] = pq->queue[i];
          pq->dist[i + 1] = pq->dist[i];
        }
        else
        {
          break;
        }
      }

      pq->queue[i + 1] = index;
      pq->dist[i + 1] = dist;
      pq->size += 1;
    }
  }
  else
  {
    printf("The PQueue is full\n");
  }
}

void DequeuePQueue(PQueue *pq)
{
  if(pq->size == 0)
  {
    printf("The PQueue is empty\n");
  }
  else
  {
    pq->size -= 1;
  }
}

i32 PeekPQueue(PQueue *pq)
{
  return pq->queue[pq->size - 1];
}

i32 IsEmptyPQueue(PQueue *pq)
{
  return pq->size <= 0;
}
