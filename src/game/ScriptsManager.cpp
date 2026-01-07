#include "Logger.hpp"
#include "ScriptsManager.hpp"
#include "DebugDraw.hpp"
#include "Camera.hpp"

#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include "GameView.hpp"

ScriptsManager::ScriptsManager() {
    m_lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::string,
        sol::lib::math
    );

    m_lua.new_usertype<glm::vec2>("vec2",
        sol::call_constructor,
        sol::constructors<
            glm::vec2(),
            glm::vec2(float, float)
        >(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,

        "r", &glm::vec2::x,
        "g", &glm::vec2::y,

        sol::meta_function::less_than,              sol::resolve<glm::bvec2(const glm::vec2&, const glm::vec2&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec2(const glm::vec2&, const glm::vec2&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool      (const glm::vec2&, const glm::vec2&)>(&glm::operator==),
        sol::meta_function::subtraction,            sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(glm::operator-),
        sol::meta_function::addition,               sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator+),
        sol::meta_function::division,               sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator/),
        sol::meta_function::multiplication,         sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator*),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec2 (const glm::vec2&)                  >(&glm::operator-)
    );

    m_lua.new_usertype<glm::vec3>("vec3",
        sol::call_constructor,
        sol::constructors<
            glm::vec3(),
            glm::vec3(float, float, float)
        >(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,

        "r", &glm::vec3::x,
        "g", &glm::vec3::y,
        "b", &glm::vec3::z,

        sol::meta_function::less_than,              sol::resolve<glm::bvec3(const glm::vec3&, const glm::vec3&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec3(const glm::vec3&, const glm::vec3&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool      (const glm::vec3&, const glm::vec3&)>(&glm::operator==),
        sol::meta_function::subtraction,            sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(glm::operator-),
        sol::meta_function::addition,               sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator+),
        sol::meta_function::division,               sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator/),
        sol::meta_function::multiplication,         sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator*),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec3 (const glm::vec3&)                  >(&glm::operator-)
        // sol::meta_function::modulus,        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(&glm::mod)
    );

    m_lua.new_usertype<glm::ivec3>("ivec3",
        sol::call_constructor,
        sol::constructors<
            glm::ivec3(),
            glm::ivec3(glm::vec3),
            glm::ivec3(int32_t, int32_t, int32_t),
            glm::ivec3(float, float, float)
        >(),
        "x", &glm::ivec3::x,
        "y", &glm::ivec3::y,
        "z", &glm::ivec3::z,

        "r", &glm::ivec3::x,
        "g", &glm::ivec3::y,
        "b", &glm::ivec3::z,

        sol::meta_function::less_than,              sol::resolve<glm::bvec3(const glm::ivec3&, const glm::ivec3&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec3(const glm::ivec3&, const glm::ivec3&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool      (const glm::ivec3&, const glm::ivec3&)>(&glm::operator==),
        sol::meta_function::subtraction,            sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(glm::operator-),
        sol::meta_function::addition,               sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator+),
        sol::meta_function::division,               sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator/),
        sol::meta_function::multiplication,         sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator*),
        sol::meta_function::unary_minus,            sol::resolve<glm::ivec3 (const glm::ivec3&)                  >(&glm::operator-)
        // sol::meta_function::modulus,        sol::resolve<glm::ivec3(const glm::ivec3&, const glm::ivec3&)>(&glm::mod)
    );

    m_lua.new_usertype<glm::vec4>("vec4",
        sol::call_constructor,
        sol::constructors<
            glm::vec4(),
            glm::vec4(float, float, float, float)
        >(),
        "x", &glm::vec4::x,
        "y", &glm::vec4::y,
        "z", &glm::vec4::z,
        "w", &glm::vec4::w,

        "r", &glm::vec4::x,
        "g", &glm::vec4::y,
        "b", &glm::vec4::z,
        "a", &glm::vec4::w,

        sol::meta_function::less_than,              sol::resolve<glm::bvec4(const glm::vec4&, const glm::vec4&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec4(const glm::vec4&, const glm::vec4&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool      (const glm::vec4&, const glm::vec4&)>(&glm::operator==),
        sol::meta_function::subtraction,            sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(glm::operator-),
        sol::meta_function::addition,               sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator+),
        sol::meta_function::division,               sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator/),
        sol::meta_function::multiplication,         sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator*),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec4 (const glm::vec4&)                  >(&glm::operator-)
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

    // temporary code for accessing placeSphere()
    m_lua.new_usertype<GameView>("GameView",
        sol::no_constructor,
        "placeSphere", &GameView::placeSphere
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
