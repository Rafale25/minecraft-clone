#include "command_line_args.h"
#include "Context.hpp"
#include "GameView.hpp"
#include "Logger.hpp"

// #include "toml.hpp"

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;
const char* const TITLE = "Minecraft Clone";
const bool VSYNC = true;

int main(int argc, char** argv)
{
    // toml::table tbl;
    // try
    // {
    //     tbl = toml::parse_file("gameconfig.ini");
    //     std::cout << tbl << "\n";
    // }
    // catch (const toml::parse_error& err)
    // {
    //     std::cerr << "Parsing failed:\n" << err << "\n";
    //     return 1;
    // }
    global_argc = 4;//argc;
    global_argv = argv;

    if (global_argc < 4) {
        logE("Please enter ip, port and render distance in command line argument");
        // return -1;
    }

    Context ctx(SCR_WIDTH, SCR_HEIGHT, TITLE, 0, 4);
    GameView view(ctx);

    ctx.setVsync(VSYNC);
    ctx.showView(view);
    ctx.run();

    return 0;
}
