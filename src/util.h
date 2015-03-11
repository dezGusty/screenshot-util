#ifndef UTIL_H_
#define UTIL_H_
#include <windows.h>
#include <vector>
#include <gdiplus.h>

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