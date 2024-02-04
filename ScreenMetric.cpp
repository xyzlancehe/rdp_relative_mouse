#include <windows.h>
#include "ScreenMetric.h"

ScreenMetric::ScreenMetric()
{
    this->_xSystemSize = GetSystemMetrics(SM_CXSCREEN);
    this->_ySystemSize = GetSystemMetrics(SM_CYSCREEN);

    HDC hDC = GetDC(NULL);
    this->_xDeviceSize = GetDeviceCaps(hDC, DESKTOPHORZRES);
    this->_yDeviceSize = GetDeviceCaps(hDC, DESKTOPVERTRES);
    ReleaseDC(NULL, hDC);
}

int ScreenMetric::xSystemSize() const
{
    return this->_xSystemSize;
}

int ScreenMetric::ySystemSize() const
{
    return this->_ySystemSize;
}

int ScreenMetric::xDeviceSize() const
{
    return this->_xDeviceSize;
}

int ScreenMetric::yDeviceSize() const
{
    return this->_yDeviceSize;
}

int ScreenMetric::xSystemToDevice(int x) const
{
    return x * this->_xDeviceSize / this->_xSystemSize;
}

int ScreenMetric::ySystemToDevice(int y) const
{
    return y * this->_yDeviceSize / this->_ySystemSize;
}

int ScreenMetric::xDeviceToSystem(int x) const
{
    return x * this->_xSystemSize / this->_xDeviceSize;
}

int ScreenMetric::yDeviceToSystem(int y) const
{
    return y * this->_ySystemSize / this->_yDeviceSize;
}
