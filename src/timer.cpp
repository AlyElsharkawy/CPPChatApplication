#include <mutex>
#include <thread>
#include "timer.h"

using namespace std;

Timer::Timer(int expectedTime)
{
  this->expectedTime = expectedTime;
  this->currentState = false;
}

void Timer::configureTime(int newTime)
{
  this->expectedTime = newTime;
}

void Timer::timerThreadFunction()
{
  while(true)
  {
    if(this->currentState == false)
    {
      this_thread::sleep_for(chrono::seconds(this->expectedTime));
      mtx.lock();
      this->currentState = true;
      mtx.unlock();
    }
  }
}

void Timer::initializeTimer()
{
  thread tf = thread([&](){Timer::timerThreadFunction();});
}

void Timer::setState(bool toSet)
{
  mtx.lock();
  this->currentState = toSet;
  mtx.unlock();
}

bool Timer::getState()
{
  return this->currentState;
}
