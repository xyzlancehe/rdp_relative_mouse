#include <windows.h>
#include <iostream>
#include <string>
#include "ScreenMetric.h"

void sendRelativeMouseMove(int dx, int dy)
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

ScreenMetric screenMetric{};
int lastX = 0;
int lastY = 0;
int lastDx = 0;
int lastDy = 0;
int crossXBoundaryJudgeThreshold;
int crossYBoundaryJudgeThreshold;

LRESULT CALLBACK hookRelativeMove(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam != WM_MOUSEMOVE)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    MSLLHOOKSTRUCT *pMouseStruct = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);

    if (pMouseStruct->flags & (LLMHF_INJECTED | LLMHF_LOWER_IL_INJECTED))
    {
        // this mouse move is from a SendInput call, block it
        return 1;
    }

    int x = pMouseStruct->pt.x;
    int y = pMouseStruct->pt.y;

    int dx = x - lastX;
    int dy = y - lastY;

    // cross boundary judge
    if (dx < -crossXBoundaryJudgeThreshold && lastDx > 0)
    {
        dx += screenMetric.xDeviceSize();
    }
    else if (dx > crossXBoundaryJudgeThreshold && lastDx < 0)
    {
        dx -= screenMetric.xDeviceSize();
    }
    if (dy < -crossYBoundaryJudgeThreshold && lastDy > 0)
    {
        dy += screenMetric.yDeviceSize();
    }
    else if (dy > crossYBoundaryJudgeThreshold && lastDy < 0)
    {
        dy -= screenMetric.yDeviceSize();
    }

    sendRelativeMouseMove(dx, dy);

    // update last position
    lastX = x;
    lastY = y;
    lastDx = dx;
    lastDy = dy;

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, char *argv[])
{

    crossXBoundaryJudgeThreshold = screenMetric.xDeviceSize() / 2;
    crossYBoundaryJudgeThreshold = screenMetric.yDeviceSize() / 2;

    HHOOK hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, hookRelativeMove, NULL, 0);

    if (hMouseHook == NULL)
    {
        MessageBox(NULL, "Failed to install mouse hook", "Error", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hMouseHook);

    return 0;
}