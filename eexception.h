#ifndef __EEXCEPTION_H__
#define __EEXCEPTION_H__
//------------------------------------------------------------------------------

#include <string>
//------------------------------------------------------------------------------

class EException : public std::exception
{
  private:
    std::string Message;
  protected:
  public:
    EException(const std::string aMessage) : Message(aMessage), exception() {}

    virtual std::string GetMessage() { return Message; }
};
//------------------------------------------------------------------------------

#endif // __EEXCEPTION_H__
