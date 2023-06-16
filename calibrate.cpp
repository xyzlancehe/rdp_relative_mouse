#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <windows.h>
#include "interception/interception.h"
#include "config.h"

int main()
{
    int xMin = 65536;
    int xMax = 0;
    int yMin = 65536;
    int yMax = 0;

    std::ifstream configFile(configFileName);
    if (!configFile.good())
    {
        std::cout << "Config file not found. Creating it automatically." << std::endl;
        createNewConfig();
    }
    configFile.close();

    InterceptionContext ctx = interception_create_context();
    InterceptionDevice device;
    InterceptionStroke stroke;

    interception_set_filter(ctx, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);
    interception_set_filter(ctx, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);

    std::cout << "Let mouse reach each boundary of the screen to calibrate." << std::endl;
    std::cout << "Press ESC to finish." << std::endl;
    std::cout << "Current boundary values:" << std::endl;

    // Get the current console cursor position
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO cursorInfo;
    GetConsoleScreenBufferInfo(consoleHandle, &cursorInfo);
    int currentLine = cursorInfo.dwCursorPosition.Y;

    while (true)
    {
        int nstroke = interception_receive(ctx, device = interception_wait(ctx), &stroke, 1);

        if (nstroke <= 0)
        {
            continue;
        }

        if (interception_is_keyboard(device))
        {
            InterceptionKeyStroke &kstroke = reinterpret_cast<InterceptionKeyStroke &>(stroke);
            if (kstroke.code == 0x01)
            {
                break;
            }
        }
        else
        {
            InterceptionMouseStroke &mstroke = reinterpret_cast<InterceptionMouseStroke &>(stroke);
            xMin = min(xMin, mstroke.x);
            xMax = max(xMax, mstroke.x);
            yMin = min(yMin, mstroke.y);
            yMax = max(yMax, mstroke.y);

            // write to a fixed line
            std::cout << "\033[" << currentLine + 1 << ";0H";
            std::cout << "\033[2K";
            std::cout << "xMin, xMax, yMin, yMax = "
                      << xMin << ", " << xMax << ", "
                      << yMin << ", " << yMax << std::endl;
            interception_send(ctx, device, &stroke, 1);
        }
    }

    interception_destroy_context(ctx);
    std::cout << "Calibration finished. Writing configs." << std::endl;
    std::string value;
    value = std::to_string(xMax);
    WritePrivateProfileString("Boundary", "xMax", value.c_str(), configFileName);
    value = std::to_string(xMin);
    WritePrivateProfileString("Boundary", "xMin", value.c_str(), configFileName);
    value = std::to_string(yMax);
    WritePrivateProfileString("Boundary", "yMax", value.c_str(), configFileName);
    value = std::to_string(yMin);
    WritePrivateProfileString("Boundary", "yMin", value.c_str(), configFileName);
    std::cout << "Done!" << std::endl;
    std::cout << "Clearing messages in 1.5s." << std::endl;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1.5s);
    system("cls");

    return 0;
}