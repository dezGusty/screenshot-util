#ifndef DISPLAY_HANDLE_POOL_H_
#define DISPLAY_HANDLE_POOL_H_

#include <vector>
#include <windows.h>
typedef std::vector<std::pair<HDC, RECT>> HDCPoolType;

class CDisplayHandlesPool
{
private:
  HDCPoolType m_hdcPool;

  CDisplayHandlesPool(const CDisplayHandlesPool & other);
  CDisplayHandlesPool & operator = (CDisplayHandlesPool);
public:

  typedef HDCPoolType::iterator iterator;

  CDisplayHandlesPool()
  {
    HDC hDesktopDC = GetDC(NULL);
    if (!hDesktopDC)
    {
      throw std::runtime_error("CDisplayHandlesPool: GetDC failed");
    }
    if (!EnumDisplayMonitors(hDesktopDC, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(this)))
    {
      throw std::runtime_error("CDisplayHandlesPool: EnumDisplayMonitors failed");
    }
  }
  ~CDisplayHandlesPool()
  {
    for (HDCPoolType::iterator it = m_hdcPool.begin(); it != m_hdcPool.end(); ++it)
    {
      if (it->first)
        DeleteDC(it->first);
    }
  }

  void AddHdcToPool(guards::CDCGuard & hdcGuard, RECT rect)
  {
    m_hdcPool.push_back(std::make_pair(hdcGuard.get(), rect));
    hdcGuard.release();
  }

  iterator begin()
  {
    return m_hdcPool.begin();
  }
  iterator end()
  {
    return m_hdcPool.end();
  }
};

#endif // DISPLAY_HANDLE_POOL_H_