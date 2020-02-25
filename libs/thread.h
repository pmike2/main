#ifndef THREAD_H
#define THREAD_H

#include <mutex>
#include <queue>
#include <cstddef>

/*
Template d'une queue thread-safe
*/

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

#endif
