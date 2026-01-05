function onInit()
    print("Lua: onInit()")
end

function onUpdate(timeSinceStart, deltaTime)
    local formatted = string.format("Lua: onUpdate %.2f %.5f", timeSinceStart, deltaTime)
    print(formatted)
    -- print("Lua: onUpdate "..timeSinceStart.." "..deltaTime)
end
