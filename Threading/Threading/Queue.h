#ifndef QUEUE_H
#define QUEUE_H

#include "TheadSafeQueue.h"
namespace Threading
{
template<typename T> using Queue = TheadSafeQueue<T, stdwrap::Queue<T>>;
template<typename T> using PriorityQueue = TheadSafeQueue<T, stdwrap::PriorityQueue<T>>;
}
#endif // QUEUE_H
