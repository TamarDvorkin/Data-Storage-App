
#include <queue>
#include <iostream>
#include <thread>
#include <vector>
#include "pq.hpp"


using namespace hrd29;
bool MyCompare(const int &x, const int &y);

int main()
{   
    WPQ<int> wpq;

    // Producer thread
    std::thread producer([&wpq]() 
    {
        for (int i = 10; i > 0; --i) 
        {
            wpq.Push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Consumer thread
    std::thread consumer([&wpq]() 
    {
        for (int i = 0; i < 10; ++i) 
        {
            int value = wpq.Pop();
            std::cout << "Popped: " << value << std::endl;
        }
    });

    producer.join();
    consumer.join();

    return 0;
}





