//   This file is part of the screenshot-util program, licensed under the terms of the MIT License.
//
//   The MIT License
//   Copyright (C) 2010-2014  The screenshot-util team (See AUTHORS file)
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files (the "Software"), to deal
//   in the Software without restriction, including without limitation the rights
//   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   copies of the Software, and to permit persons to whom the Software is
//   furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE SOFTWARE.

#include <windows.h>

#include "util.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")
/*
SYNOPSIS:
screenshot[-wt WINDOW_TITLE | -rc LEFT TOP RIGHT BOTTOM | -mn MONITOR_VALUE |-o FILENAME | -h]

OPTIONS :
-wt     WINDOW_TITLE                Select window with this title.

-rc     LEFT TOP RIGHT BOTTOM       Crop source.If no WINDOW_TITLE is provided
(0, 0) is left top corner of desktop,
else if WINDOW_TITLE maches a desktop window
(0, 0) is it's top left corner.

-mn     MONITOR VALUE               Specify what monitors the program should take a
screeenshot of. Separate values through commas
[ex. 1,2]. This option is ignored if WINDOW_TITLE
is provided.

-o FILENAME                         Output file name, if none, the image will be saved
as "screenshot.png" in the current working directory.
-h
Shows this help info.

*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  std::wstring helpString = L"\tscreenshot-util -\tSave a screenshot of the Windows desktop\n\t\t\tor window in .png format.\n\n" \
    L"SYNOPSIS:\n"\
    L"\tscreenshot [ -wt WINDOW_TITLE |\n\t\t    -rc LEFT TOP RIGHT BOTTOM |\n\t\t    -wn MONITOR_VALUE|\n\t\t    -o  FILENAME |\n\t\t    -h help]\n\n"\
    L"OPTIONS:\n"\
    L"\t-wt WINDOW_TITLE\n"\
    L"\t\t\tSelect window with this title.\n\t\t\t\n"\
    L"\t-rc LEFT TOP RIGHT BOTTOM\n"\
    L"\t\t\tCrop source. If no WINDOW_TITLE is provided\n"\
    L"\t\t\t(0,0) is left top corner of desktop,\n"\
    L"\t\t\telse if WINDOW_TITLE maches a desktop window\n"\
    L"\t\t\t(0,0) is it's top left corner.\n"\
    L"\t-mn MONITOR_VALUE\n"\
    L"\t\t\tSpecify what monitors the program should take a \n"
    L"\t\t\tscreeenshot of.Separate values through commas \n"
    L"\t\t\t[ex. 1, 2].This option is ignored if WINDOW_TITLE \n"
    L"\t\t\tis provided. Monitor values coincide with \n"
    L"\t\t\tthose found @ Control Panel\\All Control Panel\n" \
    L"\t\t\tItems\\Display\\Screen Resolution \n"
    L"\t-o FILENAME\n"\
    L"\t\t\tOutput file name, if none, the image will be saved\n"\
    L"\t\t\tas \"screenshot.png\" in the current working \n"
    L"\t\t\tdirectory.\n"\
    L"\t-h\n"\
    L"\t\t\tShows this help info.\n";

  // err 2  - window not found by title 
  // err 4  - internal error 
  // err 8  - can't write to file
  // err 12 - problem parsing arguments 
  short error = 0;
  unsigned errorReportingLevel = 0;
  bool monitorsSpecified = false;
  bool windowNotFound = false;
  bool separateScreenShots = false;
  std::vector<int>monitorsToDisplay;
  std::vector<std::wstring> arguments;
  // parse command line arguments 
  error = split(lpCmdLine, ' ', arguments);

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
      windowSearched = FindWindowW(NULL, arguments[i + 1].c_str());
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
      parseMonitorsToDisplay(monitorsToDisplay, arguments[i + 1]);
      monitorsSpecified = true;
    }
    //error reporting level 
    else if (wcscmp(arguments[i].c_str(), L"-e") == 0 && i + 1 < arguments.size())
    {
      errorReportingLevel = _wtoi(arguments[i + 1].c_str());
    }
    //help
    else if (wcscmp(arguments[i].c_str(), L"-h") == 0)
    {
      MessageBox(NULL, helpString.c_str(), L"Help for screenshot-util", MB_OK);
    }
    //separate screenshots
    else if (wcscmp(arguments[i].c_str(), L"-split") == 0)
    {
      separateScreenShots = true;
    }
  }
  if (windowNotFound && errorReportingLevel > 0)
  {
    error = 2;
    return error;
  }
  if (error  && errorReportingLevel > 0)
  {
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
      if (separateScreenShots)
      {
        createScreenShotForEachDesktop(filename, monitorsToDisplay);
      }
      else
      {
        getSomeDesktopsScreenshot(filename, monitorsToDisplay);
      }
    }
    else
    {
      if (separateScreenShots)
      {
        createScreenShotForEachDesktop(filename, monitorsToDisplay);
      }
      else
      {
        getAllDesktopsScreenshot(filename);

      }
      
    }

  }
  return error;
}
