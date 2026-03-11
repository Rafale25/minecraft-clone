local offset = vec3()

-- x = x or 0

function onInit()
    print("Lua: onInit()")
end

function onRefresh()
    print("Lua: onRefresh()")
end

local projectiles = {}

function onUpdate(timeSinceStart, deltaTime)
    local camPos = (glm::vec3)Camera:getPosition()
    local camForward = Camera:forward()

    local pos = camPos + camForward * 4.0
    -- local raycastHit = World:blockRaycast(camPos, camForward, 10.0)

    -- DebugDraw:drawCube(pos, 1.0, vec3(1, 0, 0))
    -- DebugDraw:drawSphere(pos, 0.5, vec3(1, 0, 0))

    for i=#projectiles, 1, -1 do
        proj = projectiles[i]
        proj.pos = proj.pos + proj.vel

        local block = World:getBlock(ivec3(glm.floor(proj.pos)))

        if block ~= 0 then
            table.remove(projectiles, i)
            GameView:placeSphere(ivec3(proj.pos), proj.power, 0)
        end

        DebugDraw:drawCube(proj.pos, 0.1, vec3(1, 1, 0))
    end

end

function onKeyPress(key)
    -- GLFW_KEY_F = 70
    if (key == 70) then
        FireMultiple(Camera:getPosition(), Camera:forward(), 1000)
    end
end

function FireMultiple(position, direction, count)
    for i=0, count do
        Fire(position, direction)
    end
end

function Fire(position, direction)
    local offset = glm.ballRand(0.5)
    offset = direction + offset
    local vel = (direction + offset) * glm.linearRand(0.4, 0.8)

    local proj = {}
    proj.pos = position
    proj.vel = vel
    proj.power = glm.linearRand(2.3, 5.2)

    table.insert(projectiles, proj)
end
