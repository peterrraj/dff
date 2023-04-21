#ifndef __FOLDER_SCANNER_H__
#define __FOLDER_SCANNER_H__
//------------------------------------------------------------------------------

#include "eexception.h"
//------------------------------------------------------------------------------

class EFolderScanner : public EException
{
  public:
    EFolderScanner(const std::string& aMessage) : EException(aMessage) {};
};
//------------------------------------------------------------------------------

class FolderScanner
{
  private:

  protected:
    virtual void OnFile(const std::string& aFileName);
    // Called on folder processing. Returns true if the folder should be scanned
    virtual bool OnFolder(const std::string& aFolderName);

    // Called when unable to open a folder. By default throws an exception
    virtual void OnFolderError(const std::string& aFolderName);

  public:
    FolderScanner();
    virtual ~FolderScanner();

    virtual void Scan(const std::string& aFolder);
};
//------------------------------------------------------------------------------

#endif // __FOLDER_SCANNER_H__
