#pragma once

#include "Context.hpp"
#include "View.hpp"
#include "camera/FpsCamera.hpp"
#include "WorldRenderer.hpp"
#include "BlockRaycastHit.hpp"
#include "ScriptsManager.hpp"

class GameView: public View {
public:
    GameView(Context& ctx);

    void onUpdate(double time_since_start, float dt);
    void onHideView();

    void deleteFarChunks();
    void processNewChunks();
    void networkUpdate();

    void playerMovements(float dt);
    void sendTextMessage();
    void placeSphere(const glm::ivec3& center, float radius, BlockType blocktype);
    void setPlayerPosition(const glm::vec3& p);

    void drawPlayersNames();
    void guiWorldEdit();
    void gui(float dt);

    void onDraw(double time_since_start, float dt);
    void onKeyPress(int key);
    void onMousePress(int x, int y, int button);
    void onMouseDrag(int x, int y, int dx, int dy);
    void onMouseScroll(int scroll_x, int scroll_y);
    void onMouseMotion(int x, int y, int dx, int dy);
    void onResize(int width, int height);

private:
    WorldRenderer m_worldRenderer{ctx.width, ctx.height};
    ScriptsManager m_scriptManager;

    float m_networkTimer = 1.0f;

    bool m_showDebugGui = false;
    bool m_showProfilerGui = false;
    bool m_cursorEnabled = false;
    bool m_vsyncEnabled = true;
    bool m_deleteFarChunks = true;

    bool m_drawChunksBorders = false;
    bool m_drawPlayerChunk = false;
    bool m_drawPlayerColliders = false;
    bool m_drawHitPoint = false;

    FPSCamera m_camera = {
        glm::vec3(0.0f, 0.0, 0.0f), 0.0f, 0.0f,
        60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 5000.0f,
        true
    };

    BlockType m_blockInHand = BlockType::Grass;
    float m_bulkEditRadius = 4.0f;

    BlockRaycastHit m_playerBlockRaycastHit;

    // -- Player physic -- //
        bool m_freeCamEnabled = true;
        const float m_playerHeight = 1.8f;
        const float m_playerRadius = 0.3f;
        const float m_playerGravity = 45.0f;
        glm::vec3 m_playerVelocity = {0.0f, 0.0f, 0.0f};
        float m_lastJumpPress = 0;
    // --

    // -- World editor -- //
    bool m_blockSelectionMode = false;
    glm::ivec3 blockA = {0.0f, 0.0f, 0.0f};
    glm::ivec3 blockB = {0.0f, 0.0f, 0.0f};
    // --

    char m_inputTextBuffer[4096] = {};
    std::vector<std::string> m_tchat;
};
