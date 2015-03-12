#include <windows.h>

#include "util.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
  unsigned errorReportingLevel = 0;
  // err 2  - window not found by title 
  // err 4  - internal error 
  // err 8  - can't write to file 
  short error; 
  bool monitorsSpecified = false;
  bool windowNotFound = false; 
  std::vector<int>monitorsToDisplay;
  std::vector<std::wstring> arguments; 
  // parse command line arguments 
  split(lpCmdLine, ' ', arguments);
  HWND windowSearched = NULL;
  RECT rect = { 0 };
  wchar_t filename[MAX_PATH] = { 0 };

  bool rectProvided = false;
 
  for (short i = 0; i < arguments.size(); i++)
  {
    // window title ptobided
    if (wcscmp(arguments[i].c_str(), L"-wt") == 0 && i + 1<arguments.size())
    {
      const wchar_t* seaching = arguments[i + 1].c_str();
      windowSearched = FindWindowW(NULL, arguments[i+1].c_str());
      if (windowSearched)
      {
        SetWindowPos(windowSearched, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        Sleep(200); //TODO: Arbitrary waiting time for window to become topmost
        if (!rectProvided) GetWindowRect(windowSearched, &rect);
      }
      else
      {
        windowNotFound = true;
      }
    }
    // rect provided
    else if (wcscmp(arguments[i].c_str(), L"-rt") == 0 && i + 4 < arguments.size())
    {
      rect.left = _wtoi(arguments[i + 1].c_str());
      rect.top = _wtoi(arguments[i + 2].c_str());
      rect.right = _wtoi(arguments[i + 3].c_str());
      rect.bottom = _wtoi(arguments[i + 4].c_str());
      rectProvided = true;
    }
    // output file provided
    else if (wcscmp(arguments[i].c_str(), L"-o") == 0 && i + 1 < arguments.size())
    {
      wcscpy_s(filename, arguments[i + 1].c_str());
    }
    //monitor number to display
    else if (wcscmp(arguments[i].c_str(), L"-mn") == 0 && i + 1 < arguments.size())
    {
      parseMonitorsToDisplay(monitorsToDisplay,arguments[i + 1]);
      monitorsSpecified = true;
    }
    //error reporting level 
    else if (wcscmp(arguments[i].c_str(), L"-e") == 0 && i + 1 < arguments.size())
    {
      errorReportingLevel = _wtoi(arguments[i + 1].c_str());
    }
  }
  if (windowNotFound && errorReportingLevel > 0)
  {
    error = 2;
    return error;
  }
  // if filename is not set  => default value
  if (wcslen(filename) == 0) wcscpy_s(filename, L"screenshot.png");
  // found window by title 
  if (windowSearched || rectProvided)
  {
    getScreenShotByWindowTitleOrRect(windowSearched, filename, rect, rectProvided);
  }
  else
  {
    if (monitorsSpecified)
    {
      getSomeDesktopsScreenshot(filename, monitorsToDisplay);
    }
    else
    {
      getAllDesktopsScreenshot(filename);
    }
  
  }
  return 0;
}
