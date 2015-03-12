#ifndef UTIL_H_
#define UTIL_H_
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

//format type
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// split string 's' by delimiter 'delim' results are stored in vector elems
// if an argument starts with '"' it finds the next occourence in the argument 
// or the following arguments. Concatenates arguments between the two '"'
void split(const std::string &s, char delim, std::vector<std::wstring> &elems);

// outputs screeshot if window or rect is provided.
void getScreenShotByWindowTitleOrRect(HWND windowSearched, wchar_t* filename, RECT rect, bool rectProvided);

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
typedef std::vector<std::pair<HDC, RECT>> HDCPoolType;
class HDCPool{
  HDCPoolType hdcPool;
public:
  int totalWidth = 0;
  int totalHeight = 0;
  std::vector<int> monitorsToDisplay;
  bool checkMonitor = false;
  HDCPoolType::iterator iterator;
  HDCPool(){}
  ~HDCPool(){}
  void addToPool(HDC hdc, RECT rect,int Height, int Width);
  void iterateThroughDesktops();
  void spliceImages(HDC &capture, HBITMAP & bmp, HGDIOBJ & originalBmp, int * width, int * height);
};
void createScreenShot(std::vector<unsigned char>& data, std::vector<int> monitorsToDisplay);
#endif 