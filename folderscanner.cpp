#include "folderscanner.h"
//------------------------------------------------------------------------------

#include <dirent.h>
#include <string.h>
//------------------------------------------------------------------------------

FolderScanner::FolderScanner()
{
}
//------------------------------------------------------------------------------

FolderScanner::~FolderScanner()
{
}
//------------------------------------------------------------------------------

void FolderScanner::OnFile(const std::string& aFileName)
{
}
//------------------------------------------------------------------------------

bool FolderScanner::OnFolder(const std::string& aFolderName)
{
  return true;
}
//------------------------------------------------------------------------------

void FolderScanner::OnFolderError(const std::string& aFolder)
{
  throw EFolderScanner("Unable to open " + aFolder + " directory");
}
//------------------------------------------------------------------------------

void FolderScanner::Scan(const std::string& aFolder)
{
  DIR* d_fh;
  struct dirent* entry;

  std::string Folder = aFolder;
  if (Folder.length() > 0 && Folder[Folder.length() - 1] != '/')
    Folder += '/';

  if ((d_fh = opendir(Folder.c_str())) == NULL)
  {
    OnFolderError(Folder);
    return;
  }

  try
  {
    while((entry=readdir(d_fh)) != NULL)
    {
      std::string Entry = Folder + entry->d_name;

      switch(entry->d_type)
      {
        case DT_DIR:
          // Ignore the "." and ".." folders
          if (strcmp(entry->d_name, "..") == 0 ||
              strcmp(entry->d_name, ".") == 0)
            continue;
          if (OnFolder(Entry))
            Scan(Entry);
          break;
        case DT_REG:
          OnFile(Entry);
          break;
      }
    }
    closedir(d_fh);
  }
  catch(...)
  {
    closedir(d_fh);
    throw;
  }
}
//------------------------------------------------------------------------------
