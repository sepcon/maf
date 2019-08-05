#ifndef QUEUE_H
#define QUEUE_H

#include "ThreadSafeQueue.h"

namespace maf {
namespace threading {

template<typename T> using Queue = ThreadSafeQueue<stdwrap::Queue<T>>;
template<typename T, typename Comp = std::less<T> > using PriorityQueue = ThreadSafeQueue<stdwrap::PriorityQueue<T, Comp>>;

}
}
#endif // QUEUE_H
