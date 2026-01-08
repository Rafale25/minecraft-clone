#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <vector>

struct ScriptCallbacks {
    sol::optional<sol::protected_function> onInit;
    sol::optional<sol::protected_function> onRefresh;
    sol::optional<sol::protected_function> onUpdate;
    sol::optional<sol::protected_function> onKeyPress;
};

struct Script {
    std::string path;
    sol::environment env;
    bool valid;

    ScriptCallbacks callbacks = {};
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
    x Camera

API
    all DebugDraws
    // setSphere
    // setCube
    World::setCube(pos)
    World::setSphere(pos, radius)
    World::setCuboid(pos, width)
*/

// global variables
// events
// World API

class Camera;
class GameView;

class ScriptsManager {
public:
    ScriptsManager();

    void registerScript(const char* path);
    void refresh();
    void init(const Camera& camer, const GameView& gameview);

    void GLMBindings();

    // void updateVariables(const Camera& camera);
    void update(float timeSinceStart, float deltaTime);
    void onKeyPress(int key);

private:
    sol::state m_lua;
    std::vector<Script> m_scripts;
};
