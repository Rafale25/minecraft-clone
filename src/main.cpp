#include "command_line_args.h"
#include "Context.hpp"
#include "GameView.hpp"

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const char * const TITLE = "Minecraft Clone";
const int VSYNC = 1;

int main(int argc, char **argv)
{
    global_argc = argc;
    global_argv = argv;

    if (global_argc < 2) {
        printf("Error: Please enter server address in command line argument.\n");
        return -1;
    }

    Context ctx(SCR_WIDTH, SCR_HEIGHT, TITLE, 0, 4);
    GameView view(ctx);

    ctx.setVsync(VSYNC);
    ctx.showView(view);
    ctx.run();

    return 0;
}
