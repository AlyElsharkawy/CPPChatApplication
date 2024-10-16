#include <iostream>
#include "timer.h"

using namespace std;

int main()
{
  Timer tim(4);
  tim.initializeTimer();
  while(true)
  {
    if(tim.getState() == true)
    {
      cout << "Hambola!\n";
      tim.setState(false);
    }
  }
}
