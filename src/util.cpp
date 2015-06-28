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


#include "util.h"

// splice catured hdc 
void HDCPool::spliceImages(HDC &capture
  , HBITMAP & bmp
  , HGDIOBJ & originalBmp
  , int * width
  , int * height)
{
  // retrieves a handle to the device context
  // _IN NULL retrives the DC for the entire screen 
  HDC hDesktopDC = GetDC(NULL);
  
  // retrieve width of the virtual screen
  unsigned int nScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  // retrieve height of the virtual screen 
  unsigned int nScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  *width = nScreenWidth;
  *height = nScreenHeight;

  // retrieve a value to a memory dc
  HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);

  // reset device context 
  if (!hCaptureDC)
  {
    throw std::runtime_error("SpliceImages: CreateCompatibleDC failed");
  }
  capture = hCaptureDC;

  // retrieve compatible bitmap
  HBITMAP hCaptureBmp = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
  if (!hCaptureBmp)
  {
    throw std::runtime_error("SpliceImages: CreateCompatibleBitmap failed");
  }
  bmp =hCaptureBmp;
  // load bitmap into device context
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
  // get screen width
  unsigned int nScreenWidth = GetDeviceCaps(desktop, HORZRES);

  // get screen height
  unsigned int nScreenHeight = GetDeviceCaps(desktop, VERTRES);

  *height = nScreenHeight;
  *width = nScreenWidth;
  
  // create compatible device context 
  HDC hCaptureDC = CreateCompatibleDC(desktop);

  //reset handle to device context 
  if (capture)
    CloseHandle(capture);
  capture = hCaptureDC;

  // create bitmap 
  HBITMAP hCaptureBmp = CreateCompatibleBitmap(desktop, *width, *height);

  // reset bmp 
  if (bmp)
    DeleteObject(bmp);
  bmp = hCaptureBmp;
  
  // Selecting an object for the specified DC
  originalBmp = SelectObject(hCaptureDC, hCaptureBmp);

  // bit-block transfer of the color data into the device context 
  BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight, desktop, left, top, SRCCOPY | CAPTUREBLT);
}

// iterate through each desktop, check if desktop should be screenshoted 
// add the context and the rectangle of the desktop to the hdcPool
BOOL CALLBACK MonitorEnumProc(
  HMONITOR hMonitor,    // handle to display monitor
  HDC hdcMonitor,       // handle to monitor DC
  LPRECT lprcMonitor,   // monitor intersection rectangle
  LPARAM dwData         // data
  )
{
  HBITMAP bmp = NULL;
  HGDIOBJ originalBmp = NULL;
  int height = 0;
  int width = 0;

  //get handles
  HDC desktop(hdcMonitor);
  HDC capture(0);

  // data sent between callbacks
  HDCPool * hdcPool = reinterpret_cast<HDCPool *>(dwData);

  // check if program should check which monitors to screenshot 
  if (hdcPool->checkMonitor)
  {
    // check if monitor should be screenshoted 
    auto result = std::find(std::begin(hdcPool->monitorsToDisplay), std::end(hdcPool->monitorsToDisplay), hdcPool->desktopNum++);
    if (result != std::end(hdcPool->monitorsToDisplay))
    {
      //fill bitmap with current desktop 
      CaptureDesktop(desktop, capture, bmp,
        originalBmp, &width, &height, lprcMonitor->left, lprcMonitor->top);
      
      // save monitor coordinates 
      RECT rect = *lprcMonitor;
      hdcPool->addToPool(capture, rect, height, width); // TODO(Vlad): height and width can be extracted from width
                                                        // no need to to pass extra parameters. 
    }
  }
  // 
  else
  {
    //fill bitmap with current desktop 
    CaptureDesktop(desktop, capture, bmp,
      originalBmp, &width, &height, lprcMonitor->left, lprcMonitor->top);

    RECT rect = *lprcMonitor;
    hdcPool->addToPool(capture, rect,height, width);    // TODO(Vlad): height and width can be extracted from rect
                                                        // no need to to pass extra parameters. 
  }
  // increment desktop number 
  hdcPool->desktopNum++;  
  // return true to keep iterating through all valid desktops 
  return true;
}

