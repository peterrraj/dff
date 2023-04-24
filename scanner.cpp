#include <chrono>
#include <thread>
#include <vector>
//------------------------------------------------------------------------------

#include "scanner.h"
//------------------------------------------------------------------------------

#include "errorreporter.h"
//------------------------------------------------------------------------------

class MutexLock
{
  private:
    std::mutex& Mutex;

  protected:

  public:
    MutexLock(std::mutex& aMutex) : Mutex(aMutex)
    {
      Mutex.lock();
    }

    virtual ~MutexLock()
    {
      Mutex.unlock();
    }
};
//------------------------------------------------------------------------------


Scanner::Scanner(uint aMaxThreadCount) : MaxThreadCount(aMaxThreadCount)
{
  if (MaxThreadCount == 0)
    MaxThreadCount = std::thread::hardware_concurrency();
}
//------------------------------------------------------------------------------

Scanner::~Scanner()
{
  Cleanup();
}
//------------------------------------------------------------------------------

void Scanner::Scan(const std::string& aFolder)
{
  FolderScanner::Scan(aFolder);
}
//------------------------------------------------------------------------------

void Scanner::Cleanup()
{
  FCount = 0;
  Failures = 0;

  // Only holds the references to no cleanup
  DuplicateGroups.clear();

  std::map<uint, std::set<FileHandler*>>::iterator IT;
  for(IT = AllFiles.begin(); IT != AllFiles.end(); IT++)
  {
    std::set<FileHandler*>::iterator IT2;
    for (IT2 = IT->second.begin(); IT2 != IT->second.end(); IT2++)
      delete (*IT2);
    IT->second.clear();
  }
  AllFiles.clear();

  SizesPerGroup.clear();
}
//------------------------------------------------------------------------------

void Scanner::OnFile(const std::string& aFile)
{
  try
  {
    FileHandler* FH = new FileHandler(aFile);
    AllFiles[FH->GetFileSize()].insert(FH);
    FCount++;
  }
  catch(...)
  {
    // Not mutex protected as this method is never called by the processing
    Failures++;
  }
}
//------------------------------------------------------------------------------

bool Scanner::OnFolder(const std::string& aFolder)
{
  SetState("Scanning " + aFolder);
  ProcessingUpdate();

  std::string FolderName;

  size_t Pos = aFolder.find_last_of('/');
  if (Pos != std::string::npos)
    FolderName = aFolder.substr(Pos + 1);

  return (FoldersToIgnore.find(aFolder) == FoldersToIgnore.end()) &&
         (FoldersToIgnore.find(FolderName) == FoldersToIgnore.end());
}
//------------------------------------------------------------------------------

void Scanner::OnFolderError(const std::string& aFolder)
{
  // TODO: Save the failures and show them
  Error("Failed to open: " + aFolder);
}
//------------------------------------------------------------------------------

void Scanner::ProcessingUpdate()
{
}
//------------------------------------------------------------------------------

void Scanner::Compare()
{
  // Prepare the progress iterator
  ProcessingIT = AllFiles.begin();

  // Start the threads
  std::vector<std::thread> Threads;
  for (int T = 0; T < MaxThreadCount; T++)
  {
    Threads.push_back(
      std::move(
        std::thread([](Scanner* aOwner) { while(aOwner->DoProcessing());}, this)
      )
    );
  }

  // Status update loop
  while(true)
  {
    {
      MutexLock Lock(MLock);
      if (ProcessingIT == AllFiles.end())
        break;
    }
    ProcessingUpdate();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }

  SetState("Finishing the compare ...");

  // Wait all threads to terminate
  for (std::thread& Th : Threads)
  {
    if (Th.joinable())
      Th.join();
  }
}
//------------------------------------------------------------------------------

void Scanner::Run(const std::string& aBaseFolder)
{
  std::set<std::string> FoldersToScan;
  FoldersToScan.insert(aBaseFolder);
  Run(FoldersToScan);
}
//------------------------------------------------------------------------------

void Scanner::Run(std::set<std::string> aFoldersToScan)
{
  Cleanup();
  for (std::string Folder : aFoldersToScan)
    Scan(Folder);
  Compare();

  SetState("Done.");
  ProcessingUpdate();
}
//------------------------------------------------------------------------------

bool Scanner::DoProcessing()
{
  std::set<FileHandler*>& FilesSet = GetNextSet();

  if (FilesSet.size() < 1)
    return false;

  for (
    std::set<FileHandler*>::iterator IOut = FilesSet.begin();
    IOut != FilesSet.end();
    IOut++
  )
  {
    for (
      std::set<FileHandler*>::iterator IIn = IOut;
      IIn != FilesSet.end();
      IIn++
    )
    {
      if (IOut == IIn)
        continue;

      try
      {
        if (*(*IOut) == *(*IIn))
        {
          RegisterDuplicate(*IOut, *IIn);
          break;
        }
      }
      catch(EFileHandler& E)
      {
        MutexLock Lock(MLock);
        Failures++;
      }
    }
  }

  return true;
}
//------------------------------------------------------------------------------

std::string Scanner::IToS(uint aInt)
{
  return std::to_string(aInt);
}
//------------------------------------------------------------------------------

std::set<ulong> Scanner::GetGroups()
{
  std::set<ulong> Result;

  std::map<ulong,std::set<FileHandler*>>::iterator IT;
  for (IT = DuplicateGroups.begin(); IT != DuplicateGroups.end(); IT++)
    Result.insert(IT->first);
  return Result;
}
//------------------------------------------------------------------------------

std::set<FileHandler*> Scanner::GetFiles(ulong aID)
{
  return DuplicateGroups[aID];
}
//------------------------------------------------------------------------------

ulong Scanner::GetSizes(ulong aID)
{
  return SizesPerGroup[aID];
}
//------------------------------------------------------------------------------

void Scanner::IgnoreFolder(const std::string& aFolderToIgnore)
{
  FoldersToIgnore.insert(aFolderToIgnore);
}
//------------------------------------------------------------------------------

void Scanner::RegisterDuplicate(FileHandler* aElement1, FileHandler* aElement2)
{
  MutexLock Lock(MLock);

  ulong ID = aElement1->GetID();
  SizesPerGroup[ID] = aElement1->GetFileSize();
  DuplicateGroups[ID].insert(aElement1);
  DuplicateGroups[ID].insert(aElement2);
}
//------------------------------------------------------------------------------

std::set<FileHandler*>& Scanner::GetNextSet()
{
  // Not perfect but seems to be the only option
  static std::set<FileHandler*>EmptySet;

  MutexLock Lock(MLock);

  // Skip all sets with one file
  while(ProcessingIT != AllFiles.end() && ProcessingIT->second.size() < 2)
    ProcessingIT++;

  // Still something to process?
  if (ProcessingIT == AllFiles.end())
    return EmptySet;

  // Progress report
  SetState(
    "Comparing " + IToS(ProcessingIT->second.size()) +
    " files of size " + IToS(ProcessingIT->first)
  );

  return (ProcessingIT++)->second;
}
//------------------------------------------------------------------------------

void Scanner::SetState(const std::string& aNewState)
{
  MutexLock Lock(MState);
  State = aNewState;
}
//------------------------------------------------------------------------------

std::string Scanner::GetState()
{
  MutexLock Lock(MState);
  return State;
}
//------------------------------------------------------------------------------
