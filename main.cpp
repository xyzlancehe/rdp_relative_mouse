#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <signal.h>
#include <windows.h>
#include "interception/interception.h"
#include "config.h"

int xMin = 0;
int xMax = 65536;
int yMin = 0;
int yMax = 65536;
InterceptionContext ctx;
InterceptionStroke stroke;
InterceptionDevice device;
float factorX;
float factorY;
bool running = true;

void setRunningStop(int signal);
void loopMode0(int boundarySpeed);
void loopMode1();
void loopMode1Boundary(int interval, int speedStart, int speedEnd, int speedStep);

int main()
{
    // set high priority to make sure performance
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    ctx = interception_create_context();
    interception_set_filter(ctx, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

    std::ifstream configFile(configFileName);
    if (!configFile.good())
    {
        std::cout << "Config file not found. It will be created but the boundary is now calibrated." << std::endl;
        createNewConfig();
    }
    configFile.close();

    // read boundary
    std::cout << "Loading boundaries" << std::endl;
    xMin = GetPrivateProfileInt("Boundary", "xMin", 0, configFileName);
    xMax = GetPrivateProfileInt("Boundary", "xMax", 65535, configFileName);
    yMin = GetPrivateProfileInt("Boundary", "yMin", 0, configFileName);
    yMax = GetPrivateProfileInt("Boundary", "yMax", 65535, configFileName);
    std::cout << "xMin, xMax, yMin, yMax = "
              << xMin << ", " << xMax << ", "
              << yMin << ", " << yMax << std::endl;

    // speed factor at non boundary positions
    char *factorString = new char[128];
    int factorStringLen = GetPrivateProfileString("Main", "factor", NULL, factorString, 128, configFileName);
    float factor = std::stof(std::string(factorString, factorStringLen));
    delete[] factorString;
    int w = GetSystemMetrics(SM_CXFULLSCREEN);
    int h = GetSystemMetrics(SM_CYFULLSCREEN);
    factorX = factor * w / 65535.0f;
    factorY = factor * h / 65535.0f;

    // register ctrl + C
    signal(SIGINT, setRunningStop);

    // main process
    int mode = GetPrivateProfileInt("Main", "mode", 0, configFileName);
    if (mode == 0)
    {
        int boundarySpeed = GetPrivateProfileInt("Manual", "speed", 50, configFileName);
        loopMode0(boundarySpeed);
    }
    else if (mode == 1)
    {
        int interval = GetPrivateProfileInt("Auto", "interval", 100, configFileName);
        int boundarySpeedStart = GetPrivateProfileInt("Auto", "speedStart", 30, configFileName);
        int boundarySpeedEnd = GetPrivateProfileInt("Auto", "speedEnd", 30, configFileName);
        int boundarySpeedStep = GetPrivateProfileInt("Auto", "speedStop", 30, configFileName);
        std::thread boundaryHelper(loopMode1Boundary, interval, boundarySpeedStart, boundarySpeedEnd, boundarySpeedStep);
        loopMode1();
        boundaryHelper.join();
    }
    else
    {
        std::cout << "unknown mode: " << mode << std::endl;
    }

    interception_destroy_context(ctx);
    std::cout << "program exit normally" << std::endl;

    return 0;
}

void setRunningStop(int signal)
{
    running = false;
}

void loopMode0(int boundarySpeed)
{
    std::cout << "start mode0 main loop" << std::endl;
    int lastX = 32767;
    int lastY = 32767;
    while (running)
    {
        int nstroke = interception_receive(ctx, device = interception_wait(ctx), &stroke, 1);

        if (nstroke <= 0)
        {
            continue;
        }

        InterceptionMouseStroke &mstroke = reinterpret_cast<InterceptionMouseStroke &>(stroke);

        // if is not absolute position, pass
        if (!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE))
        {
            interception_send(ctx, device, &stroke, 1);
            continue;
        }

        int currentX = mstroke.x;
        int currentY = mstroke.y;

        // calculate relative movement and send
        int dx = static_cast<int>((currentX - lastX) * factorX);
        int dy = static_cast<int>((currentY - lastY) * factorY);

        if (currentX <= xMin)
        {
            dx = -boundarySpeed;
        }
        if (currentX >= xMax)
        {
            dx = boundarySpeed;
        }
        if (currentY <= yMin)
        {
            dy = -boundarySpeed;
        }
        if (currentY >= yMax)
        {
            dy = boundarySpeed;
        }

        auto relativeStroke = mstroke;
        relativeStroke.x = dx;
        relativeStroke.y = dy;
        relativeStroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;

        interception_send(ctx, device, reinterpret_cast<InterceptionStroke *>(&relativeStroke), 1);

        // then send the original absolute position
        // this is necessary, because the caculated relative move usually does not match local mouse move
        // if not do this, the local mouse cursor we see will not be the real cursor position on remote machine
        interception_send(ctx, device, &stroke, 1);

        lastX = currentX;
        lastY = currentY;

#ifdef _DEBUG
        std::cout << "get absolute mouse move  { flag: " << mstroke.flags << ", x :" << mstroke.x << ", y :" << mstroke.y << "} " << std::endl;
        std::cout << "send dx = " << dx << ", dy = " << dy << std::endl;
#endif
    }
    std::cout << "exit mode0 main loop" << std::endl;
}

