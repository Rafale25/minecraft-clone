local offset = vec3()

-- x = x or 0
x = 0

function onInit()
    print("Lua: onInit()")
end

function onRefresh()
    print("Lua: onRefresh()")
end

function onUpdate(timeSinceStart, deltaTime)
    offset.y = math.sin(timeSinceStart * 2);
    x = x + deltaTime;

    local p = Camera:getPosition()
    local pos = vec3(-x, offset.y, 0) + p

    DebugDraw:drawCube(pos, 1.0, vec3(1, 0, 0));
    DebugDraw:drawSphere(pos, 0.5, vec3(1, 0, 0));

    GameView:placeSphere(ivec3(pos), 1.0, 0)
    -- print(string.format("Lua: onUpdate %.2f %.5f", timeSinceStart, deltaTime))
end
