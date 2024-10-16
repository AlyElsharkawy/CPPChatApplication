#include <wx/app.h>
#include <wx/wx.h>
#include <fstream>
#include "MainFrame.h"
#include "CLIENT_INTERFACE.h"

class App : public wxApp
{
public:
  bool OnInit();
};

wxIMPLEMENT_APP(App);

//Main App logic goes here
bool App::OnInit()
{
  //Clear outputFile
  ofstream outputFile("update_loop");
  outputFile << "";
  outputFile.close();

  MainFrame* mainFrame = new MainFrame("Chat Client");
  mainFrame->Show();
  //Always return true;
  return true;
}
