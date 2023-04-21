#include "mainwindow.h"
//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  // Hide the command line parameters. Not doing so casuses premature app exit
  argc = 1;
  Glib::RefPtr<Gtk::Application> application =
    Gtk::Application::create(argc, argv);
  MainWindow MW;
  return application->run(MW);
}
//------------------------------------------------------------------------------
