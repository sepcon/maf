#ifndef QUEUE_H
#define QUEUE_H

#include "ThreadSafeQueue.h"

namespace Threading
{

template<typename T> using Queue = ThreadSafeQueue<T, stdwrap::Queue<T>>;
template<typename T> using PriorityQueue = ThreadSafeQueue<T, stdwrap::PriorityQueue<T>>;

}
#endif // QUEUE_H
