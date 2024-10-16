#include <wx/wx.h>
#include <string>
#include <fstream>
#include "LoginScreen.h"

using namespace std;

LoginScreen::LoginScreen(const wxString& title) : wxFrame(nullptr, wxID_ANY, title
    , wxDefaultPosition, wxDefaultSize)
{
  //VARIABLES DEFINITION
  logoFont = wxFont(wxFontInfo(34).Bold());
  fieldFont = wxFont(wxFontInfo(18));
  errorFont = wxFont(wxFontInfo(16).Bold());
  COMPANY_NAME = "Al Ahmady Tech";
  BRANCH_ID_MESSAGE = "Enter Branch ID:";
  USER_NAME_MESSAGE = "Enter Username:";
  PASSWORD_MESSAGE = "Enter Password:";
  CHECK_BOX_MESSAGE = "Stay Logged In?";
  debugColors = false;

  //MASTER SIZER:
  masterSizer = new wxBoxSizer(wxVERTICAL);

  //COMPANY_NAME label
  companyNameMessage = new wxStaticText(this, wxID_ANY, COMPANY_NAME, wxDefaultPosition,
      wxDefaultSize, wxALIGN_CENTER);
  companyNameMessage->SetFont(logoFont);
  masterSizer->Add(companyNameMessage, 0, wxEXPAND | wxBOTTOM, 20);
  if(debugColors)
    companyNameMessage->SetBackgroundColour(*wxRED);

  //BRANCH_ID_MESSAGE label
  branchSelectMessage = new wxStaticText(this, wxID_ANY, BRANCH_ID_MESSAGE,
      wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  branchSelectMessage->SetFont(fieldFont);
  masterSizer->Add(branchSelectMessage, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    branchSelectMessage->SetBackgroundColour(*wxBLUE);
  
  //BRANCH_ID_MESSAGE input
  branchEnterField = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  branchEnterField->SetFont(fieldFont);
  masterSizer->Add(branchEnterField, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    branchEnterField->SetBackgroundColour(*wxRED);

  //USER_NAME_MESSAGE label
  userSelectMessage = new wxStaticText(this, wxID_ANY, USER_NAME_MESSAGE
      , wxDefaultPosition, wxDefaultSize);
  userSelectMessage->SetFont(fieldFont);
  masterSizer->Add(userSelectMessage, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    userSelectMessage->SetBackgroundColour(*wxBLUE);
  this->SetSizerAndFit(masterSizer);

  //USER_NAME_MESSAGE input
  userEnterField = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
  userEnterField->SetFont(fieldFont);
  masterSizer->Add(userEnterField, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    userEnterField->SetBackgroundColour(*wxRED);

  //PASSWORD_MESSAGE label
  passwordEnterMessage = new wxStaticText(this, wxID_ANY, PASSWORD_MESSAGE, 
      wxDefaultPosition, wxDefaultSize);
  passwordEnterMessage->SetFont(fieldFont);
  masterSizer->Add(passwordEnterMessage, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    passwordEnterMessage->SetBackgroundColour(*wxBLUE);

  //PASSWORD_MESSAGE input 
  passwordEnterField = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
  passwordEnterField->SetFont(fieldFont);
  masterSizer->Add(passwordEnterField, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    passwordEnterField->SetBackgroundColour(*wxRED);

  //Stay logged in box
  stayLoggedIn = new wxCheckBox(this, wxID_ANY, CHECK_BOX_MESSAGE, 
      wxDefaultPosition, wxDefaultSize);
  stayLoggedIn->SetFont(fieldFont);
  masterSizer->Add(stayLoggedIn, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
  if(debugColors)
    stayLoggedIn->SetBackgroundColour(*wxBLUE);

  //loginButton section
  loginButton = new wxButton(this, wxID_ANY, "Log In", wxDefaultPosition, wxDefaultSize);
  loginButton->SetFont(fieldFont);
  masterSizer->Add(loginButton, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    loginButton->SetBackgroundColour(*wxRED);

  //Error message box
  errorMessage = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
  errorMessage->SetFont(errorFont);
  errorMessage->SetForegroundColour(*wxRED);
  masterSizer->Add(errorMessage, 0, wxEXPAND | wxALL, 5);
  if(debugColors)
    errorMessage->SetBackgroundColour(*wxGREEN);

  //BINDING EVENT HANDLERS 
  BindEventHandlers();
}

void LoginScreen::BindEventHandlers()
{
  loginButton->Bind(wxEVT_BUTTON, &LoginScreen::Login, this);
  this->Bind(wxEVT_CLOSE_WINDOW, &LoginScreen::ConfirmClose, this);
}

void LoginScreen::DisplayBranchIDError()
{
 errorMessage->SetLabel("The entered branch ID does not exist!"); 
}

void LoginScreen::DisplayUsernameError()
{
  errorMessage->SetLabel("The entered username does not exist!");
}

void LoginScreen::DisplayPasswordError()
{
  errorMessage->SetLabel("Incorrect Password");
}

void LoginScreen::DisplayUsernamePasswordError()
{
  errorMessage->SetLabel("Username or password is incorrect");
}

void LoginScreen::ConfirmClose(wxCloseEvent& event)
{
  //If it was user initiated (means CanVeto is true)
  std::ofstream logFile("hambola.txt");
  logFile << event.CanVeto() << "\n";
  if(event.CanVeto() == true)
  {
    wxMessageDialog dialog(this, "Are you sure you want to quit?", "Quit?", wxYES_NO
    | wxNO_DEFAULT | wxICON_QUESTION);
    int result = dialog.ShowModal();
    if(result == wxID_YES)
    {
      //event.Skip();
    }
    else
      event.Veto();
  }
  //its a program initiated close
  //else
    //event.Skip();
}

void LoginScreen::Login(wxCommandEvent& event)
{
  //PLACEHOLDER:
  //HAZEM ADD YOUR CODE HERE FOR LOGIN VERIFICATION
  //THIS BOOL IS FOR REMEMBERING IF THE USER WILL REMAIN LOGGED IN OR NOT
  bool rememberMe = stayLoggedIn->IsChecked();
  string BRANCH_ID_INPUT = branchEnterField->GetValue().ToStdString();
  string USERNAME_INPUT = userEnterField->GetValue().ToStdString();
  string PASSWORD_INPUT = passwordEnterField->GetValue().ToStdString();
  if(BRANCH_ID_INPUT != "AHM")
  {
    DisplayBranchIDError();
    return;
  }
  else if(USERNAME_INPUT != "Aly")
  {
    DisplayUsernameError();
    return;
  }
  else if(PASSWORD_INPUT != "1234")
  {
    DisplayPasswordError();
    return;
  }
  else
  {
    //Succesfully logged in
    this->Close(true);
  }
}



