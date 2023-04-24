#include "scanner.h"
#include "mainwindow.h"
#include "errorreporter.h"
//------------------------------------------------------------------------------

class ScannerImpl : public Scanner
{
  private:
    MainWindow* Owner;

  protected:
    void ProcessingUpdate();

    virtual std::string IToS(uint aInt)
    {
      return MainWindow::ToHRFormat(aInt);
    }

  public:
    ScannerImpl(
      MainWindow* aOwner,
      uint aThreads = 0)
    : Scanner(aThreads), Owner(aOwner) {};
};
//------------------------------------------------------------------------------


MainWindow::MainWindow() : Window(), ResultsBox(true, 0)
{
  add(ViewPort);
  ViewPort.pack_start(FoldersArea, false, true, 0);
    FoldersArea.pack_start(SourceFoldersArea, true, true, 5);
      SourceFoldersArea.pack_start(SourceFoldersLabel, false, false, 3);
      SourceFoldersArea.pack_start(SourceFoldersBox, true, true);
        SourceFoldersBox.pack_start(SourceFoldersFrame, true, true, 5);
          SourceFoldersFrame.add(SourceFoldersScroller);
            SourceFoldersScroller.add(GridSourceFolders);
        SourceFoldersBox.pack_start(SourceFoldersBtnBox, false, false);
          SourceFoldersBtnBox.pack_start(BtnAddSourceFolder, false, false);
          SourceFoldersBtnBox.pack_start(BtnDelSourceFolder, false, false);
          SourceFoldersBtnBox.pack_start(BtnScan, false, false);
    FoldersArea.pack_start(IgnoresArea, true, true, 0);
      IgnoresArea.pack_start(HdrLabel, false, true, 0);
      IgnoresArea.pack_start(Hdr1Row, true, true);
        Hdr1Row.pack_start(Hdr1Left, true, true);
          Hdr1Left.pack_start(IgnoreFrame, true, true);
            IgnoreFrame.add(IgnoreListScroller);
              IgnoreListScroller.add(GridIgnores);
        Hdr1Row.pack_start(Hdr1Right, false, false, 5);
          Hdr1Right.pack_start(BtnAddIgnore, false, false);
          Hdr1Right.pack_start(InputIgnorePat, false, false);
          Hdr1Right.pack_start(BtnAddIgnorePat, false, false);
          Hdr1Right.pack_start(BtnDelIgnore, false, false);

  ViewPort.pack_start(ResultsArea, true, true, 0);
    ResultsArea.pack_start(ResultsLabel, false, true, 0);
    ResultsArea.pack_start(ResultsBox, true, true, 5);
      ResultsBox.pack_start(Res1Row, false, true);
        Res1Row.pack_start(Res1Left, true, true);
          Res1Left.pack_start(ResultsFrame, true, true);
            ResultsFrame.add(ResultsBoxScroller);
              ResultsBoxScroller.add(Duplicates);
        Res1Row.pack_end(Res1Right, false, false, 5);
          Res1Right.pack_start(SelectPattern, false, false);
          Res1Right.pack_start(BtnSelectPattern, false, false);
          Res1Right.pack_start(BtnUnselectPattern, false, false);
          Res1Right.pack_start(BtnSelectAllButFirst, false, false);
          Res1Right.pack_start(BtnSelectAllButLast, false, false);
          Res1Right.pack_start(EmptyLine, false, false);
          Res1Right.pack_start(BtnDelete, false, false);
          Res1Right.pack_start(BtnLink, false, false);

  ViewPort.pack_end(StatusArea, false, true, 0);
    StatusArea.pack_end(Footer2Scroller, false, true);
      Footer2Scroller.add(FooterText);

  HdrLabel.set_label("Ignores");
  ResultsLabel.set_label("Results");
  SourceFoldersLabel.set_label("Source folders");

  BtnAddIgnore.set_label("Add Folder");
  BtnAddIgnore.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnIgnoreHandler)
  );

  BtnAddIgnorePat.set_label("Add pattern");
  BtnAddIgnorePat.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnIgnorePatHandler)
  );

  BtnDelIgnore.set_label("Delete");
  BtnDelIgnore.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnDelIgnoreHandler)
  );

  BtnAddSourceFolder.set_label("Add");
  BtnAddSourceFolder.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnAddSourceFolderHandler)
  );

  BtnDelSourceFolder.set_label("Delete");
  BtnDelSourceFolder.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnDelSourceFolderHandler)
  );

  BtnScan.set_label("Scan ...");
  BtnScan.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnScanHandler)
  );

  BtnSelectPattern.set_label("Select");
  BtnSelectPattern.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnSelectHandler)
  );

  BtnUnselectPattern.set_label("Unselect");
  BtnUnselectPattern.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnUnselectHandler)
  );

  BtnSelectAllButFirst.set_label("Select all but first");
  BtnSelectAllButFirst.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnSelectAllButFirstHandler)
  );

  BtnSelectAllButLast.set_label("Select all but last");
  BtnSelectAllButLast.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnSelectAllButLastHandler)
  );

  BtnDelete.set_label("Delete");
  BtnDelete.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnDeleteHandler)
  );
  BtnDelete.set_tooltip_text("Delete all selected files");

  BtnLink.set_label("Link");
  BtnLink.signal_button_release_event().connect(
    sigc::mem_fun(*this, &MainWindow::BtnLinkHandler)
  );
  BtnLink.set_tooltip_text(
    "Link all unselected files to the selected one in the group"
  );

  FooterText.set_text("No search was completed yet");

  UpdateButtons(true, false);

  set_title("DFF - Duplicate File Finder");
  resize(720, 480);
  show_all();
}
//------------------------------------------------------------------------------

