
#include <iostream>
#include "eventchannel.hpp"


using namespace hrd29;

class Message
{
public:
    Message(const std::string& content) : m_content(content) {}
    std::string getContent() const { return m_content; }

private:
    std::string m_content;
};

class Observer
{
public:
    void handleMessage(const Message& msg)
    {
        std::cout << "Observer received message: " << msg.getContent() << std::endl;
    }

    void stop()
    {
        std::cout << "Observer is stopping." << std::endl;
    }
};

int main()
{
    Dispatcher<Message>* dispatcher = new Dispatcher<Message>;

    Observer observer1;
    Observer observer2;

    Callback<Message, Observer> callback1(dispatcher, observer1, &Observer::handleMessage, &Observer::stop);
    Callback<Message, Observer> callback2(dispatcher, observer2, &Observer::handleMessage, &Observer::stop);

    Message msg1("Hello, observers!");
    dispatcher->NotifyAll(msg1);

    Message msg2("Another message.");
    dispatcher->NotifyAll(msg2);

    Message msg3("Final message.");
    dispatcher->NotifyAll(msg3);

    delete dispatcher;


    return 0;
}

