#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <windows.h>
#include "interception/interception.h"
#include "config.h"

int main()
{
    std::vector<std::pair<int, std::string>> flagAndNames{
        {INTERCEPTION_MOUSE_MOVE_ABSOLUTE, "absolute"},
        {INTERCEPTION_MOUSE_MOVE_RELATIVE, "relative"},
        {INTERCEPTION_MOUSE_VIRTUAL_DESKTOP, "virtual desktop"},
        {INTERCEPTION_MOUSE_ATTRIBUTES_CHANGED, "attribute change"},
        {INTERCEPTION_MOUSE_MOVE_NOCOALESCE, "no coalesce"},
        {INTERCEPTION_MOUSE_TERMSRV_SRC_SHADOW, "termsrv src shadow"}};

    InterceptionContext ctx = interception_create_context();
    InterceptionDevice device;
    InterceptionStroke stroke;

    interception_set_filter(ctx, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

    POINT p;

    while (true)
    {
        int nstroke = interception_receive(ctx, device = interception_wait(ctx), &stroke, 1);

        if (nstroke <= 0)
        {
            continue;
        }
        InterceptionMouseStroke &mstroke = reinterpret_cast<InterceptionMouseStroke &>(stroke);

        std::cout << "Got mouse move: { types : [";
        bool first = true;
        for (auto &[flag, name] : flagAndNames)
        {
            bool flagExist = (mstroke.flags & flag);
            if (mstroke.flags == INTERCEPTION_MOUSE_MOVE_RELATIVE && flag == INTERCEPTION_MOUSE_MOVE_RELATIVE)
            {
                flagExist = true;
            }
            if (!flagExist)
            {
                continue;
            }

            if (first)
            {
                std::cout << '"';
            }
            else
            {
                std::cout << ", \"";
            }
            std::cout << name << '"';
            first = false;
        }
        std::cout << "], x : " << mstroke.x << ", y : " << mstroke.y << " }" << std::endl;

        GetCursorPos(&p);
        std::cout << "CursorPos : (" << p.x << ", " << p.y << ")" << std::endl;

        interception_send(ctx, device, &stroke, 1);
    }

    interception_destroy_context(ctx);
    return 0;
}