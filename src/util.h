#ifndef UTIL_H_
#define UTIL_H_

//   This file is part of the screenshot-util program, licensed under the terms
//   of the MIT License.
//
//   The MIT License
//   Copyright (C) 2014-2015  The screenshot-util team (See AUTHORS file)
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
#include <vector>
#include <string>
#include <sstream>
#include <array>

#include <stdio.h> 
#include <gdiplus.h>


// iterates through availible data. 
// Screenshots of monitors are taken depending on the values stored in monitorsToDisplay.
// final screenshot width = total width of monitor screeenshots
// final screenshot height = height of the tallest screenshot
void createScreenShot(std::vector<unsigned char>& data, std::vector<int> monitorsToDisplay);

void createScreenShotForEachDesktop(const wchar_t* filename, const std::vector<int>& monitorsToDisplay);


//format type
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// split string 's' by delimiter 'delim' results are stored in vector elems
// if an argument starts with '"' it finds the next occourence in the argument 
// or the following arguments. Concatenates arguments between the two '"'
int split(const std::string &s, char delim, std::vector<std::wstring> &elems);

// outputs screeshot if window or rect is provided.
int getScreenShotByWindowTitleOrRect(HWND windowSearched, wchar_t* filename, RECT rect, bool rectProvided);

// saves the data contained in the vector in BMP
// BMP saved to file in a PNG format. 
void SaveVectorToFile(const wchar_t* fileName, const std::vector<unsigned char>& data);

// generates a screenshot of all possible monitors
void getAllDesktopsScreenshot(wchar_t* filename);

// generates a screenshot of the monitors found specified in 'monitorsToDisplay'
// value for monitors is the same as that found in Control PaneDisplay\Screen Resolution
void getSomeDesktopsScreenshot(const wchar_t* filename, const std::vector<int>& monitorsToDisplay);

// parses -mt argument. Monitor values are separated by commas. Stores each value in
// monitorsToDisplay
void parseMonitorsToDisplay(std::vector<int>& monitorsToDisplay, std::wstring& monitorArg);

// entry point for creating screenshot of multiple monitors.
// checks if the program should check for selected monitors
void createScreenShot(std::vector<unsigned char>& data, std::vector<int> monitorsToDisplay);

typedef std::vector<std::pair<HDC, RECT>> HDCPoolType;

// model can be found @ http://www.codeproject.com/Articles/101272/Creation-of-Multi-monitor-Screenshots-Using-WinAPI
struct HDCPool{
  int desktopNum = 0;

  HDCPoolType hdcPool;
  
  //total width of the output screenshot. Represents the width of all monitors selected
  int totalWidth = 0;
  // height of the output screenshot. Represents the height of the tallest monitor. 
  int totalHeight = 0;

  // monitors to display
  // value of monitors coincides with those found @ Control Panelo\Display\Screen Resolution
  std::vector<int> monitorsToDisplay;

  // if the program should check for selected monitors
  bool checkMonitor = false;
  HDCPoolType::iterator iterator;
  HDCPool(){}
  ~HDCPool(){}

  // if the monitor is selected adds it's context and rect to the hdcPool
  // Height and Width used to compute final screenshot dimmensions
  void addToPool(HDC hdc, RECT rect, int Height, int Width);
  // iterate through all availible desktops
  void iterateThroughDesktops();

  //splice screenshots of individual monitors together
  void spliceImages(HDC &capture, HBITMAP & bmp, HGDIOBJ & originalBmp, int * width, int * height);

};
#endif 