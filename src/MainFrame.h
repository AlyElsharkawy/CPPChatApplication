#pragma once
#include <string>
#include <wx/event.h>
#include <wx/wx.h>

using namespace std;

class MainFrame : public wxFrame
{
public:
  MainFrame(const wxString& title = "No Title");
  //Essential
  void AddMessageToWindow(const string& name, const string& message);
  void AddMessageToWindow(char* nameInput, char* messageInput);

  void ChangeUsername(char* newUsername);

private:
  //QOL
  void BindEventHandlers();
  void ConfirmClose(wxCloseEvent& event);

  //Event Handlers 
  void SendMessageHandler(wxCommandEvent& event);
};
