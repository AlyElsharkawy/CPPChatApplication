#pragma once
#include <mutex>
#include <thread>

using namespace std;

class Timer
{
private:
  int expectedTime;
  bool currentState;
  void timerThreadFunction();
  thread timerThread;
  mutex mtx;

public:
  Timer(int expectedTime);
  bool getState();
  void setState(bool toSet);
  void configureTime(int newTime);
  void initializeTimer();
};
