#include "util.h"
#include <algorithm>


void HDCPool::spliceImages(HDC &capture
  , HBITMAP & bmp
  , HGDIOBJ & originalBmp
  , int * width
  , int * height)
{
  HDC hDesktopDC = GetDC(NULL);
 
  unsigned int nScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  unsigned int nScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  *width = nScreenWidth;
  *height = nScreenHeight;

  HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
  if (!hCaptureDC)
  {
    throw std::runtime_error("SpliceImages: CreateCompatibleDC failed");
  }
  capture = hCaptureDC;

  HBITMAP hCaptureBmp = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
  if (!hCaptureBmp)
  {
    throw std::runtime_error("SpliceImages: CreateCompatibleBitmap failed");
  }
  bmp =hCaptureBmp;

  originalBmp = SelectObject(hCaptureDC, hCaptureBmp);
  if (!originalBmp || (originalBmp == (HBITMAP)GDI_ERROR))
  {
    throw std::runtime_error("SpliceImages: SelectObject failed");
  }

  // Calculating coordinates shift if any monitor has negative coordinates
  long shiftLeft = 0;
  long shiftTop = 0;
  for (HDCPoolType::iterator it = hdcPool.begin(); it != hdcPool.end(); ++it)
  {
    if (it->second.left < shiftLeft)
      shiftLeft = it->second.left;
    if (it->second.top < shiftTop)
      shiftTop = it->second.top;
  }

  for (HDCPoolType::iterator it = hdcPool.begin(); it != hdcPool.end(); ++it)
  {
    if (!BitBlt(hCaptureDC, it->second.left - shiftLeft, it->second.top - shiftTop, it->second.right - it->second.left, it->second.bottom - it->second.top, it->first, 0, 0, SRCCOPY))
    {
      throw std::runtime_error("SpliceImages: BitBlt failed");
    }
  }
}
void CaptureDesktop(HDC desktop   // handle to monitor DC
  , HDC &capture                  // handle to destination DC
  , HBITMAP & bmp                 // handle to BITMAP
  , HGDIOBJ & originalBmp         // handle to GDIOBJ
  , int * width
  , int * height
  , int left
  , int top)
{
  unsigned int nScreenWidth = GetDeviceCaps(desktop, HORZRES);
  unsigned int nScreenHeight = GetDeviceCaps(desktop, VERTRES);

  *height = nScreenHeight;
  *width = nScreenWidth;
  HDC hCaptureDC = CreateCompatibleDC(desktop);

  if (capture)
    CloseHandle(capture);
  capture = hCaptureDC;

  HBITMAP hCaptureBmp = CreateCompatibleBitmap(desktop, *width, *height);

  if (bmp)
    DeleteObject(bmp);
  bmp = hCaptureBmp;
  
  // Selecting an object for the specified DC
  originalBmp = SelectObject(hCaptureDC, hCaptureBmp);

  // Selecting an object for the specified DC
  originalBmp = SelectObject(hCaptureDC, hCaptureBmp);

  BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight, desktop, left, top, SRCCOPY | CAPTUREBLT);
}

BOOL CALLBACK MonitorEnumProc(
  HMONITOR hMonitor,    // handle to display monitor
  HDC hdcMonitor,       // handle to monitor DC
  LPRECT lprcMonitor,   // monitor intersection rectangle
  LPARAM dwData         // data
  )
{
  static int desktopNum = 0;
  desktopNum++;
  
  HBITMAP bmp;
  HGDIOBJ originalBmp = NULL;
  int height = 0;
  int width = 0;
  HDC desktop(hdcMonitor);
  HDC capture(0);

  HDCPool * hdcPool = reinterpret_cast<HDCPool *>(dwData);
  if (hdcPool->checkMonitor)
  {
    auto result = std::find(std::begin(hdcPool->monitorsToDisplay), std::end(hdcPool->monitorsToDisplay), desktopNum);
    if (result != std::end(hdcPool->monitorsToDisplay))
    {
      CaptureDesktop(desktop, capture, bmp,
        originalBmp, &width, &height, lprcMonitor->left, lprcMonitor->top);

      RECT rect = *lprcMonitor;
      hdcPool->addToPool(capture, rect, height, width);
    }
  }
  else
  {
    CaptureDesktop(desktop, capture, bmp,
      originalBmp, &width, &height, lprcMonitor->left, lprcMonitor->top);

    RECT rect = *lprcMonitor;
    hdcPool->addToPool(capture, rect,height, width);
  }
  
  return true;
}

