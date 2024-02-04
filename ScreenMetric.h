/**
 *  This class gets the size of screen, and can convert coordinates.
 *
 *  `SystemSize` is the virtual resolution for programs.
 *      This size is aquired by function `GetSystemMetrics()`. Function `GetCursorPos()` and `SetCursorPos()` use coordinates in this size.
 *
 *  `DeviceSize` is the resolution of the real monitor device.
 *      This size is aquired by function `GetDeviceCaps()`. Struct `MSLLHOOKSTRUCT` uses coordinates in this size.
 *
 *  Usually, `DeviceSize == SystemSize * ScaleFactor`.
 *  For example, if the monitor is 2560x1440 and the scale factor is 1.25, the SystemSize is 2048x1152.
 */
class ScreenMetric
{
public:
    ScreenMetric();

    int xSystemSize() const;
    int ySystemSize() const;
    int xDeviceSize() const;
    int yDeviceSize() const;

    int xSystemToDevice(int) const;
    int ySystemToDevice(int) const;
    int xDeviceToSystem(int) const;
    int yDeviceToSystem(int) const;

private:
    int _xSystemSize;
    int _ySystemSize;
    int _xDeviceSize;
    int _yDeviceSize;
};