void loopMode1()
{
    std::cout << "start mode1 main loop" << std::endl;
    int lastX = 32767;
    int lastY = 32767;
    while (running)
    {
        int nstroke = interception_receive(ctx, device = interception_wait(ctx), &stroke, 1);

        if (nstroke <= 0)
        {
            continue;
        }

        InterceptionMouseStroke &mstroke = reinterpret_cast<InterceptionMouseStroke &>(stroke);

        // similar to mode 0, but do not check the boundary

        if (!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE))
        {
            interception_send(ctx, device, &stroke, 1);
            continue;
        }

        int currentX = mstroke.x;
        int currentY = mstroke.y;
        int dx = static_cast<int>((currentX - lastX) * factorX);
        int dy = static_cast<int>((currentY - lastY) * factorY);

        auto relativeStroke = mstroke;
        relativeStroke.x = dx;
        relativeStroke.y = dy;
        relativeStroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;

        interception_send(ctx, device, reinterpret_cast<InterceptionStroke *>(&relativeStroke), 1);
        interception_send(ctx, device, &stroke, 1);

        lastX = currentX;
        lastY = currentY;

#ifdef _DEBUG
        std::cout << "get absolute mouse move  { flag: " << mstroke.flags << ", x :" << mstroke.x << ", y :" << mstroke.y << "} " << std::endl;
        std::cout << "send dx = " << dx << ", dy = " << dy << std::endl;
#endif
    }

    std::cout << "exit mode1 main loop" << std::endl;
}

void loopMode1Boundary(int interval, int boundarySpeedStart, int boundarySpeedEnd, int boundarySpeedStep)
{
    int speed = boundarySpeedStart;
    std::cout << "start mode1 boundary loop" << std::endl;

    while (running)
    {
        InterceptionMouseStroke &mstroke = reinterpret_cast<InterceptionMouseStroke &>(stroke);
        auto relativeStroke = mstroke;
        relativeStroke.x = 0;
        relativeStroke.y = 0;
        relativeStroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;

        bool isBoundary = false;
        if (mstroke.x <= xMin)
        {
            relativeStroke.x = -speed;
            isBoundary = true;
        }
        if (mstroke.x >= xMax)
        {
            relativeStroke.x = speed;
            isBoundary = true;
        }
        if (mstroke.y <= yMin)
        {
            relativeStroke.y = -speed;
            isBoundary = true;
        }
        if (mstroke.y >= yMax)
        {
            relativeStroke.y = speed;
            isBoundary = true;
        }

        if (isBoundary)
        {
            interception_send(ctx, device, reinterpret_cast<InterceptionStroke *>(&relativeStroke), 1);
#ifdef _DEBUG
            std::cout << "at boundary, send dx = " << relativeStroke.x << ", dy = " << relativeStroke.y << std::endl;
#endif
            speed += boundarySpeedStep;
            speed = min(speed, boundarySpeedEnd);
        }
        else
        {
            speed = boundarySpeedStart;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }

    std::cout << "exit mode1 boundary loop" << std::endl;
}