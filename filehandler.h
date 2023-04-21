#ifndef __FILE_HANDLER_H__
#define __FILE_HANDLER_H__
//------------------------------------------------------------------------------

#include <string>
//------------------------------------------------------------------------------

#include "eexception.h"
//------------------------------------------------------------------------------

class EFileHandler : public EException
{
  private:
  protected:
  public:
    EFileHandler(const std::string& aMessage) : EException(aMessage) {};
};
//------------------------------------------------------------------------------

class FileHandler
{
  private:
    ulong FileID;
    ulong FileSize;
    std::string FileName;

    bool HasFailed;
    std::string ShortChecksum;
    std::string LongChecksum;

    void CalcChecksum(bool aFull);

    const std::string& GetShortChecksum();
    const std::string& GetLongChecksum();

  protected:
    uint GetCRC(const std::string& aString);

  public:
    FileHandler(const std::string& aFileName);
    virtual ~FileHandler();

    ulong GetFileSize();
    const std::string& GetFileName();

    bool operator==(FileHandler& aOther);

    // This is VERY expensive operation as it calculates the FULL CRC
    ulong GetID();
};

//------------------------------------------------------------------------------
#endif // __FILE_HANDLER_H__
