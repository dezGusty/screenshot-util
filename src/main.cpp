#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <array>

#include <gdiplus.h>
#include <stdio.h> 
#include "util.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;  // Failure

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

void split(const std::string &s, char delim, std::vector<std::wstring> &elems) {
  std::stringstream ss(s);
  std::vector<std::string> tmpVec;
  std::string item;
  bool b = false;
  while (std::getline(ss, item, delim)) {
    tmpVec.push_back(item);
  }
  for (int i = 0; i < tmpVec.size(); i++)
  {
    if (b){
      b = false;
      continue;
    }
    char c = tmpVec[i].at(0);
    if (c == '\"' && i+1 < tmpVec.size())
    {
      tmpVec[i] = tmpVec[i].substr(1) +" "+ tmpVec[i+1].substr(0, tmpVec[i + 1].size() - 1);
      b = true;
    }
    std::wstring tmp(tmpVec[i].begin(), tmpVec[i].end());
    elems.push_back(tmp);
  }
 // 
}

void getScreenShotByWindowTitle(HWND windowSearched, wchar_t* filename, RECT rect)
{
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  HWND desktop = GetDesktopWindow();
  HDC desktopdc = GetDC(desktop);
  HDC mydc = CreateCompatibleDC(desktopdc);

  int width = (rect.right - rect.left == 0) ? GetSystemMetrics(SM_CXSCREEN) : rect.right - rect.left;
  int height = (rect.bottom - rect.top == 0) ? GetSystemMetrics(SM_CYSCREEN) : rect.bottom - rect.top;

  HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
  HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);
  BitBlt(mydc, 0, 0, width, height, desktopdc, rect.left, rect.top, SRCCOPY | CAPTUREBLT);
  SelectObject(mydc, oldbmp);

  if (windowSearched) SetWindowPos(windowSearched, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  Bitmap* b = Bitmap::FromHBITMAP(mybmp, NULL);
  CLSID  encoderClsid;
  Status stat = GenericError;
  if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
    stat = b->Save(filename, &encoderClsid, NULL);
  }
  if (b)
    delete b;

  // cleanup
  GdiplusShutdown(gdiplusToken);
  ReleaseDC(desktop, desktopdc);
  DeleteObject(mybmp);
  DeleteDC(mydc);
}

// for all desktops 

void SaveVectorToFile(const wchar_t* fileName, const std::vector<unsigned char>& data)
{
  HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    throw std::logic_error("SaveVectorToFile : can't open file ");
  DWORD bytesWriten = 0;
  if (!WriteFile(hFile, &data[0], (DWORD)data.size(), &bytesWriten, 0))
    throw std::logic_error("SaveVectorToFile : can't write to file ");
}

void getAllDesktopsScreenshot(wchar_t* filename){
  std::vector<int> placeholder;
  std::vector<unsigned char> data;
  createScreenShot(data, placeholder);
  SaveVectorToFile(filename, data); // saves in bmp format, not png for now

}

void parseMonitorsToDisplay(std::vector<int>& monitorsToDisplay, std::wstring& monitorArg)
{
  std::string tmpstr(monitorArg.begin(), monitorArg.end());
  std::stringstream ss(tmpstr);
  std::string item;
  bool b = false;
  while (std::getline(ss, item, ',')) {
    monitorsToDisplay.push_back(std::stoi(item));
  }
}
void getSomeDesktopsScreenshot(const wchar_t* filename, const std::vector<int>& monitorsToDisplay){
  std::vector<unsigned char> data;
  createScreenShot(data, monitorsToDisplay);
  SaveVectorToFile(filename, data);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  bool monitorsSpecified = false;
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
  }
  // If windowSearched and rectangle was provided we should recalculate rectangle to the windowSearched coordinates 
  if (windowSearched && rectProvided)
  {
    RECT wrect;
    GetWindowRect(windowSearched, &wrect);
    OffsetRect(&rect, wrect.left, wrect.top);
  }
  // if filename is not set  => default value
  if (wcslen(filename) == 0) wcscpy_s(filename, L"screenshot.png");
  // found window by title 
  if (windowSearched || rectProvided)
  {
    getScreenShotByWindowTitle(windowSearched, filename, rect);
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
