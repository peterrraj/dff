#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__
//------------------------------------------------------------------------------

#include <gtkmm.h>
//------------------------------------------------------------------------------

class MainWindow : public Gtk::Window
{
  private:
    Gtk::VBox ViewPort;

    Gtk::HBox FoldersArea;
    Gtk::VBox IgnoresArea;
    Gtk::Label HdrLabel;
    Gtk::HBox Hdr1Row;
    Gtk::HBox Hdr1Left;
    Gtk::VBox Hdr1Right;
    Gtk::Frame IgnoreFrame;
    Gtk::ScrolledWindow IgnoreListScroller;
    Gtk::Label EmptyLine;
    Gtk::Button BtnDelete;
    Gtk::Button BtnLink;
    Gtk::Button BtnAddIgnore;
    Gtk::Entry  InputIgnorePat;
    Gtk::Button BtnAddIgnorePat;
    Gtk::Button BtnDelIgnore;
    Gtk::Entry  SelectPattern;
    Gtk::Button BtnSelectPattern;
    Gtk::Button BtnUnselectPattern;
    Gtk::Button BtnSelectAllButFirst;
    Gtk::Button BtnSelectAllButLast;
    Gtk::Grid GridIgnores;
    Gtk::VBox SourceFoldersArea;
    Gtk::Label SourceFoldersLabel;
    Gtk::HBox SourceFoldersBox;
    Gtk::Frame SourceFoldersFrame;
    Gtk::ScrolledWindow SourceFoldersScroller;
    Gtk::Grid GridSourceFolders;
    Gtk::VBox SourceFoldersBtnBox;
    Gtk::Button BtnAddSourceFolder;
    Gtk::Button BtnDelSourceFolder;
    Gtk::Button BtnScan;

    Gtk::VBox ResultsArea;
    Gtk::Label ResultsLabel;
    Gtk::HBox ResultsBox;
    Gtk::HBox Res1Row;
    Gtk::HBox Res1Left;
    Gtk::VBox Res1Right;
    Gtk::Frame ResultsFrame;
    Gtk::ScrolledWindow ResultsBoxScroller;
    Gtk::Grid Duplicates;

    Gtk::VBox StatusArea;
    Gtk::ScrolledWindow Footer2Scroller;
    Gtk::Label FooterText;


    bool BtnScanHandler(GdkEventButton*);
    bool BtnAddSourceFolderHandler(GdkEventButton*);
    bool BtnDelSourceFolderHandler(GdkEventButton*);
    bool BtnDeleteHandler(GdkEventButton*);
    bool BtnLinkHandler(GdkEventButton*);
    bool BtnIgnoreHandler(GdkEventButton*);
    bool BtnIgnorePatHandler(GdkEventButton*);
    bool BtnDelIgnoreHandler(GdkEventButton*);
    bool BtnSelectHandler(GdkEventButton*);
    bool BtnUnselectHandler(GdkEventButton*);
    bool BtnSelectAllButFirstHandler(GdkEventButton*);
    bool BtnSelectAllButLastHandler(GdkEventButton*);

    void HandleToggle();

    std::string ScanFolder;
    std::string IgnoreFolder;
    std::string SelectFolder(std::string& aFolder);

    std::set<std::string> SourceFolders;
    std::set<std::string> FoldersToIgnore;

    void UpdateSourceFoldersView();
    void AddSourceFolder(const std::string& aNewSourceFolder);

    void AddIgnore(const std::string& aIgnore);
    void HandlePattern(const std::string& aPattern, bool aSet);

    void CleanupEmptyGroups();
    void UpdateButtons(bool aInitial, bool aScanning);

    // Suppresses status update when handling mass select/unselect
    bool SuppresStatusUpdate = false;
    void UpdateStatus();

    // Do the actual scanning
    void DoScan();

    // Status line info
    ulong TotalLost = 0;
    ulong TotalFiles = 0;
    ulong TotalGroups = 0;

    // Save the file sizes for the status line
    std::map<std::string,ulong>FileSizes;

  protected:
    // Replace a file with a link
    void SafeLink(const std::string& aFile, const std::string& aLink);

    int MessageBox(
      const std::string& aTitle,
      const std::string& aText,
      Gtk::ButtonsType aButtons
    );

    virtual bool on_delete_event(GdkEventAny* any_event);

  public:
    MainWindow();

    void SetStatus(const std::string& aNewStatus);

    static std::string ToHRFormat(ulong aSize);
};
//------------------------------------------------------------------------------

#endif // __MAIN_WINDOW_H__
