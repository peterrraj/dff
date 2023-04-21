#include <zlib.h>
#include <sys/stat.h>
//------------------------------------------------------------------------------

#include "sha256.h"
#include "filehandler.h"
#include "errorreporter.h"
//------------------------------------------------------------------------------

FileHandler::FileHandler(const std::string& aFileName) :
  FileID(0),
  FileName(aFileName),
  HasFailed(false)
{
  struct stat StatBuf;
  if (stat(FileName.c_str(), &StatBuf) < 0)
    throw EFileHandler("Unable to get the file size for " + FileName);
  FileSize = StatBuf.st_size;
}
//------------------------------------------------------------------------------

FileHandler::~FileHandler()
{
}
//------------------------------------------------------------------------------

void FileHandler::CalcChecksum(bool aFull)
{
  unsigned char Buff[4 * 1024];

  FILE* InF = fopen(FileName.c_str(),"rb");
  if (InF == NULL)
    throw EFileHandler("Failed to open " + FileName);

  ulong CRC = crc32(0L, Z_NULL, 0);   // Initialize the seed
  SHA256 Sha256;
  try
  {
    // Skip recalculation of the "short" part of the checksum
    if (aFull && FileSize > sizeof(Buff))
      fseek(InF, sizeof(Buff), SEEK_SET);

    while (!feof(InF))
    {
      ulong R = fread(Buff, 1, sizeof(Buff), InF);
      if (ferror(InF))
        throw EFileHandler("Read failure for " + FileName);
      CRC = crc32(CRC, Buff, R);

      Sha256.Update((unsigned char*)Buff, R);
      if (!aFull)
        break;
    }
  }
  catch(...)
  {
    fclose(InF);
    throw;
  }
  fclose(InF);

  if (aFull)
  {
    LongChecksum = std::to_string(CRC) + Sha256.GetChecksum();
  }
  else
  {
    ShortChecksum = std::to_string(CRC) + Sha256.GetChecksum();

    // Optimization: Skip long check sum calculation for small files
    if (FileSize <= sizeof(Buff))
      LongChecksum = ShortChecksum;
  }
}
//------------------------------------------------------------------------------

const std::string& FileHandler::GetShortChecksum()
{
  if (ShortChecksum.length() < 1)
    CalcChecksum(false);
  return ShortChecksum;
}
//------------------------------------------------------------------------------

const std::string& FileHandler::GetLongChecksum()
{
  if (LongChecksum.length() < 1)
    CalcChecksum(true);
  return LongChecksum;
}
//------------------------------------------------------------------------------

uint FileHandler::GetCRC(const std::string& aString)
{
  return crc32(crc32(0L, Z_NULL, 0),                        // Init the seed
               (const unsigned char*)aString.c_str(),
               aString.length());
}
//------------------------------------------------------------------------------

ulong FileHandler::GetFileSize()
{
  return FileSize;
}
//------------------------------------------------------------------------------

const std::string& FileHandler::GetFileName()
{
  return FileName;
}
//------------------------------------------------------------------------------

bool FileHandler::operator==(FileHandler& aOther)
{
  // Never report file as equal if it has failed
  if (HasFailed)
    return false;
  if (this == &aOther)
    return true;

  try
  {
    return (GetFileSize() == aOther.GetFileSize()) &&
           (GetShortChecksum() == aOther.GetShortChecksum()) &&
           (GetLongChecksum() == aOther.GetLongChecksum());
  }
  catch(...)
  {
    HasFailed = true;
    Error("Failed to compare: " + FileName);
  }
  return false;
}
//------------------------------------------------------------------------------

ulong FileHandler::GetID()
{
  // There is a risk that the FileID will remain zero but that won't be that
  // bad and the risk is accepted
  if (FileID == 0)
    FileID = (((ulong)GetCRC(GetShortChecksum())) << 32) |
             GetCRC(GetLongChecksum());
  return FileID;
}
//------------------------------------------------------------------------------
