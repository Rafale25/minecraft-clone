#include "command_line_args.h"
#include "Context.hpp"
#include "GameView.hpp"
#include "Logger.hpp"

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const char* const TITLE = "Minecraft Clone";
const bool VSYNC = true;

#include <glm/vec4.hpp>

int main(int argc, char** argv)
{
    global_argc = argc;
    global_argv = argv;

    if (global_argc < 4) {
        logE("Please enter ip, port and render distance in command line argument");
        return -1;
    }

    Context ctx(SCR_WIDTH, SCR_HEIGHT, TITLE, 0, 4);
    GameView view(ctx);

    ctx.setVsync(VSYNC);
    ctx.showView(view);
    ctx.run();

    return 0;
}
