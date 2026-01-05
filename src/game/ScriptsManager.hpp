#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <vector>

struct Script {
    std::string path;
    sol::environment env;
};

// onInit
// onFixedUpdate
// onUpdate
// onRefresh
// onEvent
// onEntityCreated
// onEntityDestroyed
// onMessage

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
