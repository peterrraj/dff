#include "errorreporter.h"
//------------------------------------------------------------------------------

#include <mutex>
#include <iostream>
//------------------------------------------------------------------------------

std::mutex MErrorReporter;
//------------------------------------------------------------------------------

void Error(const std::string& aErrorMessage)
{
  std::lock_guard<std::mutex> Lock (MErrorReporter);

  std::cerr << aErrorMessage << std::endl;
}
//------------------------------------------------------------------------------