void MainWindow::SafeLink(const std::string& aFile, const std::string& aLink)
{
  std::string TmpName = aLink + ".__tmp__";

  // First rename to a temporary file name
  if (rename(aLink.c_str(), TmpName.c_str()) != 0)
    throw EException("Failed to rename " + aLink);

  // Now try to create the symlink
  if (symlink(aFile.c_str(), aLink.c_str()) != 0)
  {
    // Try to restore the file name
    rename(TmpName.c_str(), aLink.c_str());
    throw EException("symlink creation failed for " + aLink);
  }

  if (unlink(TmpName.c_str()) != 0)
    throw EException("Failed to delete the temporary file " + TmpName);
}
//------------------------------------------------------------------------------

int MainWindow::MessageBox(
  const std::string& aTitle,
  const std::string& aText,
  Gtk::ButtonsType aButtons)
{
  Gtk::MessageDialog Dlg(*this,
    aTitle,
    false /* use_markup */,
    Gtk::MESSAGE_QUESTION,
    aButtons
  );
  Dlg.set_secondary_text(aText);
  Dlg.set_modal(true);
  return Dlg.run();
}
//------------------------------------------------------------------------------

bool MainWindow::on_delete_event(GdkEventAny* any_event)
{
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Duplicates.remove(*W);
    delete W;
  }
  for (Gtk::Widget* W: GridSourceFolders.get_children())
  {
    GridSourceFolders.remove(*W);
    delete W;
  }
  for (Gtk::Widget* W: GridIgnores.get_children())
  {
    GridIgnores.remove(*W);
    delete W;
  }
  return false;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnLinkHandler(GdkEventButton*)
{
  try
  {
    uint Links = 0;
    uint Groups = 0;
    bool Ignore = false;     // True if more than one file in a grup is unselected
    Gtk::CheckButton* FileToLinkTo = NULL;
    std::vector<Gtk::CheckButton*> FilesToLink;
    for (Gtk::Widget* W: Duplicates.get_children())
    {
      // Fill the list
      Gtk::CheckButton* Btn = dynamic_cast<Gtk::CheckButton*>(W);
      if (Btn)
      {
        if (Ignore)       // Ignore the group ?
          continue;

        // Reverse logic - link all unchecked files to the checked one
        if (!Btn->get_active())
        {
          FilesToLink.push_back(Btn);
        }
        else
        {
          if (FileToLinkTo)
            Ignore = true;
          FileToLinkTo = Btn;
        }
        continue;
      }

      Gtk::Label* Lbl = dynamic_cast<Gtk::Label*>(W);
      if (!Lbl)
        throw EException("Unexpected element found");

      if (!Ignore && (FileToLinkTo != NULL) && (FilesToLink.size() > 0))
      {
        for (Gtk::CheckButton* Btn: FilesToLink)
        {
          SafeLink(FileToLinkTo->get_label(), Btn->get_label());
          Links++;
          Duplicates.remove(*Btn);
          delete Btn;
        }
        Groups++;
        Duplicates.remove(*FileToLinkTo);
        delete FileToLinkTo;
      }
      else
      {
        // More than one selected or no selection at all in the group
      }

      // Do a cleanup
      Ignore = false;
      FileToLinkTo = NULL;
      FilesToLink.clear();
    }
    MessageBox(
      "Link",
      std::to_string(Links) + " files in " + std::to_string(Groups) +
      " groups were linked",
      Gtk::BUTTONS_OK
    );
  }
  catch(EException& E)
  {
    MessageBox("Link failure", E.GetMessage(),Gtk::BUTTONS_OK);
  }
  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnDeleteHandler(GdkEventButton*)
{
  uint Errors = 0;

  // The for workrs in reverse so it actually starts with the last element
  // and traverses back to the first
  uint FilesInTheGroup = 0;
  std::vector<Gtk::CheckButton*> ToDeleteW;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::Label* Lbl = dynamic_cast<Gtk::Label*>(W);
    Gtk::CheckButton* Btn = dynamic_cast<Gtk::CheckButton*>(W);

    if (Lbl)
    {
      // If nothing is selected just continue
      if (ToDeleteW.size() == 0)
      {
        FilesInTheGroup = 0;
        continue;
      }

      // Ask for confirmation if all files in a group are selected
      bool Delete = true;
      if (FilesInTheGroup == ToDeleteW.size())
      {
        std::string Msg = "All " + std::to_string(ToDeleteW.size()) +
          " files in group \"" + Lbl->get_label()  +
          "\" are selected. Are you sure you want to delete them ALL?";
        int Result = MessageBox("Delete ALL",
          Msg,
          Gtk::BUTTONS_YES_NO
        );
        Delete = Result == Gtk::RESPONSE_YES;
        if (Result == Gtk::RESPONSE_DELETE_EVENT)
          break;
      }

      if (Delete)
      {
        for (Gtk::CheckButton* B: ToDeleteW)
        {
          if (unlink(B->get_label().c_str()) != 0)
          {
            Errors++;
            Error("Failed to delete: " + B->get_label());
          }
          else
          {
            Duplicates.remove(*B);
            delete B;
          }
        }
      }

      FilesInTheGroup = 0;
      ToDeleteW.clear();
      continue;
    }
    if (Btn)
    {
      FilesInTheGroup++;
      if (Btn->get_active())
        ToDeleteW.push_back(Btn);
      continue;
    }
    // TODO: Should never reach here
  }

  if (Errors > 0)
  {
    MessageBox(
      "Deletion errors",
      "Failed to delete " + std::to_string(Errors)  +
         " files. Check the console for more information.",
       Gtk::BUTTONS_OK);
  }

  CleanupEmptyGroups();
  UpdateStatus();

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnScanHandler(GdkEventButton* aBtn)
{
  // If there is no source folder selected give it a chance to select one
  if (SourceFolders.size() < 1)
    BtnAddSourceFolderHandler(aBtn);
  // If the list is still empty just return instead of clearing the result list
  if (SourceFolders.size() < 1)
    return true;

  DoScan();

  return true;
}
//------------------------------------------------------------------------------

void MainWindow::DoScan()
{
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Duplicates.remove(*W);
    delete W;
  }

  FileSizes.clear();

  UpdateButtons(false, true);

  ScannerImpl Scanner(this);
  for (std::string S: FoldersToIgnore)
    Scanner.IgnoreFolder(S);

  Scanner.Run(SourceFolders);

  // Remap to sort by size
  std::map<ulong,std::vector<ulong>>SortBySize;
  for (ulong GroupID: Scanner.GetGroups())
    SortBySize[Scanner.GetSizes(GroupID)].push_back(GroupID);

  TotalLost = 0;
  TotalFiles = 0;
  TotalGroups = 0;

  // Index in the grid
  uint Index = 0;

  std::map<ulong,std::vector<ulong>>::reverse_iterator IT;
  for (IT = SortBySize.rbegin(); IT != SortBySize.rend(); IT++)
  {
    if (TotalGroups > 2500)
      break;

    SetStatus("Publishing the records of group " + ToHRFormat(TotalGroups));
    for (ulong GroupID: IT->second)
    {
      TotalGroups++;

      ulong Sz = Scanner.GetSizes(GroupID);
      Gtk::Label* Lbl = Gtk::manage(new Gtk::Label("  " + ToHRFormat(Sz)));
      Lbl->set_xalign(0.0);
      Duplicates.attach(*Lbl, 0, Index++);

      bool FirstInGroup = true;
      std::map<std::string, Gtk::CheckButton*> GroupSorted;
      for (FileHandler* F: Scanner.GetFiles(GroupID))
      {
        TotalFiles++;
        if (FirstInGroup)
          TotalLost += Sz;
        FirstInGroup = false;

        Gtk::CheckButton *Btn = Gtk::manage(new Gtk::CheckButton(F->GetFileName()));
        Btn->signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::HandleToggle));
        // Add the Button to the set so it gets sorted
        GroupSorted[F->GetFileName()] = Btn;

        FileSizes[F->GetFileName()] = Sz;
      }
      // Now add the sorted buttons
      for (auto A: GroupSorted)
        Duplicates.attach(*(A.second), 0, Index++);
    }
  }
  SetStatus("Refreshing the view");
  Duplicates.show_all();

  UpdateButtons(false, false);

  ResultsLabel.set_label("Results");
}
//------------------------------------------------------------------------------

