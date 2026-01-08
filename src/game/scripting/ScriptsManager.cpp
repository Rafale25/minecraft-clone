#include "Logger.hpp"
#include "ScriptsManager.hpp"
#include "DebugDraw.hpp"
#include "Camera.hpp"

#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include "GameView.hpp"
#include "World.hpp"
#include "enums.hpp"


ScriptsManager::ScriptsManager() {
    m_lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::string,
        sol::lib::math,
        sol::lib::table
        // sol::lib::jit
    );


    m_lua.new_usertype<Camera>("Camera",
        sol::no_constructor,
        "getPosition", &Camera::getPosition,
        "right", &Camera::right,
        "up", &Camera::up,
        "forward", &Camera::forward
    );

    m_lua.new_usertype<DebugDraw>("DebugDraw",
        sol::no_constructor,
        "drawLine", &DebugDraw::drawLine,
        "drawRay", &DebugDraw::drawRay,
        "drawCube", &DebugDraw::drawCube,
        "drawCuboid", &DebugDraw::drawCuboid,
        "drawCuboidMinMax", &DebugDraw::drawCuboidMinMax,
        "drawSphere", &DebugDraw::drawSphere
    );

    GLMBindings();

    // m_lua.new_enum("BlockType",
    //     "Air", BlockType::Air
    // );

    // temporary code for accessing placeSphere()
    m_lua.new_usertype<GameView>("GameView",
        sol::no_constructor,
        "placeSphere", &GameView::placeSphere
    );

    m_lua.new_usertype<BlockRaycastHit>("BlockRaycastHit",
        sol::no_constructor,
        "hit", &BlockRaycastHit::hit,
        "blocktype", &BlockRaycastHit::blocktype,
        "block_pos", &BlockRaycastHit::block_pos,
        "world_pos", &BlockRaycastHit::world_pos,
        "normal", &BlockRaycastHit::normal
    );

    m_lua.new_usertype<World>("World",
        sol::no_constructor,
        "blockRaycast", sol::resolve<BlockRaycastHit (const glm::vec3&, const glm::vec3&, float) const>(&World::blockRaycast),
        "getBlock", sol::resolve<BlockType (const glm::ivec3&) const>(&World::getBlock)
    );
};

void ScriptsManager::registerScript(const char* path) {
    // sol::environment env(m_lua, sol::create); // empty dummy environment
    sol::environment env(m_lua, sol::create, m_lua.globals());

    m_scripts.push_back({path, env, false}); // might need std::move
}

void ScriptsManager::refresh() {
    for (auto& [path, env, valid, callbacks] : m_scripts) {
        // env = sol::environment(m_lua, sol::create, m_lua.globals());
        sol::load_result script = m_lua.load_file(path);
        valid = script.valid();

        if (!valid) {
            sol::error err = script;
            logE("Failed to load script {}\n{}", path, err.what());
        } else {
            script(env);

            callbacks.onInit = env[std::string("onInit")];
            callbacks.onRefresh = env[std::string("onRefresh")];
            callbacks.onUpdate = env[std::string("onUpdate")];
            callbacks.onKeyPress = env[std::string("onKeyPress")];

            logI("Successfully refreshed script: {}", path);
        }
    }
}

void ScriptsManager::init(const Camera& camera, const GameView& gameview) {
    /* Set global variables references */
    m_lua["Camera"] = &camera;
    m_lua["DebugDraw"] = &DebugDraw::instance();
    m_lua["GameView"] = &gameview;
    m_lua["World"] = &World::instance();

    // void GameView::placeSphere(const glm::ivec3& center, float radius, BlockType blocktype)

    // sol::table table_debugDraw = m_lua.create_named_table("World");
    // table_debugDraw.set_function("placeSphere",
    //     [&](const glm::vec3& pos, float size, int blocktype) {
    //         // placeSphere(const glm::ivec3& center, float radius, BlockType blocktype)
    //     });

    // sol::table table_debugDraw = m_lua.create_named_table("DebugDraw");
    // table_debugDraw.set_function("drawCube",
    //     [&](const glm::vec3& pos, float size, const glm::vec3& color) {
    //         DebugDraw::instance().drawCube(pos, size, color);
    //     });

    for (const auto& [path, env, valid, callbacks] : m_scripts) {
        if (!valid) continue;
        if (!callbacks.onInit) continue;

        sol::protected_function_result result = callbacks.onInit.value()();
        if (!result.valid()) {
            sol::error err = result;
            logE("{}", err.what());
        }
    }
}

void ScriptsManager::update(float timeSinceStart, float deltaTime) {
    for (const auto& [path, env, valid, callbacks] : m_scripts) {
        if (!valid) continue;
        if (!callbacks.onUpdate) continue;

        sol::protected_function_result result = callbacks.onUpdate.value()(timeSinceStart, deltaTime);
        if (!result.valid()) {
            sol::error err = result;
            logE("{}", err.what());
        }
    }
}

void ScriptsManager::onKeyPress(int key) {
    for (const auto& [path, env, valid, callbacks] : m_scripts) {
        if (!valid) continue;
        if (!callbacks.onKeyPress) continue;

        sol::protected_function_result result = callbacks.onKeyPress.value()(key);
        if (!result.valid()) {
            sol::error err = result;
            logE("{}", err.what());
        }
    }
}
