#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <vector>

struct Script {
    std::string path;
    sol::environment env;
};

/*

Callbacks
    onInit
    onFixedUpdate
    onUpdate
    onRefresh
    onEvent
    onEntityCreated
    onEntityDestroyed
    onMessage

Global Variables
    Camera
    ...?

API
    all DebugDraws
    // setSphere
    // setCube
    World::setCube(pos)
    World::setSphere(pos, radius)
    World::setCuboid(pos, width)
    World::
*/

class ScriptsManager {
public:
    ScriptsManager();

    void registerScript(const char* path);
    void init();
    void update(float timeSinceStart, float deltaTime);
    // void initialize() {}

private:
    sol::state lua;
    std::vector<Script> m_scripts;
};