bool MainWindow::BtnAddSourceFolderHandler(GdkEventButton*)
{
  std::string Folder = SelectFolder(ScanFolder);
  if (Folder.length() < 1)
    return true;

  AddSourceFolder(Folder);

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnDelSourceFolderHandler(GdkEventButton*)
{
  for (Gtk::Widget* W: GridSourceFolders.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (B && B->get_active())
      SourceFolders.erase(B->get_label());
  }

  UpdateSourceFoldersView();

  return true;
}
//------------------------------------------------------------------------------

std::string MainWindow::SelectFolder(std::string& aFolder)
{
  Gtk::FileChooserDialog dialog(
    "Please select a folder",
    Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER
  );
  dialog.set_transient_for(*this);

  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("_Open", Gtk::RESPONSE_OK);

  if (aFolder != "")
    dialog.set_filename(aFolder);

  if (dialog.run() != Gtk::RESPONSE_OK)
    return "";

  aFolder = dialog.get_filename();
  return aFolder;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnIgnoreHandler(GdkEventButton*)
{
  std::string Folder = SelectFolder(IgnoreFolder);
  if (Folder.length() > 0)
    AddIgnore(Folder);

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnIgnorePatHandler(GdkEventButton*)
{
  std::string S = InputIgnorePat.get_text();
  if (S.length() > 0)
    AddIgnore(S);

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnDelIgnoreHandler(GdkEventButton*)
{
  for (Gtk::Widget* W: GridIgnores.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (B && B->get_active())
      FoldersToIgnore.erase(B->get_label());
  }
  // Refresh the list
  AddIgnore("");

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnSelectHandler(GdkEventButton*)
{
  std::string S = SelectPattern.get_text();
  if (S.length() < 1)
    return true;

  HandlePattern(S, true);

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnUnselectHandler(GdkEventButton*)
{
  std::string S = SelectPattern.get_text();
  if (S.length() < 1)
    return true;

  HandlePattern(S, false);

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnSelectAllButFirstHandler(GdkEventButton*)
{
  // The GUI elements are in Z order, i.e. last to first, so we have to reverse
  // the logic and "save" the first element, which is actually the last ...
  Gtk::CheckButton* Prev = NULL;
  SuppresStatusUpdate = true;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (!B)
    {
      Prev = NULL;
      continue;
    }
    if (Prev)
      Prev->set_active(true);
    Prev = B;
  }

  SuppresStatusUpdate = false;
  UpdateStatus();

  return true;
}
//------------------------------------------------------------------------------

bool MainWindow::BtnSelectAllButLastHandler(GdkEventButton*)
{
  // The visual elements are in Z order, i.e. last to first !!!
  bool IsLast = true;
  SuppresStatusUpdate = true;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (!B)
    {
      IsLast = true;
      continue;
    }
    if (!IsLast)
      B->set_active(true);
    IsLast = false;
  }
  SuppresStatusUpdate = false;
  UpdateStatus();

  return true;
}
//------------------------------------------------------------------------------

void MainWindow::HandleToggle()
{
  if (!SuppresStatusUpdate)
    UpdateStatus();
}
//------------------------------------------------------------------------------

void MainWindow::UpdateSourceFoldersView()
{
  for (Gtk::Widget* W: GridSourceFolders.get_children())
  {
    GridSourceFolders.remove(*W);
    delete W;
  }

  uint Index = 0;
  for(std::string S: SourceFolders)
  {
    Gtk::CheckButton* Btn = Gtk::manage(new Gtk::CheckButton(S));
    GridSourceFolders.attach(*Btn, 0, Index++);
  }
  GridSourceFolders.show_all();
}
//------------------------------------------------------------------------------

void MainWindow::AddSourceFolder(const std::string& aNewSourceFolder)
{
  if (aNewSourceFolder.length() < 1)
    return;

  // TODO: Check if the new folder is parent/subfolder of another folder
  // Maybe this is not really needed as the scan itself somehow handles it
  SourceFolders.insert(aNewSourceFolder);

  UpdateSourceFoldersView();
}
//------------------------------------------------------------------------------

void MainWindow::AddIgnore(const std::string& aIgnore)
{
  if (aIgnore.length() > 0)
    FoldersToIgnore.insert(aIgnore);

  for (Gtk::Widget* W: GridIgnores.get_children())
  {
    GridIgnores.remove(*W);
    delete W;
  }

  uint Index = 0;
  for(std::string S: FoldersToIgnore)
  {
    Gtk::CheckButton* Btn = Gtk::manage(new Gtk::CheckButton(S));
    GridIgnores.attach(*Btn, 0, Index++);
  }
  GridIgnores.show_all();
}
//------------------------------------------------------------------------------

void MainWindow::HandlePattern(const std::string& aPattern, bool aSet)
{
  SuppresStatusUpdate = true;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (!B)
      continue;
    if (B->get_label().find(aPattern) != std::string::npos)
      B->set_active(aSet);
  }
  SuppresStatusUpdate = false;
  UpdateStatus();
}
//------------------------------------------------------------------------------

void MainWindow::CleanupEmptyGroups()
{
  std::set<Gtk::Widget*>ToDelete;

  // Scan all groups and prepare a list of things to be removed
  std::set<Gtk::CheckButton*>InTheGroup;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    Gtk::Label* L = dynamic_cast<Gtk::Label*>(W);

    if (L)
    {
      if (InTheGroup.size() < 2)
      {
        ToDelete.insert(L);
        for(Gtk::Widget* W2: InTheGroup)
          ToDelete.insert(W2);
      }
      InTheGroup.clear();
      continue;
    }
    if (B)
      InTheGroup.insert(B);
  }

  for (Gtk::Widget* W: ToDelete)
  {
    Duplicates.remove(*W);
    delete W;
  }
}
//------------------------------------------------------------------------------

void MainWindow::UpdateStatus()
{
  // First collect the number of selected items
  uint Selected = 0;
  ulong SelectedSize = 0;
  for (Gtk::Widget* W: Duplicates.get_children())
  {
    Gtk::CheckButton* B = dynamic_cast<Gtk::CheckButton*>(W);
    if (B && B->get_active())
    {
      Selected++;
      SelectedSize += FileSizes[B->get_label()];
    }
  }

  FooterText.set_text(
    "Duplicate groups: " + ToHRFormat(TotalGroups) + ", " +
    "Duplicate files : " + ToHRFormat(TotalFiles) + ", " +
    "Total bytes lost: " + ToHRFormat(TotalLost) + ", " +
    "Selected: " + ToHRFormat(Selected) + ", " +
    "Selected size: " + ToHRFormat(SelectedSize)
  );
}
//------------------------------------------------------------------------------

void MainWindow::UpdateButtons(bool aInitial, bool aScanning)
{
  BtnScan.set_sensitive(!aScanning);
  BtnDelete.set_sensitive(!aInitial && !aScanning);
  BtnSelectPattern.set_sensitive(!aInitial && !aScanning);
  BtnUnselectPattern.set_sensitive(!aInitial && !aScanning);
  BtnSelectAllButFirst.set_sensitive(!aInitial && !aScanning);
  BtnSelectAllButLast.set_sensitive(!aInitial && !aScanning);

  BtnAddIgnore.set_sensitive(!aScanning);
  BtnDelIgnore.set_sensitive(!aScanning);
  BtnAddIgnorePat.set_sensitive(!aScanning);

  UpdateStatus();
}
//------------------------------------------------------------------------------

void MainWindow::SetStatus(const std::string& aNewStatus)
{
  FooterText.set_text(aNewStatus);
  while (Gtk::Main::events_pending()) Gtk::Main::iteration(false);
}
//------------------------------------------------------------------------------

std::string MainWindow::ToHRFormat(ulong aSize)
{
  std::string Res = std::to_string(aSize);

  int Pos = Res.length();
  while (Pos > 3)
  {
    Res.insert(Pos - 3, ",");
    Pos -= 3;
  }
  return Res;
}
//------------------------------------------------------------------------------



//==============================================================================
void ScannerImpl::ProcessingUpdate()
{
  Owner->SetStatus(GetState());
}
//------------------------------------------------------------------------------
