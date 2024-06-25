#include "context.hpp"
#include "game_view.hpp"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const char *TITLE = "Minecraft Clone";
const int VSYNC = 1;

int main()
{
    Context ctx(SCR_WIDTH, SCR_HEIGHT, TITLE, 0, 4);
    View* view = new GameView(ctx);

    ctx.setVsync(VSYNC);
    ctx.showView(view);
    ctx.run();

    return 0;
}
