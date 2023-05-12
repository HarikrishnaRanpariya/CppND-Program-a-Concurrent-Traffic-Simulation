#include <iostream>
#include <random>
#include <iostream>
#include <cstdlib>
#include "TrafficLight.h"


std::random_device rd;
std::mt19937 gen(rd());

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    // perform vector modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.clear();

    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add msg to queue
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();

}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    flag = false;
}

// Generate random number from given range
int TrafficLight::randomWithRange(int min, int max)
{
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
      TrafficLightPhase lPhase = msgQueue.receive();

      if(lPhase == TrafficLightPhase::green)
      {
          break;
      }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  std::chrono::time_point<std::chrono::system_clock> currUpdate;
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    while(true)
    {
      if(   (_currentPhase == TrafficLightPhase::red)
         && (flag == false))
      {
          flag = true;
          lastUpdate = std::chrono::system_clock::now();
      }
      else if(   (_currentPhase == TrafficLightPhase::green)
               && (flag == true))
      {
          flag = false;
          lastUpdate = std::chrono::system_clock::now();
      }

      currUpdate = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = currUpdate - lastUpdate;

      srand((unsigned) time(NULL));
      unsigned random =  4 + (rand() % 3);

      if (elapsed_seconds.count() > random)
      {
          if  (_currentPhase == TrafficLightPhase::red)
          {
              // std::cout << "Red -> green"<<std::endl;
              // create a new TrafficLightPhase instance and move it into the queue
              _currentPhase = TrafficLightPhase::green;
              msgQueue.send(std::move(_currentPhase));
          }
          else
          {
              // std::cout << "green -> red"<<std::endl;
              // create a new TrafficLightPhase instance and move it into the queue
              _currentPhase = TrafficLightPhase::red;
              msgQueue.send(std::move(_currentPhase));
          }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