// save device context handle and coordinates from current desktop 
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
  // get handle to main desktop 
  HDC hDesktopDC = GetDC(NULL);
  
  //start iterating though desktops 
  // @IN HDC   - defines visible region of interset 
  // @IN LPCRECT  - defines clipping rectangle 
  // @IN MONITORENUMPROC  - defines callback function 
  // @IN LPARAM - data sent between callbacks 
  EnumDisplayMonitors(hDesktopDC, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
}

 // @OUT std::vector<unsigned char>& data -- final screenshot data 
void createBitmapFinal(std::vector<unsigned char> & data, HDC &capture, HBITMAP & bmp, HGDIOBJ & originalBmp, int nScreenWidth, int nScreenHeight)
{
  // save data to buffer
  unsigned char charBitmapInfo[sizeof(BITMAPINFOHEADER)+256 * sizeof(RGBQUAD)] = { 0 };

  // load buffer into header defining the dib(device independet bitmap)
  LPBITMAPINFO lpbi = (LPBITMAPINFO)charBitmapInfo;
  lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  lpbi->bmiHeader.biHeight = nScreenHeight;
  lpbi->bmiHeader.biWidth = nScreenWidth;
  lpbi->bmiHeader.biPlanes = 1;
  lpbi->bmiHeader.biBitCount = 32;
  lpbi->bmiHeader.biCompression = BI_RGB;

  //load bitmap into selected device context 
  SelectObject(capture, originalBmp);

  // GetDIBits function retrieves the bits of the specified compatible bitmap 
  // and copies them into a buffer as a DIB using the specified format.
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
// iterate through valid desktops
// splice bitmaps 
// retrieve final vector containing spliced bitmaps
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


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders

  // Integer that specifies the size, in bytes, of the array of ImageCodecInfo objects.
  // Call GetImageEncodersSize to determine this number. 
  UINT  size = 0;         // size of the image encoder array in bytes

  Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

  Gdiplus::GetImageEncodersSize(&num, &size);
  if (size == 0)
  {
    return -1;  // Failure
  }

  pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
  {
    return -1;  // Failure
  }

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j)
  {
    Gdiplus::ImageCodecInfo codecInfo = pImageCodecInfo[j];
    if (wcscmp(codecInfo.MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

//split string on delim 
int split(const std::string &s, char delim, std::vector<std::wstring> &elems) {
  try
  {
    int err = 0;
    std::stringstream ss(s);
    std::vector<std::string> tmpVec;
    std::string item;
    bool b = false;
    // read split arguments into temp vector
    while (std::getline(ss, item, delim)) {
      tmpVec.push_back(item);
    }
    for (unsigned i = 0; i < tmpVec.size(); i++)
    {
      if (b){
        b = false;
        continue;
      }
      // don't add empty entries to final vector
      if (tmpVec[i].size()>0)
      {
        char c = tmpVec[i].at(0);
        //check if the " character is found
        //check if there is another argument
        if (c == '\"' && i + 1 < tmpVec.size())
        {
          // if no end '\"' symbol is found keep iterating until the 
          // end of the temporary vector

          // skip next iteration of enclosing vector
          b = true;
          unsigned k = i;
          for (; k < tmpVec.size(); k++){
            if (i != k)
            {
              tmpVec[i] += (" " + tmpVec[k]);
            }
            // first charater is '\"'
            size_t pos = tmpVec[k].substr(1).find('\"');
            if (pos != std::string::npos)
            {
              tmpVec[i] = tmpVec[i].substr(1, tmpVec[i].size() - 2);
              break;
            }

          }
        }
        // create wstring from string and add it to final vector. 
        std::wstring tmp(tmpVec[i].begin(), tmpVec[i].end());
        elems.push_back(tmp);
      }
    }

  }
  catch (std::exception& e)
  {
    e.what();
    //error reading arguments
    return 12;
  }
  return 0;
}

int getScreenShotByWindowTitleOrRect(HWND windowSearched, wchar_t* filename, RECT rect, bool rectProvided)
{
  try
  {
    // If windowSearched and rectangle was provided we should recalculate rectangle to the windowSearched coordinates 
    if (windowSearched && rectProvided)
    {
      RECT wrect;
      GetWindowRect(windowSearched, &wrect);
      OffsetRect(&rect, wrect.left, wrect.top);
    }
    // instantiate gdi
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // get handles 
    HWND desktop = GetDesktopWindow();
    HDC desktopdc = GetDC(desktop);
    HDC mydc = CreateCompatibleDC(desktopdc);

    // get final width and height
    int width = (rect.right - rect.left == 0) ? GetSystemMetrics(SM_CXSCREEN) : rect.right - rect.left;
    int height = (rect.bottom - rect.top == 0) ? GetSystemMetrics(SM_CYSCREEN) : rect.bottom - rect.top;

    // create compatible bitmap 
    HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);

    // load created bitmap into device context 
    HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);

    // This function transfers pixels from a specified source rectangle to a specified destination rectangle, 
    // altering the pixels according to the selected raster operation (ROP) code.
    BitBlt(mydc, 0, 0, width, height, desktopdc, rect.left, rect.top, SRCCOPY | CAPTUREBLT);

    // load bitmap into device context 
    SelectObject(mydc, oldbmp);

    // retain window position and size and display it 
    if (windowSearched) SetWindowPos(windowSearched, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    //save it to localdisk
    Gdiplus::Bitmap* b = Gdiplus::Bitmap::FromHBITMAP(mybmp, NULL);
    CLSID  encoderClsid;
    Gdiplus::Status stat = Gdiplus::GenericError;
    if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
      stat = b->Save(filename, &encoderClsid, NULL);
    }
    if (b)
      delete b;

    // cleanup
    Gdiplus::GdiplusShutdown(gdiplusToken);
    ReleaseDC(desktop, desktopdc);
    DeleteObject(mybmp);
    DeleteDC(mydc);
  }
  catch (std::exception& e)
  {
    e.what();
    return 8;
  }
  return 0;
}
// for all desktops 

void SaveVectorToFile(const wchar_t* fileName, const std::vector<unsigned char>& data)
{
  // instantiate gdi 
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // gdi has no method to create bitmap from vector, 
  // but it does have one for streams
  Gdiplus::Bitmap* pBitmap = NULL;
  IStream* pStream = NULL;

  HRESULT hResult = ::CreateStreamOnHGlobal(NULL, TRUE, &pStream);
  if (hResult == S_OK && pStream)
  {
    // write from buffer to stream 
    hResult = pStream->Write(&data[0], ULONG(data.size()), NULL);
    if (hResult == S_OK)
    {
      // create bitmap from stream
      pBitmap = Gdiplus::Bitmap::FromStream(pStream, 1);
    }
     
    // write to localdisk 
    CLSID  encoderClsid;
    Gdiplus::Status stat = Gdiplus::GenericError;
    if (pBitmap && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
      stat = pBitmap->Save(fileName, &encoderClsid, NULL);
    }
    //reset bitmap and pStream
    if (pBitmap)
      delete pBitmap;
    pStream->Release();
  }

  //clean up
  Gdiplus::GdiplusShutdown(gdiplusToken);
  
}

void getAllDesktopsScreenshot(wchar_t* filename){
  std::vector<int> placeholder;
  std::vector<unsigned char> data;
  createScreenShot(data, placeholder);
  SaveVectorToFile(filename, data); // saves in bmp format, not png for now

}

//@OUT monitorsToDisplay - vector of monitors to display, duplicates don't matter, the value 
//                         of the monitor just needs to be there
//@IN  monitorArg        - wstring to parse over
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

class CallbackData{
public:
  // what monitors to display
  std::vector<int> monitorsToDisplay;
  // value of monitor 
  int index = 1;
  //filename to save to 
  const wchar_t* filename;
  CallbackData(const wchar_t* fileName, const std::vector<int>& vec)
  {
    filename = fileName;
    monitorsToDisplay = vec;
  }
};

BOOL CALLBACK IterateThroughDesktopsAndPrint(
  HMONITOR hMonitor,    // handle to display monitor
  HDC hdcMonitor,       // handle to monitor DC
  LPRECT lprcMonitor,   // monitor intersection rectangle
  LPARAM dwData         // data
  )
{
  CallbackData* callbackData = reinterpret_cast<CallbackData *>(dwData);
  
  // if there's something in the monitorsToDisplay, it means the program should 
  // try and only screenshot selected desktops 
  if (callbackData->monitorsToDisplay.size() > 0){
    auto result = std::find(std::begin(callbackData->monitorsToDisplay), std::end(callbackData->monitorsToDisplay),callbackData->index);
    if (result != std::end(callbackData->monitorsToDisplay))
    {
      HGDIOBJ originalBmp = NULL;
      int height = 0;
      int width = 0;
      //instantiate gdi 
      Gdiplus::GdiplusStartupInput gdiplusStartupInput;
      ULONG_PTR gdiplusToken;
      Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

      // get handle 
      HWND desktop = GetDesktopWindow();
      HDC desktopdc = GetDC(desktop);
      HDC mydc = CreateCompatibleDC(desktopdc);

      width = GetSystemMetrics(SM_CXSCREEN);
      height = GetSystemMetrics(SM_CYSCREEN);

      // create compatible bitmap 
      HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
      // load bitmap into selected device context 
      HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);

      // The BitBlt function performs a bit-block transfer of the color data corresponding 
      // to a rectangle of pixels from the specified source device context into a destination device context.
      BitBlt(mydc, 0, 0, width, height, desktopdc, lprcMonitor->left, lprcMonitor->top, SRCCOPY | CAPTUREBLT);

      SelectObject(mydc, oldbmp);

      // save to filedisk 
      Gdiplus::Bitmap* b = Gdiplus::Bitmap::FromHBITMAP(mybmp, NULL);
      CLSID  encoderClsid;
      Gdiplus::Status stat = Gdiplus::GenericError;
      if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
        // get filename to write to 
        std::wstring tmp(callbackData->filename);
        // add value of current deskpt as identifier 
        tmp = std::to_wstring(callbackData->index) + L"_" + tmp;
        // save to localdisk 
        stat = b->Save(tmp.c_str(), &encoderClsid, NULL);
      }
      if (b)
        delete b;

      // cleanup
      Gdiplus::GdiplusShutdown(gdiplusToken);
      ReleaseDC(desktop, desktopdc);
      DeleteObject(mybmp);
      DeleteDC(mydc);
    }
  }
  else
  {
    HGDIOBJ originalBmp = NULL;
    int height = 0;
    int width = 0;

    // instantiate gdi 
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // get device context handle 
    HWND desktop = GetDesktopWindow();
    HDC desktopdc = GetDC(desktop);
    HDC mydc = CreateCompatibleDC(desktopdc);

    // get width and height 
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);

    // create compatible bitmap 
    HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
    // load bitmap into device context 
    HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);

    // copy bits from selected dc and handle to our dc 
    BitBlt(mydc, 0, 0, width, height, desktopdc, lprcMonitor->left, lprcMonitor->top, SRCCOPY | CAPTUREBLT);
    SelectObject(mydc, oldbmp);

    // save to file 
    Gdiplus::Bitmap* b = Gdiplus::Bitmap::FromHBITMAP(mybmp, NULL);
    CLSID  encoderClsid;
    Gdiplus::Status stat = Gdiplus::GenericError;
    if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
      std::wstring tmp(callbackData->filename);
      tmp = std::to_wstring(callbackData->index) + L"_" + tmp;
      stat = b->Save(tmp.c_str(), &encoderClsid, NULL);
    }
    if (b)
      delete b;

    // cleanup
    Gdiplus::GdiplusShutdown(gdiplusToken);
    ReleaseDC(desktop, desktopdc);
    DeleteObject(mybmp);
    DeleteDC(mydc);
  }
  callbackData->index++;
  return true;
}

void createScreenShotForEachDesktop(const wchar_t* filename, const std::vector<int>& monitorsToDisplay)
{
  
  CallbackData CallbackData(filename, monitorsToDisplay);
  HDC hDesktopDC = GetDC(NULL);
  EnumDisplayMonitors(hDesktopDC, NULL, IterateThroughDesktopsAndPrint, reinterpret_cast<LPARAM>(&CallbackData));

}
