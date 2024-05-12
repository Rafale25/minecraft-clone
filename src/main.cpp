#include <iostream>

#include "Context.hpp"
#include "GameView.hpp"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const char *TITLE = "Minecraft Clone";
const int VSYNC = 1;

int main()
{
    std::cout << "Hello World!" << std::endl;

    Context ctx(SCR_WIDTH, SCR_HEIGHT, TITLE);
    View* view = new GameView(ctx);

    ctx.setVsync(VSYNC);
    ctx.show_view(view);
    ctx.run();

    return 0;
}
