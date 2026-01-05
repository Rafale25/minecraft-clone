#include "Logger.hpp"
#include <string>
#include "ScriptsManager.hpp"

ScriptsManager::ScriptsManager() {
    lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::string);
};

void ScriptsManager::registerScript(const char* path) {
    sol::environment env(lua, sol::create, lua.globals());

    sol::load_result script = lua.load_file(path);
    if (!script.valid()) {
        sol::error err = script;
        logE("Failed to load script (%s)\n%s", path, err.what());
        // throw std::runtime_error(err.what());
    } else {
        script(env);
    }

    m_scripts.push_back({path, env}); // might need std::move
}

void ScriptsManager::init() {
    for (const auto& [path, env] : m_scripts) {
        sol::optional<sol::function> funcOnInit = env["onInit"];
        if (funcOnInit) {
            (*funcOnInit)();
        }
    }
}

void ScriptsManager::update(float timeSinceStart, float deltaTime) {
    for (const auto& [path, env] : m_scripts) {
        sol::optional<sol::function> funcOnUpdate = env["onUpdate"];
        if (funcOnUpdate) {
            (*funcOnUpdate)(timeSinceStart, deltaTime);
        }

        // auto func = env["onInit"];
        // if (func.valid() && func.is<sol::function>()) {
        //     func();
        // }
    }
}
