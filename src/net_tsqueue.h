#pragma once

#include "net_common.h"

template<typename T>
class tsqueue
{
public:
	tsqueue() = default;
	tsqueue(const tsqueue<T>&) = delete;
	virtual ~tsqueue() { clear(); }

public:
	// Returns and maintains item at front of Queue
	const T& front()
	{
		scoped_lock lock(muxQueue);
		return deqQueue.front();
	}

	// Returns and maintains item at back of Queue
	const T& back()
	{
		scoped_lock lock(muxQueue);
		return deqQueue.back();
	}

	// Removes and returns item from front of Queue
	T pop_front()
	{
		scoped_lock lock(muxQueue);
		auto t = move(deqQueue.front());
		deqQueue.pop_front();
		return t;
	}

	// Removes and returns item from back of Queue
	T pop_back()
	{
		scoped_lock lock(muxQueue);
		auto t = move(deqQueue.back());
		deqQueue.pop_back();
		return t;
	}

	// Adds an item to back of Queue
	void push_back(const T& item)
	{
		scoped_lock lock(muxQueue);
		deqQueue.emplace_back(move(item));

		unique_lock<mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Adds an item to front of Queue
	void push_front(const T& item)
	{
		scoped_lock lock(muxQueue);
		deqQueue.emplace_front(move(item));

		unique_lock<mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Returns true if Queue has no items
	bool empty()
	{
		scoped_lock lock(muxQueue);
		return deqQueue.empty();
	}

	// Returns number of items in Queue
	size_t count()
	{
		scoped_lock lock(muxQueue);
		return deqQueue.size();
	}

	// Clears Queue
	void clear()
	{
		scoped_lock lock(muxQueue);
		deqQueue.clear();
	}

	void wait()
	{
		while (empty())
		{
			unique_lock<mutex> ul(muxBlocking);
			cvBlocking.wait(ul);
		}
	}

protected:
	mutex muxQueue;
	deque<T> deqQueue;
	condition_variable cvBlocking;
	mutex muxBlocking;
};
