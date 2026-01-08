#include "ScriptsManager.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/random.hpp>

void ScriptsManager::GLMBindings() {
    sol::table table_glm = m_lua.create_named_table("glm");

    /* Vector Types */
    m_lua.new_usertype<glm::vec2>("vec2",
        sol::call_constructor,
        sol::constructors<
            glm::vec2(),
            glm::vec2(glm::ivec2),
            glm::vec2(float),
            glm::vec2(float, float)
        >(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,

        "r", &glm::vec2::x,
        "g", &glm::vec2::y,

        sol::meta_function::less_than,              sol::resolve<glm::bvec2(const glm::vec2&, const glm::vec2&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec2(const glm::vec2&, const glm::vec2&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool      (const glm::vec2&, const glm::vec2&)>(&glm::operator==),

        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator-),
                                                    sol::resolve<glm::vec2 (const glm::vec2&, float)           >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator+),
                                                    sol::resolve<glm::vec2 (const glm::vec2&, float)           >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator/),
                                                    sol::resolve<glm::vec2 (const glm::vec2&, float)           >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::vec2 (const glm::vec2&, const glm::vec2&)>(&glm::operator*),
                                                    sol::resolve<glm::vec2 (const glm::vec2&, float)           >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec2 (const glm::vec2&)                  >(&glm::operator-)
    );

    m_lua.new_usertype<glm::vec3>("vec3",
        sol::call_constructor,
        sol::constructors<
            glm::vec3(),
            glm::vec3(glm::ivec3),
            glm::vec3(float),
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
        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator-),
                                                    sol::resolve<glm::vec3 (const glm::vec3&, float)           >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator+),
                                                    sol::resolve<glm::vec3 (const glm::vec3&, float)           >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator/),
                                                    sol::resolve<glm::vec3 (const glm::vec3&, float)           >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::vec3 (const glm::vec3&, const glm::vec3&)>(&glm::operator*),
                                                    sol::resolve<glm::vec3 (const glm::vec3&, float)           >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec3 (const glm::vec3&)                  >(&glm::operator-)
    );

    m_lua.new_usertype<glm::vec4>("vec4",
        sol::call_constructor,
        sol::constructors<
            glm::vec4(),
            glm::vec4(glm::ivec4),
            glm::vec4(float),
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
        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator-),
                                                    sol::resolve<glm::vec4 (const glm::vec4&, float)           >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator+),
                                                    sol::resolve<glm::vec4 (const glm::vec4&, float)           >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator/),
                                                    sol::resolve<glm::vec4 (const glm::vec4&, float)           >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::vec4 (const glm::vec4&, const glm::vec4&)>(&glm::operator*),
                                                    sol::resolve<glm::vec4 (const glm::vec4&, float)           >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::vec4 (const glm::vec4&)                  >(&glm::operator-)
    );

    m_lua.new_usertype<glm::ivec2>("ivec2",
        sol::call_constructor,
        sol::constructors<
            glm::ivec2(),
            glm::ivec2(glm::vec2),
            glm::ivec2(int32_t),
            glm::ivec2(int32_t, int32_t)
        >(),
        "x", &glm::ivec2::x,
        "y", &glm::ivec2::y,

        "r", &glm::ivec2::x,
        "g", &glm::ivec2::y,

        sol::meta_function::less_than,              sol::resolve<glm::bvec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool       (const glm::ivec2&, const glm::ivec2&)>(&glm::operator==),
        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::operator-),
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, int32_t)          >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::operator+),
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, int32_t)          >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::operator/),
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, int32_t)          >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, const glm::ivec2&)>(&glm::operator*),
                                                    sol::resolve<glm::ivec2 (const glm::ivec2&, int32_t)          >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::ivec2 (const glm::ivec2&)                   >(&glm::operator-)
    );

    m_lua.new_usertype<glm::ivec3>("ivec3",
        sol::call_constructor,
        sol::constructors<
            glm::ivec3(),
            glm::ivec3(glm::vec3),
            glm::ivec3(int32_t)       ,
            glm::ivec3(int32_t, int32_t, int32_t)
        >(),
        "x", &glm::ivec3::x,
        "y", &glm::ivec3::y,
        "z", &glm::ivec3::z,

        "r", &glm::ivec3::x,
        "g", &glm::ivec3::y,
        "b", &glm::ivec3::z,

        sol::meta_function::less_than,              sol::resolve<glm::bvec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool       (const glm::ivec3&, const glm::ivec3&)>(&glm::operator==),
        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator-),
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, int32_t)          >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator+),
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, int32_t)          >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator/),
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, int32_t)          >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, const glm::ivec3&)>(&glm::operator*),
                                                    sol::resolve<glm::ivec3 (const glm::ivec3&, int32_t)          >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::ivec3 (const glm::ivec3&)                   >(&glm::operator-)
    );

    m_lua.new_usertype<glm::ivec4>("ivec4",
        sol::call_constructor,
        sol::constructors<
            glm::ivec4(),
            glm::ivec4(glm::vec4),
            glm::ivec4(int32_t),
            glm::ivec4(int32_t, int32_t, int32_t, int32_t)
        >(),
        "x", &glm::ivec4::x,
        "y", &glm::ivec4::y,
        "z", &glm::ivec4::z,
        "w", &glm::ivec4::w,

        "r", &glm::ivec4::x,
        "g", &glm::ivec4::y,
        "b", &glm::ivec4::z,
        "a", &glm::ivec4::w,

        sol::meta_function::less_than,              sol::resolve<glm::bvec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::lessThan),
        sol::meta_function::less_than_or_equal_to,  sol::resolve<glm::bvec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::lessThanEqual),
        sol::meta_function::equal_to,               sol::resolve<bool       (const glm::ivec4&, const glm::ivec4&)>(&glm::operator==),
        sol::meta_function::subtraction, sol::overload(
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::operator-),
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, int32_t)          >(&glm::operator-)
        ),
        sol::meta_function::addition, sol::overload(
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::operator+),
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, int32_t)          >(&glm::operator+)
        ),
        sol::meta_function::division, sol::overload(
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::operator/),
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, int32_t)          >(&glm::operator/)
        ),
        sol::meta_function::multiplication, sol::overload(
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, const glm::ivec4&)>(&glm::operator*),
                                                    sol::resolve<glm::ivec4 (const glm::ivec4&, int32_t)          >(&glm::operator*)
        ),
        sol::meta_function::unary_minus,            sol::resolve<glm::ivec4 (const glm::ivec4&)                   >(&glm::operator-)
    );

    /* Common functions */
    table_glm.set_function("abs", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::floor),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::floor),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::floor),

        sol::resolve<glm::ivec2(const glm::ivec2&)>(&glm::sign),
        sol::resolve<glm::ivec3(const glm::ivec3&)>(&glm::sign),
        sol::resolve<glm::ivec4(const glm::ivec4&)>(&glm::sign)
    ));

    table_glm.set_function("ceil", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::floor),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::floor),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::floor)
    ));

    table_glm.set_function("floor", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::floor),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::floor),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::floor)
    ));

    table_glm.set_function("max", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, float)>(&glm::max),
        sol::resolve<glm::vec3(const glm::vec3&, float)>(&glm::max),
        sol::resolve<glm::vec4(const glm::vec4&, float)>(&glm::max),

        sol::resolve<glm::vec2(const glm::vec2&, const glm::vec2&)>(&glm::max),
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(&glm::max),
        sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&)>(&glm::max),

        sol::resolve<glm::ivec2(const glm::ivec2&, int)>(&glm::max),
        sol::resolve<glm::ivec3(const glm::ivec3&, int)>(&glm::max),
        sol::resolve<glm::ivec4(const glm::ivec4&, int)>(&glm::max),

        sol::resolve<glm::ivec2(const glm::ivec2&, const glm::ivec2&)>(&glm::max),
        sol::resolve<glm::ivec3(const glm::ivec3&, const glm::ivec3&)>(&glm::max),
        sol::resolve<glm::ivec4(const glm::ivec4&, const glm::ivec4&)>(&glm::max)
    ));

    table_glm.set_function("min", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, float)>(&glm::min),
        sol::resolve<glm::vec3(const glm::vec3&, float)>(&glm::min),
        sol::resolve<glm::vec4(const glm::vec4&, float)>(&glm::min),

        sol::resolve<glm::vec2(const glm::vec2&, const glm::vec2&)>(&glm::min),
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(&glm::min),
        sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&)>(&glm::min),

        sol::resolve<glm::ivec2(const glm::ivec2&, int)>(&glm::min),
        sol::resolve<glm::ivec3(const glm::ivec3&, int)>(&glm::min),
        sol::resolve<glm::ivec4(const glm::ivec4&, int)>(&glm::min),

        sol::resolve<glm::ivec2(const glm::ivec2&, const glm::ivec2&)>(&glm::min),
        sol::resolve<glm::ivec3(const glm::ivec3&, const glm::ivec3&)>(&glm::min),
        sol::resolve<glm::ivec4(const glm::ivec4&, const glm::ivec4&)>(&glm::min)
    ));

    table_glm.set_function("mix", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, const glm::vec2&, float)>(&glm::mix),
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&, float)>(&glm::mix),
        sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&, float)>(&glm::mix),
        sol::resolve<float(float, float, float)>(&glm::mix)
    ));

    table_glm.set_function("mod", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, float)>(&glm::mod),
        sol::resolve<glm::vec3(const glm::vec3&, float)>(&glm::mod),
        sol::resolve<glm::vec4(const glm::vec4&, float)>(&glm::mod)
    ));

    table_glm.set_function("round", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::round),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::round),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::round)
    ));

    table_glm.set_function("sign", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::sign),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::sign),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::sign),

        sol::resolve<glm::ivec2(const glm::ivec2&)>(&glm::sign),
        sol::resolve<glm::ivec3(const glm::ivec3&)>(&glm::sign),
        sol::resolve<glm::ivec4(const glm::ivec4&)>(&glm::sign)
    ));

    /* Geometric functions */
    table_glm.set_function("cross",
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(&glm::cross));

    table_glm.set_function("distance", sol::overload(
        sol::resolve<float(const glm::vec2&, const glm::vec2&)>(&glm::distance),
        sol::resolve<float(const glm::vec3&, const glm::vec3&)>(&glm::distance),
        sol::resolve<float(const glm::vec4&, const glm::vec4&)>(&glm::distance)
    ));

    table_glm.set_function("dot", sol::overload(
        sol::resolve<float(const glm::vec2&, const glm::vec2&)>(&glm::dot),
        sol::resolve<float(const glm::vec3&, const glm::vec3&)>(&glm::dot),
        sol::resolve<float(const glm::vec4&, const glm::vec4&)>(&glm::dot)
    ));

    table_glm.set_function("length", sol::overload(
        sol::resolve<float(const glm::vec2&)>(&glm::length),
        sol::resolve<float(const glm::vec3&)>(&glm::length),
        sol::resolve<float(const glm::vec4&)>(&glm::length)
    ));

    table_glm.set_function("normalize", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&)>(&glm::normalize),
        sol::resolve<glm::vec3(const glm::vec3&)>(&glm::normalize),
        sol::resolve<glm::vec4(const glm::vec4&)>(&glm::normalize)
    ));

    table_glm.set_function("reflect", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, const glm::vec2&)>(&glm::reflect),
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(&glm::reflect),
        sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&)>(&glm::reflect)
    ));

    table_glm.set_function("refract", sol::overload(
        sol::resolve<glm::vec2(const glm::vec2&, const glm::vec2&, float)>(&glm::refract),
        sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&, float)>(&glm::refract),
        sol::resolve<glm::vec4(const glm::vec4&, const glm::vec4&, float)>(&glm::refract)
    ));

    /* GLM_GTC_random */

    table_glm.set_function("ballRand", sol::overload(
        sol::resolve<glm::vec3(float)>(&glm::ballRand)
    ));

    table_glm.set_function("circularRand", sol::overload(
        sol::resolve<glm::vec2(float)>(&glm::circularRand),
        sol::resolve<glm::ivec2(int32_t)>(&glm::circularRand)
    ));

    table_glm.set_function("diskRand", sol::overload(
        sol::resolve<glm::vec2(float)>(&glm::diskRand)
    ));

    table_glm.set_function("linearRand", sol::overload(
        sol::resolve<float(float, float)>(&glm::linearRand),
        sol::resolve<int32_t(int32_t, int32_t)>(&glm::linearRand)
    ));

    table_glm.set_function("sphericalRand", sol::overload(
        sol::resolve<glm::vec3(float)>(&glm::sphericalRand),
        sol::resolve<glm::ivec3(int32_t)>(&glm::sphericalRand)
    ));
}