void HDCPool::addToPool(HDC hdc, RECT rect,int Height, int Width)
{
  hdcPool.push_back(std::pair<HDC, RECT>(hdc, rect));
  totalWidth += Width;
  if (Height > totalHeight){
    totalHeight = Height;
  }
  
}
void HDCPool::iterateThroughDesktops()
{
  HDC hDesktopDC = GetDC(NULL);
  EnumDisplayMonitors(hDesktopDC, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
}

void createBitmapFinal(std::vector<unsigned char> & data, HDC &capture, HBITMAP & bmp, HGDIOBJ & originalBmp, int nScreenWidth, int nScreenHeight)
{
  // save data to buffer
  unsigned char charBitmapInfo[sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD)] = { 0 };
  LPBITMAPINFO lpbi = (LPBITMAPINFO)charBitmapInfo;
  lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  lpbi->bmiHeader.biHeight = nScreenHeight;
  lpbi->bmiHeader.biWidth = nScreenWidth;
  lpbi->bmiHeader.biPlanes = 1;
  lpbi->bmiHeader.biBitCount = 32;
  lpbi->bmiHeader.biCompression = BI_RGB;

  SelectObject(capture, originalBmp);

  if (!GetDIBits(capture, bmp, 0, nScreenHeight, NULL, lpbi, DIB_RGB_COLORS))
  {
    int err = GetLastError();
    throw std::runtime_error("CreateBitmapFinal: GetDIBits failed");
  }
  DWORD ImageSize; 
  try
  {
    ImageSize = lpbi->bmiHeader.biSizeImage; //known image size
  }
  catch (std::exception& e){
    e.what();
  }
 

  DWORD PalEntries = 3;
  if (lpbi->bmiHeader.biCompression != BI_BITFIELDS)
    PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ? (int)(1 << lpbi->bmiHeader.biBitCount) : 0;
  if (lpbi->bmiHeader.biClrUsed)
    PalEntries = lpbi->bmiHeader.biClrUsed;
  //known pal entrys count

  //all resize
  data.resize(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+PalEntries * sizeof(RGBQUAD)+ImageSize);
  //set screenshot size

  DWORD imageOffset = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+PalEntries * sizeof(RGBQUAD);
  DWORD infoHeaderOffset = sizeof(BITMAPFILEHEADER);
  BITMAPFILEHEADER * pFileHeader = (BITMAPFILEHEADER *)&data[0];
  pFileHeader->bfType = 19778; // always the same, 'BM'
  pFileHeader->bfReserved1 = pFileHeader->bfReserved2 = 0;
  pFileHeader->bfOffBits = imageOffset;
  pFileHeader->bfSize = ImageSize;

  if (!GetDIBits(capture, bmp, 0, nScreenHeight, &data[imageOffset], lpbi, DIB_RGB_COLORS))
  {
    throw std::runtime_error("CreateBitmapFinal: GetDIBits failed");
  }

  memcpy(&data[sizeof(BITMAPFILEHEADER)], &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));

}
void createScreenShot(std::vector<unsigned char>& data, std::vector<int> monitorsToDisplay){
  HDC capture;
  HBITMAP bmp;
  HGDIOBJ originalBmp = NULL;
  int height = 0;
  int width = 0;

  HDCPool hdcPool;
  if (monitorsToDisplay.size()!=0){
    hdcPool.monitorsToDisplay = monitorsToDisplay;
    hdcPool.checkMonitor = true;
  }
  hdcPool.iterateThroughDesktops();
  hdcPool.spliceImages(capture, bmp, originalBmp, &width, &height);
  createBitmapFinal(data, capture, bmp, originalBmp, hdcPool.totalWidth, hdcPool.totalHeight);
}