#include <iostream>
#include <unistd.h>

#include "thread_pool.hpp"

using namespace hrd29;

static int task = 0;

class PrintTask : public ITask
{
public:
    PrintTask() : ITask(HIGH) {}
    ~PrintTask() override = default;
    void Execute() override
    {
        std::cout << "Task = " << task << std::endl; fflush(stdout);
    }
};

class AddToTask : public ITask
{
public:
    AddToTask() {}
    ~AddToTask() override = default;
    void Execute() override { ++task; }
};

class SubFromTask : public ITask
{
public:
    SubFromTask() {}
    ~SubFromTask() override = default;
    void Execute() override { --task; }
};

class MulBy5Task : public ITask
{
public:
    MulBy5Task() : ITask(LOW) {}
    ~MulBy5Task() override = default;
    void Execute() override { task *= 5; }
};




int main()
{
    ThreadPool task_pool(4);
    std::cout << "Number of Threads in the Pool: " <<
    std::max(std::thread::hardware_concurrency(),  static_cast<unsigned int>(1)) << std::endl;

    task_pool.SetSize(8);
    task_pool.SetSize(4);
    

    task_pool.AddTask(std::make_shared<PrintTask>());
    sleep(1);
    task_pool.AddTask(std::make_shared<AddToTask>());
    sleep(1);
    task_pool.AddTask(std::make_shared<PrintTask>());
    sleep(1);
    task_pool.AddTask(std::make_shared<MulBy5Task>());
    sleep(1);
    task_pool.AddTask(std::make_shared<SubFromTask>());
    sleep(1);
    task_pool.AddTask(std::make_shared<PrintTask>());
    sleep(1);
    // task = 4


    //task_pool.SetSize(6);
    task_pool.Suspend();
    task_pool.AddTask(std::make_shared<PrintTask>());   // HIGH
    task_pool.AddTask(std::make_shared<AddToTask>());   // MEDIUM
    task_pool.AddTask(std::make_shared<PrintTask>());   // HIGH
    task_pool.AddTask(std::make_shared<MulBy5Task>());  // LOW
    task_pool.AddTask(std::make_shared<SubFromTask>()); // MEDIUM
    task_pool.AddTask(std::make_shared<PrintTask>());   // HIGH
    sleep(1);
    task_pool.Resume();
    sleep(1);
    // PrintTask - task = 4, 3 times   // HIGH
    // AddToTask - task = 4 + 1 = 5    // MEDIUM
    // SubFromTask - task = 5 - 1 = 4  // MEDIUM
    // MulBy5Task - task = 4 * 5 = 20  // LOW

    task_pool.AddTask(std::make_shared<PrintTask>());
    //sleep(3);
    // task = 20


    return 0;
}


