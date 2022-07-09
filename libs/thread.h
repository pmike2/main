#ifndef THREAD_H
#define THREAD_H

/*
Template d'une queue thread-safe
*/


#include <mutex>
#include <queue>
#include <cstddef>


template<class T>
class SafeQueue {

	std::queue<T> q;
	std::mutex m;

public:

	SafeQueue() {}

	void push(T elem) {
		std::lock_guard<std::mutex> lock(m);
		q.push(elem);
	}

	bool next(T& elem) {
		std::lock_guard<std::mutex> lock(m);
		if (q.empty()) {
			return false;
		}
		elem= q.front();
		q.pop();
		return true;
	}

};

/*
#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class SafeQueue
{
public:
  SafeQueue(void)
    : q()
    , m()
    , c()
  {}

  ~SafeQueue(void)
  {}

  void push(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  T next(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};
*/

#endif
