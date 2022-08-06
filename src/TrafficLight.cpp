#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);

    _cond.wait(uLock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());

    _queue.clear();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> myLock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true) 
    {
        TrafficLightPhase phase = _messageQueue.receive();

        if (phase == TrafficLightPhase::green)
        { return; }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


void TrafficLight::cycleThroughPhases()
{
    std::random_device randNum;
    std::mt19937 mt1(randNum());
    std::uniform_real_distribution<double> dist(4.0, 6.0);
    float cycleTime = dist(mt1);

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration =  t2 - t1;

       if (duration.count() > cycleTime) 
       {
            t1 = std::chrono::high_resolution_clock::now();
            cycleTime = dist(mt1);
            if(_currentPhase == TrafficLightPhase::green)
            {
                _currentPhase = red;
            }
            else
            {
                 _currentPhase = green;
            }
            TrafficLightPhase thisPhase = _currentPhase;
            _messageQueue.send(std::move(thisPhase));
        }
    }
}

