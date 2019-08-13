#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

//template <class TrafficLightPhase>
//TrafficLightPhase MessageQueue<TrafficLightPhase>::receive()
TrafficLightPhase MessageQueue::receive()
{
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    TrafficLightPhase msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // will not be copied due to return value optimization (RVO) in C++
}

//template <class TrafficLightPhase>
//void MessageQueue<TrafficLightPhase>::send(TrafficLightPhase &&msg)
void MessageQueue::send(TrafficLightPhase &&msg)
{
    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    //decode message
    // std::string decodedTrafficPhase = "";
    // if (msg == 0) {
    //     decodedTrafficPhase = "TrafficLightPhase::red";
    // } else {
    //     decodedTrafficPhase = "TrafficLightPhase::green";
    // }

    // add vector to queue
    //std::cout << "Phase " << decodedTrafficPhase << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        if (_queue.receive() == TrafficLightPhase::green) {
            return;
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
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.


    // - and sends an update method to the message queue using move semantics

    //initialize
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    lastUpdate = std::chrono::system_clock::now();
    srand (time(NULL));
    int cycleDuration = rand() % (6-4+1) + 4; // duration of a single simulation cycle in ms

    while (true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //get time since last cycle
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if (timeSinceLastUpdate >= cycleDuration * 1000) {
            if (_currentPhase == 0) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }

            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = rand() % (6-4+1) + 4; // duration of a single simulation cycle in ms
            _queue.send(std::move(_currentPhase));
        }

    }
}

