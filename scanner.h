#ifndef __SCANNER_H__
#define __SCANNER_H__
//------------------------------------------------------------------------------

#include <map>
#include <set>
#include <mutex>
#include "filehandler.h"
#include "folderscanner.h"
//------------------------------------------------------------------------------

class Scanner : public FolderScanner
{
  private:
    uint FCount = 0;
    uint Failures = 0;

    uint MaxThreadCount;

    std::set<std::string> FoldersToIgnore;

    // All files groupped by file size
    std::map<uint, std::set<FileHandler*>> AllFiles;

    // The variables in the group below the mutex are accessed by the
    // CompareThread and should always be mutex protected !!!
    std::mutex MLock;
    // FileID to file size mapping
    std::map<ulong, uint> SizesPerGroup;
    // Duplicated files by FileID
    std::map<ulong, std::set<FileHandler*>> DuplicateGroups;
    std::map<uint,std::set<FileHandler*>>::iterator ProcessingIT;

    void Cleanup();
    void OnFile(const std::string& aFile);
    bool OnFolder(const std::string& aFolder);
    // Override to suppress the errors
    void OnFolderError(const std::string& aFolder);

    // Called by the DoProcessing to get the set to process
    std::set<FileHandler*>& GetNextSet();

    // Called by the DoProcessing method to register duplicated file
    void RegisterDuplicate(FileHandler* aElement1, FileHandler* aElement2);

    void Scan(const std::string& aFolder);
    void Compare();

    std::mutex MState;
    std::string State;

  protected:
    virtual void ProcessingUpdate();
    void SetState(const std::string& aState);

    // Called by the processing threads to do the real job
    friend class CompareThread;
    bool DoProcessing();

    virtual std::string IToS(uint aInt);

  public:
    Scanner(uint aMaxThreadCount = 0);
    ~Scanner();

    std::set<ulong>        GetGroups();
    std::set<FileHandler*> GetFiles(ulong aID);
    ulong                  GetSizes(ulong aID);

    void Run(const std::string& aBaseFolder);
    void Run(std::set<std::string> aFoldersToScan);

    void IgnoreFolder(const std::string& aFolderToIgnore);

    std::string GetState();
};
//------------------------------------------------------------------------------

#endif //__SCANNER_H__
