#include <thread>
#include <wx/wx.h>
#include <string>
#include <fstream>
#include "CLIENT_INTERFACE.h"
#include "net_message.h"
#include "MainFrame.h"
#include "timer.h"

//NETWORKING STRINGS
wxScrolledWindow* scrolledMessageWindow;
wxBoxSizer* messagesSizer;

//Sizers
wxBoxSizer* masterSizer;
wxBoxSizer* sendMessageSizer;

//Message send section
wxTextCtrl* messageInputCtrl;
wxButton* sendMessageButton;

wxTextCtrl* tempName;
wxTextCtrl* tempMessage;

wxFont stdFont = wxFont(wxFontInfo(14));
wxFont nameFont = wxFont(wxFontInfo(20).Bold());

using namespace std;

CustomClient CLI;
string THIS_USERNAME = "";

string logName;
ofstream finalLog;
mutex mtx;

void MainFrame::BindEventHandlers()
{
  this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::ConfirmClose, this);
  sendMessageButton->Bind(wxEVT_BUTTON, &MainFrame::SendMessageHandler, this);
}

//NOTE: scrolledMessageWindow MUST BE PARENT OF ALL MESSAGES

void MainFrame::AddMessageToWindow(const string& name, const string& message)
{
  tempName = new wxTextCtrl(scrolledMessageWindow, wxID_ANY, name + ':', wxDefaultPosition
                      , wxDefaultSize, wxTE_READONLY | wxBORDER_NONE);
  tempName->SetFont(nameFont);
  tempName->SetBackgroundColour(*wxTransparentColor); 
  tempMessage = new wxTextCtrl(scrolledMessageWindow, wxID_ANY, message, wxDefaultPosition
                                           , wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_NO_VSCROLL | wxBORDER_NONE);
  tempMessage->SetFont(stdFont);
  tempMessage->SetBackgroundColour(*wxTransparentColor);
  messagesSizer->Add(tempName, 1, wxEXPAND | wxBOTTOM, 5);
  messagesSizer->Add(tempMessage, 2, wxEXPAND | wxBOTTOM, 12);
}

void MainFrame::AddMessageToWindow(char* nameInput, char* messageInput)
{
  string name;
  string message;
  while(*nameInput != '\0')
  {
    name += *nameInput;
    nameInput++;
  }
  while(*messageInput != '\0')
  {
    message += *messageInput;
    messageInput++;
  }
  tempName = new wxTextCtrl(scrolledMessageWindow, wxID_ANY, name + ':', wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxBORDER_NONE);
  tempName->SetFont(nameFont);
  tempName->SetBackgroundColour(*wxTransparentColor);
  tempMessage = new wxTextCtrl(scrolledMessageWindow, wxID_ANY, message, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_NO_VSCROLL | wxBORDER_NONE);
  tempMessage->SetFont(stdFont);
  tempMessage->SetBackgroundColour(*wxTransparentColor);
  messagesSizer->Add(tempName, 1, wxEXPAND | wxBOTTOM, 5);
  messagesSizer->Add(tempMessage, 2, wxEXPAND | wxBOTTOM, 12);
}


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title
, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
{
  this->Center();
  this->SetClientSize(400,600);
  //SIZERS DEFINITION
  masterSizer = new wxBoxSizer(wxVERTICAL);
  sendMessageSizer = new wxBoxSizer(wxHORIZONTAL);
  messagesSizer = new wxBoxSizer(wxVERTICAL);

  //SEND MESSAGEES ELEMENTS DEFINITION
  sendMessageButton = new wxButton(this, wxID_ANY, "Send", wxDefaultPosition, wxDefaultSize);
  messageInputCtrl = new wxTextCtrl(this, wxID_ANY, "Type a message...", wxDefaultPosition, wxDefaultSize);
  scrolledMessageWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledMessageWindow->SetSizer(messagesSizer);
  scrolledMessageWindow->SetScrollbars(0,1,1,1);

  //ADDING SENDING MESSAGES ELEMENTS TO THEIR SIZER
  sendMessageSizer->Add(messageInputCtrl, 5);
  sendMessageSizer->Add(sendMessageButton, 1);

  //SIZER APPLICATION
  this->SetSizerAndFit(masterSizer);
  masterSizer->Add(scrolledMessageWindow, 9, wxEXPAND | wxALL, 5);
  masterSizer->Add(sendMessageSizer, 1, wxEXPAND | wxALL, 5);

  //NETWORKING STUFF
  CLI.InitalizeServer();
  while(THIS_USERNAME == "")
  {
    CLI.RequestName();
    THIS_USERNAME = CLI.UsernameListen();
  }

  //Clearing update_loop.txt 
  ofstream clearFile("update_loop.txt");
  clearFile << "";
  clearFile.close();

  BindEventHandlers();
  thread updateLoop = thread([&]()
                             {
                              string namePart;
                              string contentPart;
                             while(true)
                             {
                              ofstream tempFile("update_loop.txt", std::ios::app);
                              string returnedString = CLI.StringListen();
                              bool isFound = false;
                             namePart = ""; contentPart = "";
                              for(int i = 0; i < returnedString.size(); i++)
                              {
                                if(returnedString[i] == ' ')
                                  isFound = true;
                                if(isFound == false)
                                  namePart += returnedString[i];
                                if(isFound == true)
                                  contentPart += returnedString[i];
                              }
                             tempFile << "[CLIENT RECIEVER]: Name Recieved is: " << namePart << endl;
                             tempFile << "[CLIENT RECIEVER]: Content Recieved is: " << contentPart << endl;
                             tempFile.close();
                              this->CallAfter([&](){this->AddMessageToWindow(namePart, contentPart);});
                             }
                             });
                            
  updateLoop.detach();
}

void MainFrame::ConfirmClose(wxCloseEvent& event)
{
  //If it was user initiated (means CanVeto is true)
  if(event.CanVeto() == true)
  {
    wxMessageDialog dialog(this, "Are you sure you want to quit?", "Quit?", wxYES_NO
    | wxNO_DEFAULT | wxICON_QUESTION);
    int result = dialog.ShowModal();
    if(result == wxID_NO)
      event.Veto();
    else
      event.Skip();
  }
}

void MainFrame::ChangeUsername(char* newUsername)
{
  string tempString;
  while(*newUsername != '\0')
  {
    tempString += *newUsername;
    newUsername++;
  }
  THIS_USERNAME = tempString;
  ofstream outputFile("Username.txt");
  outputFile << tempString;
  outputFile.close();
}

void MainFrame::SendMessageHandler(wxCommandEvent& event)
{
    ofstream outputFile("update_loop.txt", ios::app);
    string contentString = messageInputCtrl->GetValue().ToStdString();
    CLI.AddMessage(THIS_USERNAME, contentString);
    this->AddMessageToWindow(THIS_USERNAME, contentString);
    scrolledMessageWindow->Scroll(wxPoint(0,scrolledMessageWindow->GetVirtualSize().y));
    outputFile << "[CLIENT SENDER]: username to send is " << THIS_USERNAME << endl;
    outputFile << "[CLIENT SENDER]: content to send is: " << contentString << endl;
    outputFile.close();
    messageInputCtrl->Clear();
}





