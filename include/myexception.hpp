
#ifndef _HRD29_BeezNAS_MyException
#define _HRD29_BeezNAS_MyException

#include <exception>
#include <string>


namespace hrd29
{

class MyException : public std::exception
{
public:
    explicit MyException(const std::string& errorMessage) : message(errorMessage) {}
    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

}//namespace hrd29

#endif /*_HRD29_BeezNAS_MyException*/

