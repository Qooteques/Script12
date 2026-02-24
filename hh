local WindUI
do
    local ok, result = pcall(function()
        return require("./src/Init")
    end)
    if ok then 
        WindUI = result
    else 
        WindUI = loadstring(game:HttpGet("https://github.com/Footagesus/WindUI/releases/latest/download/main.lua"))()
    end
end

local Players = game:GetService("Players")
local UserInputService = game:GetService("UserInputService")
local TweenService = game:GetService("TweenService")
local RunService = game:GetService("RunService")
local Lighting = game:GetService("Lighting")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Debris = game:GetService("Debris")
local LocalPlayer = Players.LocalPlayer
local PlayerGui = LocalPlayer:WaitForChild("PlayerGui")
local camera = workspace.CurrentCamera

getgenv().featureStates = getgenv().featureStates or {}
getgenv().espObjects = {}
getgenv().espThreads = {}
getgenv().tracerLines = {}

local Window = WindUI:CreateWindow({
    Title = "WhakizashiX-V2",
    Icon = "spiral",
    Author = "baisoku",
    Folder = "WhakizashiX-Hub-X-V2",
    NewElements = true,
    HideSearchBar = true,
    Theme = "Dark",
    Size = UDim2.fromOffset(1000, 460),
    Background = "rbxassetid://71051763264294",
    KeySystem = false
})

Window:EditOpenButton({
    Title = "WhakizashiX-Hub-X-V2",
    Icon = "app-window",
    CornerRadius = UDim.new(0, 16),
    StrokeThickness = 0,
    Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(255, 0, 0)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(200, 50, 0))
    }),
    Enabled = true,
    Draggable = true,
})

local DcTab = Window:Tab({Title = "Discord", Icon = "link"})
local MainTab = Window:Tab({Title = "Player", Icon = "player"})
local EspTab = Window:Tab({Title = "ESP", Icon = "star"})
local VisualTab = Window:Tab({Title = "Visual", Icon = "smile"})
local GameTab = Window:Tab({Title = "Game", Icon = "github"})
local MiscTab = Window:Tab({Title = "Misc", Icon = "coins"})
local WindTab = Window:Tab({Title = "Window", Icon = "gamepad"})

Window:Divider()

local requiredFields = {
    Friction = true,
    AirStrafeAcceleration = true,
    JumpHeight = true,
    RunDeaccel = true,
    JumpSpeedMultiplier = true,
    JumpCap = true,
    SprintCap = true,
    WalkSpeedMultiplier = true,
    BhopEnabled = true,
    Speed = true,
    AirAcceleration = true,
    RunAccel = true,
    SprintAcceleration = true,
}

getgenv().ApplyMode = "Unoptimized"
local appliedOnce = false
local gameStatsPath = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Stats")
local playersPath = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Players")
local playerModelPresent = false

local currentSettings = {
    Speed = "1500",
    JumpCap = "1",
    AirStrafeAcceleration = "187"
}

local function isPlayerModelPresent()
    local player = game.Players.LocalPlayer
    if not player then return false end
    if not playersPath then return false end
    for _, model in pairs(playersPath:GetChildren()) do
        if model.Name == player.Name then
            return true
        end
    end
    return false
end

local function shouldApplySettings()
    if not gameStatsPath then return false end
    local roundStarted = gameStatsPath:GetAttribute("RoundStarted")
    local timer = gameStatsPath:GetAttribute("Timer")
    return roundStarted == false and timer == 3
end

local function getMatchingTables()
    local matched = {}
    for _, obj in pairs(getgc(true)) do
        if typeof(obj) == "table" then
            local ok = true
            for field in pairs(requiredFields) do
                if rawget(obj, field) == nil then
                    ok = false
                    break
                end
            end
            if ok then
                table.insert(matched, obj)
            end
        end
    end
    return matched
end

local function applyToTables(callback)
    if not isPlayerModelPresent() then return end
    local targets = getMatchingTables()
    if #targets == 0 then return end
    
    if getgenv().ApplyMode == "Optimized" then
        task.spawn(function()
            for i, tableObj in ipairs(targets) do
                if tableObj and typeof(tableObj) == "table" then
                    pcall(callback, tableObj)
                end
                if i % 3 == 0 then task.wait() end
            end
        end)
    else
        for i, tableObj in ipairs(targets) do
            if tableObj and typeof(tableObj) == "table" then
                pcall(callback, tableObj)
            end
        end
    end
end

local function applyStoredSettings()
    local settings = {
        {field = "Speed", value = tonumber(currentSettings.Speed)},
        {field = "JumpCap", value = tonumber(currentSettings.JumpCap)},
        {field = "AirStrafeAcceleration", value = tonumber(currentSettings.AirStrafeAcceleration)}
    }
    for _, setting in ipairs(settings) do
        if setting.value and tostring(setting.value) ~= "1500" and tostring(setting.value) ~= "1" and tostring(setting.value) ~= "187" then
            applyToTables(function(obj) obj[setting.field] = setting.value end)
        end
    end
end

local function applySettingsWithDelay()
    if not shouldApplySettings() or appliedOnce then return end
    appliedOnce = true
    local settings = {
        {field = "Speed", value = tonumber(currentSettings.Speed), delay = math.random(1, 14)},
        {field = "JumpCap", value = tonumber(currentSettings.JumpCap), delay = math.random(1, 14)},
        {field = "AirStrafeAcceleration", value = tonumber(currentSettings.AirStrafeAcceleration), delay = math.random(1, 14)}
    }
    for _, setting in ipairs(settings) do
        if setting.value and tostring(setting.value) ~= "1500" and tostring(setting.value) ~= "1" and tostring(setting.value) ~= "187" then
            task.spawn(function()
                task.wait(setting.delay)
                applyToTables(function(obj) obj[setting.field] = setting.value end)
            end)
        end
    end
end

local roundStartedConnection, timerConnection
local function setupAttributeConnections()
    if roundStartedConnection then roundStartedConnection:Disconnect() end
    if timerConnection then timerConnection:Disconnect() end
    if gameStatsPath then
        roundStartedConnection = gameStatsPath:GetAttributeChangedSignal("RoundStarted"):Connect(function()
            if gameStatsPath:GetAttribute("RoundStarted") == true then appliedOnce = false end
        end)
        timerConnection = gameStatsPath:GetAttributeChangedSignal("Timer"):Connect(function()
            if shouldApplySettings() and not appliedOnce then applySettingsWithDelay() end
        end)
    end
end
setupAttributeConnections()

task.spawn(function()
    while true do
        task.wait(0.5)
        if playersPath then
            local currentlyPresent = isPlayerModelPresent()
            if currentlyPresent and not playerModelPresent then
                playerModelPresent = true
                applyStoredSettings()
            elseif not currentlyPresent and playerModelPresent then
                playerModelPresent = false
            end
        end
    end
end)

local function createValidatedInput(config)
    return function(input)
        local val = tonumber(input)
        if not val then return end
        if config.min and val < config.min then return end
        if config.max and val > config.max then return end
        currentSettings[config.field] = input
        applyToTables(function(obj) obj[config.field] = val end)
    end
end

MainTab:Input({
    Title = "Speed",
    Placeholder = "1500",
    Value = "1500",
    Callback = createValidatedInput({ field = "Speed", min = 1450, max = 100000000 })
})

MainTab:Input({
    Title = "Jump Cap",
    Placeholder = "1",
    Value = "1",
    Callback = createValidatedInput({ field = "JumpCap", min = 0.1, max = 5000000 })
})

MainTab:Input({
    Title = "Strafe speed",
    Placeholder = "187",
    Value = "187",
    Callback = createValidatedInput({ field = "AirStrafeAcceleration", min = 1, max = 1000000000 })
})

MainTab:Dropdown({
    Title = "Select Apply Method",
    Values = {"Unoptimized", "Optimized"},
    Multi = false,
    Default = "Unoptimized",
    Callback = function(value) getgenv().ApplyMode = value end,
})

MainTab:Space()

local originalEmoteSpeeds = {}
local itemsFolder = ReplicatedStorage:FindFirstChild("Items")
if itemsFolder then
    local emotesFolder = itemsFolder:FindFirstChild("Emotes")
    if emotesFolder then
        for _, module in ipairs(emotesFolder:GetChildren()) do
            if module:IsA("ModuleScript") then
                local ok, data = pcall(require, module)
                if ok and data and data.EmoteInfo then
                    originalEmoteSpeeds[module.Name] = data.EmoteInfo.SpeedMult
                end
            end
        end
    end
end

local function applyEmoteSpeed(v)
    if not itemsFolder then return end
    local emotesFolder = itemsFolder:FindFirstChild("Emotes")
    if not emotesFolder then return end
    for _, module in ipairs(emotesFolder:GetChildren()) do
        if module:IsA("ModuleScript") then
            local ok, data = pcall(require, module)
            if ok and data and data.EmoteInfo and data.EmoteInfo.SpeedMult ~= 0 then
                data.EmoteInfo.SpeedMult = v
            end
        end
    end
end

local function restoreOriginalEmoteSpeed()
    if not itemsFolder then return end
    local emotesFolder = itemsFolder:FindFirstChild("Emotes")
    if not emotesFolder then return end
    for _, module in ipairs(emotesFolder:GetChildren()) do
        if module:IsA("ModuleScript") then
            local original = originalEmoteSpeeds[module.Name]
            if original then
                local ok, data = pcall(require, module)
                if ok and data and data.EmoteInfo then
                    data.EmoteInfo.SpeedMult = original
                end
            end
        end
    end
end

getgenv().featureStates.EmoteSpeedValue = 2

MainTab:Input({
    Title = "Emote Speed Value",
    Description = "Changes the animation speed of your emotes",
    Placeholder = "1500",
    NumbersOnly = true,
    Callback = function(value)
        local num = tonumber(value)
        if not num or num <= 0 then return end
        getgenv().featureStates.EmoteSpeedValue = num
        applyEmoteSpeed(num / 1000)
    end
})

MainTab:Button({
    Title = "Reset Emote Speed",
    Description = "Restore default emote speed",
    Callback = function() restoreOriginalEmoteSpeed() end
})

MainTab:Space()

local CachedTables = {}
local PlrModel = nil
local SlideFrictionValue = -8
local HeartbeatConnection = nil
local SlideEnabled = false
local SlideCharacterAddedConnection = nil

local function RefreshCachedTables()
    local newCachedTables = {}
    for _, obj in ipairs(getgc(true)) do
        if type(obj) == "table" and rawget(obj, "Friction") and rawget(obj, "Speed") then
            table.insert(newCachedTables, obj)
        end
    end
    CachedTables = newCachedTables
end

local function UpdatePlayerModel()
    local GameFolder = workspace:FindFirstChild("Game")
    local PlayersFolder = GameFolder and GameFolder:FindFirstChild("Players")
    PlrModel = PlayersFolder and PlayersFolder:FindFirstChild(LocalPlayer.Name) or nil
end

local function SetFriction(value)
    if #CachedTables == 0 then RefreshCachedTables() end
    for _, t in ipairs(CachedTables) do pcall(function() t.Friction = value end) end
end

local function OnHeartbeat()
    if not PlrModel then SetFriction(5) return end
    local currentState = PlrModel:GetAttribute("State")
    if currentState == "Slide" then
        PlrModel:SetAttribute("State", "EmotingSlide")
    elseif currentState == "EmotingSlide" then
        SetFriction(SlideFrictionValue)
    else
        SetFriction(5)
    end
end

local function StartInfiniteSlide()
    SlideEnabled = true
    RefreshCachedTables()
    UpdatePlayerModel()
    if HeartbeatConnection then HeartbeatConnection:Disconnect() end
    HeartbeatConnection = RunService.Heartbeat:Connect(OnHeartbeat)
    if SlideCharacterAddedConnection then SlideCharacterAddedConnection:Disconnect() end
    SlideCharacterAddedConnection = LocalPlayer.CharacterAdded:Connect(function()
        task.wait(1)
        RefreshCachedTables()
        UpdatePlayerModel()
    end)
end

local function StopInfiniteSlide()
    SlideEnabled = false
    SetFriction(5)
    if HeartbeatConnection then HeartbeatConnection:Disconnect() HeartbeatConnection = nil end
    if SlideCharacterAddedConnection then SlideCharacterAddedConnection:Disconnect() SlideCharacterAddedConnection = nil end
    PlrModel = nil
    CachedTables = {}
end

MainTab:Toggle({
    Title = "Infinite Slide",
    Value = false,
    Callback = function(state)
        if state then StartInfiniteSlide() else StopInfiniteSlide() end
    end
})

MainTab:Input({
    Title = "Slide Friction",
    Desc = "Negative values make slide faster (recommended: -8)",
    Placeholder = "-8",
    NumbersOnly = true,
    Value = "-8",
    Callback = function(value)
        local num = tonumber(value)
        if num and num < 0 then
            SlideFrictionValue = num
            if SlideEnabled and PlrModel and PlrModel:GetAttribute("State") == "EmotingSlide" then
                SetFriction(SlideFrictionValue)
            end
        end
    end
})

MainTab:Space()

local autoJumpType = "Bounce"
getgenv().AutoJumpEnabled = false
local jumpCooldown = 0
local Character, Humanoid, HumanoidRootPart = nil, nil, nil
local LastJump = 0
local GROUND_CHECK_DISTANCE = 4
local MAX_SLOPE_ANGLE = 45
local autoJumpGUI = nil

local function IsOnGround()
    if not Character or not HumanoidRootPart or not Humanoid then return false end
    local state = Humanoid:GetState()
    if state == Enum.HumanoidStateType.Jumping or state == Enum.HumanoidStateType.Freefall or state == Enum.HumanoidStateType.Swimming then return false end
    local raycastParams = RaycastParams.new()
    raycastParams.FilterType = Enum.RaycastFilterType.Blacklist
    raycastParams.FilterDescendantsInstances = {Character}
    raycastParams.IgnoreWater = true
    local raycastResult = workspace:Raycast(HumanoidRootPart.Position, Vector3.new(0, -GROUND_CHECK_DISTANCE, 0), raycastParams)
    if not raycastResult then return false end
    local angle = math.deg(math.acos(raycastResult.Normal:Dot(Vector3.new(0, 1, 0))))
    return angle <= MAX_SLOPE_ANGLE
end

local function updateBhop()
    if getgenv().AutoJumpEnabled and Humanoid then
        local now = tick()
        if IsOnGround() and (now - LastJump) > jumpCooldown then
            if autoJumpType == "Realistic" then
                pcall(function()
                    LocalPlayer.PlayerScripts.Events.temporary_events.JumpReact:Fire()
                    task.wait(0.05)
                    LocalPlayer.PlayerScripts.Events.temporary_events.EndJump:Fire()
                end)
            else
                Humanoid:ChangeState(Enum.HumanoidStateType.Jumping)
            end
            LastJump = now
        end
    end
end

LocalPlayer.CharacterAdded:Connect(function(character)
    Character = character
    Humanoid = character:WaitForChild("Humanoid")
    HumanoidRootPart = character:WaitForChild("HumanoidRootPart")
end)

if LocalPlayer.Character then
    Character = LocalPlayer.Character
    Humanoid = Character:WaitForChild("Humanoid")
    HumanoidRootPart = Character:WaitForChild("HumanoidRootPart")
end

getgenv().bhopConnection = RunService.Heartbeat:Connect(updateBhop)

local function createAutoJumpGUI()
    if autoJumpGUI and autoJumpGUI.Parent then autoJumpGUI:Destroy() autoJumpGUI = nil end
    local gui = Instance.new("ScreenGui")
    gui.Name = "AutoJumpGUI"
    gui.ResetOnSpawn = false
    gui.Parent = PlayerGui
    gui.Enabled = false
    
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0, 220, 0, 44)
    btn.Position = getgenv().AutoJumpPosition or UDim2.new(0.5, -110, 0, 60)
    btn.AnchorPoint = Vector2.new(0.5, 0)
    btn.Text = "AutoJump [OFF]"
    btn.Font = Enum.Font.Gotham
    btn.TextSize = 20
    btn.TextColor3 = Color3.new(1,1,1)
    btn.AutoButtonColor = false
    btn.BackgroundTransparency = 0.15
    btn.Parent = gui
    
    local gradient = Instance.new("UIGradient", btn)
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    
    Instance.new("UICorner", btn).CornerRadius = UDim.new(1,0)
    Instance.new("UIStroke", btn).Thickness = 1.5
    btn.UIStroke.Color = Color3.new(1,1,1)
    btn.UIStroke.Transparency = 0.3
    
    local dragging = false
    local dragStart, startPos
    
    btn.MouseEnter:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    btn.MouseLeave:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)
    
    btn.MouseButton1Click:Connect(function()
        if not dragging then
            getgenv().AutoJumpEnabled = not getgenv().AutoJumpEnabled
            btn.Text = "AutoJump ["..(getgenv().AutoJumpEnabled and "ON" or "OFF").."]"
        end
    end)
    
    local function updateDrag(input)
        local delta = input.Position - dragStart
        btn.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
    
    btn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = btn.Position
            TweenService:Create(btn, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv().AutoJumpPosition = btn.Position
                    TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)
    
    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)
    
    gui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    return gui
end

MainTab:Dropdown({
    Title = "AutoJump Mode",
    Values = {"Bounce", "Realistic"},
    Multi = false,
    Default = 1,
    Callback = function(value) if value ~= "" then autoJumpType = value end end
})

MainTab:Toggle({
    Title = "AutoJump",
    Value = false,
    Callback = function(state)
        if state then
            if not autoJumpGUI or not autoJumpGUI.Parent then autoJumpGUI = createAutoJumpGUI() end
            autoJumpGUI.Enabled = true
        else
            getgenv().AutoJumpEnabled = false
            if autoJumpGUI and autoJumpGUI.Parent then autoJumpGUI.Enabled = false end
        end
    end
})

MainTab:Input({
    Title = "AutoJump Acceleration (Negative Only)",
    Placeholder = "-0.5",
    Numeric = true,
    Callback = function(value)
        if tostring(value):sub(1,1) == "-" then
            getgenv().AutoJumpAccel = tonumber(value)
            jumpCooldown = tonumber(value)
        end
    end
})

MainTab:Space()

getgenv().GravityEnabled = false
getgenv().GravityValue = 196.2
getgenv().GravityPosition = getgenv().GravityPosition or UDim2.new(0.5, -110, 0, 60)

local function createGravityGUI()
    if PlayerGui:FindFirstChild("GravityGUI") then PlayerGui.GravityGUI:Destroy() end
    local gui = Instance.new("ScreenGui")
    gui.Name = "GravityGUI"
    gui.ResetOnSpawn = false
    gui.Parent = PlayerGui
    gui.Enabled = false
    
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0, 220, 0, 44)
    btn.Position = getgenv().GravityPosition
    btn.AnchorPoint = Vector2.new(0.5, 0)
    btn.Text = "Gravitasi [OFF]"
    btn.Font = Enum.Font.Gotham
    btn.TextSize = 20
    btn.TextColor3 = Color3.new(1,1,1)
    btn.AutoButtonColor = false
    btn.BackgroundTransparency = 0.15
    btn.Parent = gui
    
    local gradient = Instance.new("UIGradient", btn)
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    
    Instance.new("UICorner", btn).CornerRadius = UDim.new(1,0)
    Instance.new("UIStroke", btn).Thickness = 1.5
    btn.UIStroke.Color = Color3.new(1,1,1)
    btn.UIStroke.Transparency = 0.3
    
    local dragging = false
    local dragStart, startPos
    
    btn.MouseEnter:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    btn.MouseLeave:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)
    
    btn.MouseButton1Click:Connect(function()
        if not dragging then
            getgenv().GravityEnabled = not getgenv().GravityEnabled
            workspace.Gravity = getgenv().GravityEnabled and getgenv().GravityValue or 196.2
            btn.Text = "Gravitasi ["..(getgenv().GravityEnabled and "ON" or "OFF").."]"
        end
    end)
    
    local function updateDrag(input)
        local delta = input.Position - dragStart
        btn.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
    
    btn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = btn.Position
            TweenService:Create(btn, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv().GravityPosition = btn.Position
                    TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)
    
    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)
    
    gui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    return gui
end

local GravityGUI = nil

MainTab:Toggle({
    Title = "Gravity GUI",
    Value = false,
    Callback = function(state)
        if state then
            if not GravityGUI or not GravityGUI.Parent then GravityGUI = createGravityGUI() end
            GravityGUI.Enabled = true
        else
            if GravityGUI then
                GravityGUI.Enabled = false
                if getgenv().GravityEnabled then
                    getgenv().GravityEnabled = false
                    workspace.Gravity = 196.2
                end
            end
        end
    end
})

MainTab:Input({
    Title = "Gravity Value",
    Placeholder = tostring(getgenv().GravityValue),
    Value = tostring(getgenv().GravityValue),
    Callback = function(val)
        local n = tonumber(val)
        if n and n > 0 then
            getgenv().GravityValue = n
            if getgenv().GravityEnabled then workspace.Gravity = n end
        end
    end
})

MainTab:Space()

getgenv().BackJumpEnabled = false
getgenv().BackJumpSpeed = 50
getgenv().BackJumpPosition = getgenv().BackJumpPosition or UDim2.new(0.5, -110, 0, 10)

local maxExtraSpeed = 100
local minSpeedOffset = 0
local lastTick = tick()
local airAccumulator = 0
local airTotalTime = 0
local wasAir = false
local activeBV = nil
local currentSpeed = getgenv().BackJumpSpeed
local countingEnabled = false
local speedometer = nil
local backJumpGUI = nil

local function truncate1Decimal(val) return math.floor(val * 10) / 10 end

local function getSpeedometer()
    local ok, spd = pcall(function() return PlayerGui.Shared.HUD.Overlay.Default.CharacterInfo.Item.Speedometer.Players end)
    return ok and spd or nil
end

local oldNewIndex
if not oldNewIndex then
    oldNewIndex = hookmetamethod(game, "__newindex", function(self, idx, val)
        if not checkcaller() and countingEnabled and speedometer and self == speedometer and idx == "Text" then
            return
        end
        return oldNewIndex(self, idx, val)
    end)
end

local function createBackJumpGUI()
    if PlayerGui:FindFirstChild("BackJumpGUI") then PlayerGui.BackJumpGUI:Destroy() end
    local gui = Instance.new("ScreenGui")
    gui.Name = "BackJumpGUI"
    gui.ResetOnSpawn = false
    gui.Parent = PlayerGui
    gui.Enabled = true
    
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0, 220, 0, 44)
    btn.Position = getgenv().BackJumpPosition
    btn.AnchorPoint = Vector2.new(0.5, 0)
    btn.Text = "Back Jump [OFF]"
    btn.Font = Enum.Font.Gotham
    btn.TextSize = 20
    btn.TextColor3 = Color3.new(1,1,1)
    btn.AutoButtonColor = false
    btn.BackgroundTransparency = 0.15
    btn.Parent = gui
    
    local gradient = Instance.new("UIGradient", btn)
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    
    Instance.new("UICorner", btn).CornerRadius = UDim.new(1,0)
    Instance.new("UIStroke", btn).Thickness = 1.5
    btn.UIStroke.Color = Color3.new(1,1,1)
    btn.UIStroke.Transparency = 0.3
    
    local dragging = false
    local dragStart, startPos
    
    btn.MouseEnter:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    btn.MouseLeave:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)
    
    btn.MouseButton1Click:Connect(function()
        if not dragging then
            getgenv().BackJumpEnabled = not getgenv().BackJumpEnabled
            btn.Text = "Back Jump ["..(getgenv().BackJumpEnabled and "ON" or "OFF").."]"
            if not getgenv().BackJumpEnabled then
                if activeBV then activeBV:Destroy() activeBV = nil end
                currentSpeed = getgenv().BackJumpSpeed
                countingEnabled = false
                airAccumulator = 0
                airTotalTime = 0
                wasAir = false
            end
        end
    end)
    
    local function updateDrag(input)
        local delta = input.Position - dragStart
        btn.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
    
    btn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = btn.Position
            TweenService:Create(btn, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv().BackJumpPosition = btn.Position
                    TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)
    
    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)
    
    gui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    return gui
end

RunService.RenderStepped:Connect(function()
    local deltaTime = tick() - lastTick
    lastTick = tick()
    local char = LocalPlayer.Character
    if char then
        local root = char:FindFirstChild("HumanoidRootPart")
        local humanoid = char:FindFirstChild("Humanoid")
        if root and humanoid then
            speedometer = getSpeedometer()
            local isAir = humanoid.FloorMaterial == Enum.Material.Air
            if wasAir and not isAir then
                currentSpeed = math.max(getgenv().BackJumpSpeed - minSpeedOffset, currentSpeed - 10)
                if speedometer then speedometer.Text = tostring(truncate1Decimal(currentSpeed)) end
                airTotalTime = 0
            end
            wasAir = isAir
            if getgenv().BackJumpEnabled then
                if isAir then
                    airAccumulator = airAccumulator + deltaTime
                    airTotalTime = airTotalTime + deltaTime
                    while airAccumulator >= 0.04 do
                        airAccumulator = airAccumulator - 0.04
                        currentSpeed = math.min(getgenv().BackJumpSpeed + maxExtraSpeed, currentSpeed + math.max(0.1, 2.5 * (0.04 / 1)))
                    end
                else
                    airAccumulator = 0
                    currentSpeed = math.max(getgenv().BackJumpSpeed - minSpeedOffset, currentSpeed - 4 * deltaTime)
                    airTotalTime = 0
                end
                if activeBV then activeBV:Destroy() end
                local lookDir = Vector3.new(-camera.CFrame.LookVector.X, 0, -camera.CFrame.LookVector.Z)
                if lookDir.Magnitude ~= 0 then lookDir = lookDir.Unit end
                local bv = Instance.new("BodyVelocity")
                bv.Velocity = lookDir * currentSpeed
                bv.MaxForce = Vector3.new(4e5, 0, 4e5)
                bv.P = 1250
                bv.Parent = root
                Debris:AddItem(bv, 0.1)
                activeBV = bv
                countingEnabled = true
                if speedometer then speedometer.Text = tostring(truncate1Decimal(currentSpeed)) end
            else
                if activeBV then activeBV:Destroy() activeBV = nil end
                currentSpeed = getgenv().BackJumpSpeed
                countingEnabled = false
                airAccumulator = 0
                airTotalTime = 0
                wasAir = false
            end
        end
    end
end)

MainTab:Toggle({
    Title = "Back Jump",
    Value = false,
    Callback = function(state)
        getgenv().BackJumpEnabled = state
        if state then
            if not backJumpGUI then backJumpGUI = createBackJumpGUI() end
        else
            if backJumpGUI then backJumpGUI:Destroy() backJumpGUI = nil end
            if activeBV then activeBV:Destroy() activeBV = nil end
            currentSpeed = getgenv().BackJumpSpeed
            countingEnabled = false
            airAccumulator = 0
            airTotalTime = 0
            wasAir = false
        end
    end
})

MainTab:Input({
    Title = "Back Jump Speed",
    Icon = "zap",
    Placeholder = "Default 50",
    Value = "50",
    Callback = function(input)
        local val = tonumber(input)
        if val and val > 0 and val <= 500 then
            getgenv().BackJumpSpeed = val
            currentSpeed = val
        end
    end
})

MainTab:Space()

getgenv().FrontJumpEnabled = false
getgenv().FrontJumpSpeed = 50
getgenv().FrontJumpPosition = getgenv().FrontJumpPosition or UDim2.new(0.5, -110, 0, 60)

local maxExtraSpeedFront = 100
local minSpeedOffsetFront = 0
local lastTickFront = tick()
local airAccumulatorFront = 0
local airTotalTimeFront = 0
local wasAirFront = false
local activeBVFront = nil
local currentSpeedFront = getgenv().FrontJumpSpeed
local countingEnabledFront = false
local speedometerFront = nil
local frontJumpGUI = nil

local function truncate1DecimalFront(val) return math.floor(val * 10) / 10 end
local function getSpeedometerFront() return getSpeedometer() end

local oldNewIndexFront
if not oldNewIndexFront then
    oldNewIndexFront = hookmetamethod(game, "__newindex", function(self, idx, val)
        if not checkcaller() and countingEnabledFront and speedometerFront and self == speedometerFront and idx == "Text" then
            return
        end
        return oldNewIndexFront(self, idx, val)
    end)
end

local function createFrontJumpGUI()
    if PlayerGui:FindFirstChild("FrontJumpGUI") then PlayerGui.FrontJumpGUI:Destroy() end
    local gui = Instance.new("ScreenGui")
    gui.Name = "FrontJumpGUI"
    gui.ResetOnSpawn = false
    gui.Parent = PlayerGui
    gui.Enabled = true
    
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0, 220, 0, 44)
    btn.Position = getgenv().FrontJumpPosition
    btn.AnchorPoint = Vector2.new(0.5, 0)
    btn.Text = "Front Jump [OFF]"
    btn.Font = Enum.Font.Gotham
    btn.TextSize = 20
    btn.TextColor3 = Color3.new(1,1,1)
    btn.AutoButtonColor = false
    btn.BackgroundTransparency = 0.15
    btn.Parent = gui
    
    local gradient = Instance.new("UIGradient", btn)
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    
    Instance.new("UICorner", btn).CornerRadius = UDim.new(1,0)
    Instance.new("UIStroke", btn).Thickness = 1.5
    btn.UIStroke.Color = Color3.new(1,1,1)
    btn.UIStroke.Transparency = 0.3
    
    local dragging = false
    local dragStart, startPos
    
    btn.MouseEnter:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    btn.MouseLeave:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)
    
    btn.MouseButton1Click:Connect(function()
        if not dragging then
            getgenv().FrontJumpEnabled = not getgenv().FrontJumpEnabled
            btn.Text = "Front Jump ["..(getgenv().FrontJumpEnabled and "ON" or "OFF").."]"
            if not getgenv().FrontJumpEnabled then
                if activeBVFront then activeBVFront:Destroy() activeBVFront = nil end
                currentSpeedFront = getgenv().FrontJumpSpeed
                countingEnabledFront = false
                airAccumulatorFront = 0
                airTotalTimeFront = 0
                wasAirFront = false
            end
        end
    end)
    
    local function updateDrag(input)
        local delta = input.Position - dragStart
        btn.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
    
    btn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = btn.Position
            TweenService:Create(btn, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv().FrontJumpPosition = btn.Position
                    TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)
    
    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)
    
    gui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    return gui
end

RunService.RenderStepped:Connect(function()
    local deltaTime = tick() - lastTickFront
    lastTickFront = tick()
    local char = LocalPlayer.Character
    if char then
        local root = char:FindFirstChild("HumanoidRootPart")
        local humanoid = char:FindFirstChild("Humanoid")
        if root and humanoid then
            speedometerFront = getSpeedometerFront()
            local isAir = humanoid.FloorMaterial == Enum.Material.Air
            if wasAirFront and not isAir then
                currentSpeedFront = math.max(getgenv().FrontJumpSpeed - minSpeedOffsetFront, currentSpeedFront - 10)
                if speedometerFront then speedometerFront.Text = tostring(truncate1DecimalFront(currentSpeedFront)) end
                airTotalTimeFront = 0
            end
            wasAirFront = isAir
            if getgenv().FrontJumpEnabled then
                if isAir then
                    airAccumulatorFront = airAccumulatorFront + deltaTime
                    airTotalTimeFront = airTotalTimeFront + deltaTime
                    while airAccumulatorFront >= 0.04 do
                        airAccumulatorFront = airAccumulatorFront - 0.04
                        currentSpeedFront = math.min(getgenv().FrontJumpSpeed + maxExtraSpeedFront, currentSpeedFront + math.max(0.1, 2.5 * (0.04 / 1)))
                    end
                else
                    airAccumulatorFront = 0
                    currentSpeedFront = math.max(getgenv().FrontJumpSpeed - minSpeedOffsetFront, currentSpeedFront - 4 * deltaTime)
                    airTotalTimeFront = 0
                end
                if activeBVFront then activeBVFront:Destroy() end
                local lookDir = Vector3.new(camera.CFrame.LookVector.X, 0, camera.CFrame.LookVector.Z)
                if lookDir.Magnitude ~= 0 then lookDir = lookDir.Unit end
                local bv = Instance.new("BodyVelocity")
                bv.Velocity = lookDir * currentSpeedFront
                bv.MaxForce = Vector3.new(4e5, 0, 4e5)
                bv.P = 1250
                bv.Parent = root
                Debris:AddItem(bv, 0.1)
                activeBVFront = bv
                countingEnabledFront = true
                if speedometerFront then speedometerFront.Text = tostring(truncate1DecimalFront(currentSpeedFront)) end
            else
                if activeBVFront then activeBVFront:Destroy() activeBVFront = nil end
                currentSpeedFront = getgenv().FrontJumpSpeed
                countingEnabledFront = false
                airAccumulatorFront = 0
                airTotalTimeFront = 0
                wasAirFront = false
            end
        end
    end
end)

MainTab:Toggle({
    Title = "Front Jump",
    Value = false,
    Callback = function(state)
        getgenv().FrontJumpEnabled = state
        if state then
            if not frontJumpGUI then frontJumpGUI = createFrontJumpGUI() end
        else
            if frontJumpGUI then frontJumpGUI:Destroy() frontJumpGUI = nil end
            if activeBVFront then activeBVFront:Destroy() activeBVFront = nil end
            currentSpeedFront = getgenv().FrontJumpSpeed
            countingEnabledFront = false
            airAccumulatorFront = 0
            airTotalTimeFront = 0
            wasAirFront = false
        end
    end
})

MainTab:Input({
    Title = "Front Jump Speed",
    Icon = "zap",
    Placeholder = "Default 50",
    Value = "50",
    Callback = function(input)
        local val = tonumber(input)
        if val and val > 0 and val <= 500 then
            getgenv().FrontJumpSpeed = val
            currentSpeedFront = val
        end
    end
})

MainTab:Space()

getgenv().LagSwitchDuration = getgenv().LagSwitchDuration or 0.5

local function triggerLagSwitch()
    task.spawn(function()
        local start = tick()
        local duration = getgenv().LagSwitchDuration
        while tick() - start < duration do
            for i = 1, 10000 do math.random(1, 1000000) end
        end
    end)
end

local function createLagSwitchGUI()
    if PlayerGui:FindFirstChild("LagSwitchGUI") then PlayerGui.LagSwitchGUI:Destroy() end
    local gui = Instance.new("ScreenGui")
    gui.Name = "LagSwitchGUI"
    gui.ResetOnSpawn = false
    gui.Parent = PlayerGui
    gui.Enabled = false
    
    local btn = Instance.new("TextButton")
    btn.Size = UDim2.new(0, 220, 0, 44)
    btn.Position = getgenv().LagSwitchPosition or UDim2.new(0.5, -110, 0, 120)
    btn.AnchorPoint = Vector2.new(0.5, 0)
    btn.Text = "Lag Switch"
    btn.Font = Enum.Font.Gotham
    btn.TextSize = 20
    btn.TextColor3 = Color3.new(1, 1, 1)
    btn.AutoButtonColor = false
    btn.BackgroundTransparency = 0.15
    btn.Parent = gui
    
    local gradient = Instance.new("UIGradient")
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    gradient.Parent = btn
    
    local uiCorner = Instance.new("UICorner")
    uiCorner.CornerRadius = UDim.new(1, 0)
    uiCorner.Parent = btn
    
    local stroke = Instance.new("UIStroke")
    stroke.Thickness = 1.5
    stroke.Color = Color3.new(1, 1, 1)
    stroke.Transparency = 0.3
    stroke.Parent = btn
    
    local dragging = false
    local dragStart, startPos
    
    btn.MouseEnter:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    btn.MouseLeave:Connect(function() 
        if not dragging then TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)
    
    btn.MouseButton1Click:Connect(function() if not dragging then triggerLagSwitch() end end)
    
    local function updateDrag(input)
        local delta = input.Position - dragStart
        btn.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
    
    btn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = btn.Position
            TweenService:Create(btn, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv().LagSwitchPosition = btn.Position
                    TweenService:Create(btn, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)
    
    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)
    
    gui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    return gui
end

local lagSwitchGUI

MainTab:Toggle({
    Title = "Lag Switch",
    Value = false,
    Callback = function(state)
        if state then
            if not lagSwitchGUI then lagSwitchGUI = createLagSwitchGUI() end
            lagSwitchGUI.Enabled = true
        else
            if lagSwitchGUI then lagSwitchGUI.Enabled = false end
        end
    end
})

MainTab:Input({
    Title = "Lag Duration (seconds)",
    Value = tostring(getgenv().LagSwitchDuration),
    Placeholder = "0.5",
    Callback = function(value)
        local num = tonumber(value)
        getgenv().LagSwitchDuration = (num and num > 0) and num or 0.5
    end
})

MainTab:Space()

local featureStates = { AutoSelfRevive = false, SelfReviveMethod = "Spawnpoint" }    
local lastSavedPosition = nil
local AutoSelfReviveConnection = nil
local respawnConnection = nil
local hasRevived = false    

local function doRevive(char)
    if not char then return end
    local hrp = char:FindFirstChild("HumanoidRootPart")
    local isDowned = char:GetAttribute("Downed")
    if not isDowned then return end

    if featureStates.SelfReviveMethod == "Spawnpoint" then
        if not hasRevived then
            hasRevived = true
            pcall(function() ReplicatedStorage.Events.Player.ChangePlayerMode:FireServer(true) end)
            task.delay(10, function() hasRevived = false end)
        end
    elseif featureStates.SelfReviveMethod == "Revive" then
        if hrp then lastSavedPosition = hrp.Position end
        task.spawn(function()
            task.wait(3)
            local startTime = tick()
            repeat
                pcall(function() ReplicatedStorage.Events.Player.ChangePlayerMode:FireServer(true) end)
                task.wait(1)
            until not char:GetAttribute("Downed") or (tick() - startTime > 1)
            local newChar
            repeat newChar = LocalPlayer.Character task.wait() until newChar and newChar:FindFirstChild("HumanoidRootPart")
            local newHRP = newChar:FindFirstChild("HumanoidRootPart")
            if lastSavedPosition and newHRP then
                newHRP.CFrame = CFrame.new(lastSavedPosition)
                task.wait(0.5)
                if (newHRP.Position - lastSavedPosition).Magnitude > 1 then lastSavedPosition = nil end
            end
        end)
    end    
end    

local function setupAutoRevive(char)
    if AutoSelfReviveConnection then AutoSelfReviveConnection:Disconnect() end
    AutoSelfReviveConnection = char:GetAttributeChangedSignal("Downed"):Connect(function()
        if char:GetAttribute("Downed") then doRevive(char) end
    end)
end    

if respawnConnection then respawnConnection:Disconnect() end
respawnConnection = LocalPlayer.CharacterAdded:Connect(function(newChar)
    task.wait(1)
    if featureStates.AutoSelfRevive then setupAutoRevive(newChar) end
end)

MainTab:Section({ Title = "Yourself" })
MainTab:Space()

MainTab:Dropdown({
    Title = "Respawn Method",
    Values = {"Spawnpoint", "Revive"},
    Value = "Spawnpoint",
    Callback = function(value) featureStates.SelfReviveMethod = value end
})    

MainTab:Button({
    Title = "Respawn",
    Callback = function() doRevive(LocalPlayer.Character) end
})    

MainTab:Toggle({
    Title = "Auto Self Revive",
    Value = false,
    Callback = function(state)
        featureStates.AutoSelfRevive = state
        if state and LocalPlayer.Character then setupAutoRevive(LocalPlayer.Character) end
    end
})

if LocalPlayer.Character and featureStates.AutoSelfRevive then setupAutoRevive(LocalPlayer.Character) end

MainTab:Space()

local INTERACT_REMOTE = ReplicatedStorage:WaitForChild("Events"):WaitForChild("Character"):WaitForChild("Interact")

getgenv().AutoCarryEnabled = false
getgenv().AutoReviveEnabled = false
getgenv().AutoCarryDelay = 1
getgenv().AutoReviveDelay = 1
getgenv().AutoCarryPosition = UDim2.new(0.5, -110, 0, 50)
getgenv().AutoRevivePosition = UDim2.new(0.5, -110, 0, 120)

local function CreateFloatingButton(name, enabledFlag, savedPosFlag, defaultPosY)
    if PlayerGui:FindFirstChild(name.."GUI") then PlayerGui[name.."GUI"]:Destroy() end
    local screenGui = Instance.new("ScreenGui")
    screenGui.Name = name.."GUI"
    screenGui.ResetOnSpawn = false
    screenGui.Parent = PlayerGui

    local Button = Instance.new("TextButton")
    Button.Size = UDim2.new(0, 220, 0, 44)
    Button.Position = getgenv()[savedPosFlag] or UDim2.new(0.5, -110, 0, defaultPosY)
    Button.AnchorPoint = Vector2.new(0.5, 0)
    Button.Text = name.." [OFF]"
    Button.Font = Enum.Font.Gotham
    Button.TextSize = 20
    Button.TextColor3 = Color3.new(1,1,1)
    Button.AutoButtonColor = false
    Button.BackgroundTransparency = 0.15
    Button.Parent = screenGui
    
    local gradient = Instance.new("UIGradient", Button)
    gradient.Color = ColorSequence.new({
        ColorSequenceKeypoint.new(0, Color3.fromRGB(64, 224, 208)),
        ColorSequenceKeypoint.new(0.5, Color3.fromRGB(255, 255, 255)),
        ColorSequenceKeypoint.new(1, Color3.fromRGB(100, 149, 237))
    })
    gradient.Rotation = 45
    
    Instance.new("UICorner", Button).CornerRadius = UDim.new(1,0)
    Instance.new("UIStroke", Button).Thickness = 1.5
    Button.UIStroke.Color = Color3.new(1,1,1)
    Button.UIStroke.Transparency = 0.3

    local dragging = false

    Button.MouseEnter:Connect(function()
        if not dragging then TweenService:Create(Button, TweenInfo.new(0.2), {BackgroundTransparency = 0.3}):Play() end
    end)
    Button.MouseLeave:Connect(function()
        if not dragging then TweenService:Create(Button, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play() end
    end)

    Button.MouseButton1Click:Connect(function()
        if not dragging then
            getgenv()[enabledFlag] = not getgenv()[enabledFlag]
            Button.Text = name.." ["..(getgenv()[enabledFlag] and "ON" or "OFF").."]"
        end
    end)

    local dragStart, startPos

    local function updateDrag(input)
        local delta = input.Position - dragStart
        Button.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end

    Button.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            dragging = true
            dragStart = input.Position
            startPos = Button.Position
            TweenService:Create(Button, TweenInfo.new(0.1), {BackgroundTransparency = 0.4}):Play()
            local conn = input.Changed:Connect(function()
                if input.UserInputState == Enum.UserInputState.End then
                    dragging = false
                    getgenv()[savedPosFlag] = Button.Position
                    TweenService:Create(Button, TweenInfo.new(0.2), {BackgroundTransparency = 0.15}):Play()
                    conn:Disconnect()
                end
            end)
        end
    end)

    local inputChangedConn = UserInputService.InputChanged:Connect(function(input)
        if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            updateDrag(input)
        end
    end)

    screenGui.Destroying:Connect(function() if inputChangedConn then inputChangedConn:Disconnect() end end)
    screenGui.Enabled = false
    return screenGui
end

local AutoCarryGUI = CreateFloatingButton("AutoCarry", "AutoCarryEnabled", "AutoCarryPosition", 50)
local AutoReviveGUI = CreateFloatingButton("AutoRevive", "AutoReviveEnabled", "AutoRevivePosition", 120)

RunService.RenderStepped:Connect(function()
    if getgenv().AutoCarryEnabled then
        local myHRP = LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
        if myHRP then
            for _, player in ipairs(Players:GetPlayers()) do
                if player ~= LocalPlayer and player.Character and player.Character:FindFirstChild("HumanoidRootPart") and player.Character:FindFirstChild("Humanoid") then
                    local targetHRP = player.Character.HumanoidRootPart
                    local targetHumanoid = player.Character.Humanoid
                    if (myHRP.Position - targetHRP.Position).Magnitude <= 10 and targetHumanoid.Health <= 0 then
                        if tick() - (getgenv().AutoCarryLast or 0) >= getgenv().AutoCarryDelay then
                            INTERACT_REMOTE:FireServer("Carry", nil, player.Name)
                            getgenv().AutoCarryLast = tick()
                        end
                    end
                end
            end
        end
    end

    if getgenv().AutoReviveEnabled then
        local myHRP = LocalPlayer.Character and LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
        if myHRP then
            for _, player in ipairs(Players:GetPlayers()) do
                if player ~= LocalPlayer and player.Character and player.Character:FindFirstChild("HumanoidRootPart") and player.Character:FindFirstChild("Humanoid") then
                    local targetHRP = player.Character.HumanoidRootPart
                    local targetHumanoid = player.Character.Humanoid
                    if (myHRP.Position - targetHRP.Position).Magnitude <= 10 and targetHumanoid.Health <= 0 then
                        if tick() - (getgenv().AutoReviveLast or 0) >= getgenv().AutoReviveDelay then
                            INTERACT_REMOTE:FireServer("Revive", true, player.Name)
                            getgenv().AutoReviveLast = tick()
                        end
                    end
                end
            end
        end
    end
end)

MainTab:Section({ Title = "Interactions" })
MainTab:Space()

MainTab:Toggle({
    Title = "Auto Carry GUI",
    Value = false,
    Callback = function(state)
        AutoCarryGUI.Enabled = state
        if not state then getgenv().AutoCarryEnabled = false end
    end
})

MainTab:Input({
    Title = "Carry Delay (s)",
    Placeholder = "1",
    Value = tostring(getgenv().AutoCarryDelay),
    Callback = function(val)
        local n = tonumber(val)
        if n and n > 0 then getgenv().AutoCarryDelay = n end
    end
})

MainTab:Space()

MainTab:Toggle({
    Title = "Auto Revive GUI",
    Value = false,
    Callback = function(state)
        AutoReviveGUI.Enabled = state
        if not state then getgenv().AutoReviveEnabled = false end
    end
})

MainTab:Input({
    Title = "Revive Delay (s)",
    Placeholder = "1",
    Value = tostring(getgenv().AutoReviveDelay),
    Callback = function(val)
        local n = tonumber(val)
        if n and n > 0 then getgenv().AutoReviveDelay = n end
    end
})

local function createESP(part, name, baseColor)
    if getgenv().espObjects[part] then return getgenv().espObjects[part] end
    local billboard = Instance.new("BillboardGui")
    billboard.Name = name .. "ESP"
    billboard.Adornee = part
    billboard.Size = UDim2.new(0, 180, 0, 40)
    billboard.StudsOffset = Vector3.new(0, 3.5, 0)
    billboard.AlwaysOnTop = true
    billboard.LightInfluence = 0
    billboard.MaxDistance = 500
    billboard.Parent = PlayerGui
    local label = Instance.new("TextLabel")
    label.Name = "Label"
    label.Size = UDim2.new(1, 0, 1, 0)
    label.BackgroundTransparency = 1
    label.TextStrokeTransparency = 0.25
    label.TextSize = 14
    label.Font = Enum.Font.GothamSemibold
    label.Text = ""
    label.TextColor3 = baseColor or Color3.fromRGB(255, 255, 255)
    label.Parent = billboard
    getgenv().espObjects[part] = {billboard = billboard, label = label, type = name}
    return getgenv().espObjects[part]
end

local function removeESP(part)
    if getgenv().espObjects[part] then
        getgenv().espObjects[part].billboard:Destroy()
        getgenv().espObjects[part] = nil
    end
end

local function cleanupESP()
    for part, data in pairs(getgenv().espObjects) do
        if data and data.billboard then data.billboard:Destroy() end
    end
    getgenv().espObjects = {}
    for name, thread in pairs(getgenv().espThreads) do
        if thread then coroutine.close(thread) getgenv().espThreads[name] = nil end
    end
end

EspTab:Toggle({
    Title = "ESP Player",
    Value = false,
    Callback = function(state)
        getgenv().featureStates.EspPlayer = state
        if getgenv().espThreads["player"] then coroutine.close(getgenv().espThreads["player"]) getgenv().espThreads["player"] = nil end
        for part, data in pairs(getgenv().espObjects) do if data and data.type == "Player" then removeESP(part) end end
        if not state then return end
        getgenv().espThreads["player"] = coroutine.create(function()
            local function getDistance(pos)
                local char = LocalPlayer.Character
                local hrp = char and char:FindFirstChild("HumanoidRootPart")
                return hrp and (pos - hrp.Position).Magnitude or 0
            end
            while getgenv().featureStates.EspPlayer do
                local folder = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Players")
                if folder then
                    for _, char in ipairs(folder:GetChildren()) do
                        if not char:IsA("Model") then continue end
                        if char:GetAttribute("Team") == "Nextbot" then continue end
                        if char.Name == LocalPlayer.Name then continue end
                        local hrp = char:FindFirstChild("HumanoidRootPart")
                        if not hrp then continue end
                        local espData = getgenv().espObjects[hrp]
                        if not espData then espData = createESP(hrp, "Player", Color3.fromRGB(100, 180, 255)) end
                        if not espData or not espData.billboard or not espData.billboard.Parent then getgenv().espObjects[hrp] = nil continue end
                        local dist = getDistance(hrp.Position)
                        local downed = char:GetAttribute("Downed")
                        local downedTime = tonumber(char:GetAttribute("DownedTimeLeft")) or 0
                        local displayText, color
                        if downed == true then
                            color = Color3.fromRGB(255, 60, 60)
                            displayText = string.format('%s (Downed %.0f)', char.Name, downedTime)
                        elseif downed == false then
                            color = Color3.fromRGB(100, 255, 100)
                            displayText = string.format('%s\n%.0f studs', char.Name, dist)
                        else
                            color = Color3.fromRGB(100, 180, 255)
                            displayText = string.format('%s\n%.0f studs', char.Name, dist)
                        end
                        espData.label.Text = displayText
                        espData.label.TextColor3 = color
                    end
                end
                task.wait(0.3)
            end
        end)
        coroutine.resume(getgenv().espThreads["player"])
    end
})

EspTab:Toggle({
    Title = "ESP Ticket",
    Value = false,
    Callback = function(state)
        getgenv().featureStates.EspTicket = state
        if getgenv().espThreads["ticket"] then coroutine.close(getgenv().espThreads["ticket"]) getgenv().espThreads["ticket"] = nil end
        for part, data in pairs(getgenv().espObjects) do if data and data.type == "Ticket" then removeESP(part) end end
        if not state then return end
        getgenv().espThreads["ticket"] = coroutine.create(function()
            local function getDistance(pos)
                local char = LocalPlayer.Character
                local hrp = char and char:FindFirstChild("HumanoidRootPart")
                return hrp and (pos - hrp.Position).Magnitude or 0
            end
            while getgenv().featureStates.EspTicket do
                local ticketFolder = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Effects") and workspace.Game.Effects:FindFirstChild("Tickets")
                if ticketFolder then
                    for _, ticketModel in ipairs(ticketFolder:GetChildren()) do
                        if not ticketModel:IsA("Model") then continue end
                        local part = ticketModel:FindFirstChildWhichIsA("BasePart")
                        if not part then continue end
                        local espData = getgenv().espObjects[part]
                        if not espData then espData = createESP(part, "Ticket", Color3.fromRGB(255, 255, 100)) end
                        if not espData or not espData.billboard or not espData.billboard.Parent then getgenv().espObjects[part] = nil continue end
                        local dist = getDistance(part.Position)
                        espData.label.Text = string.format("%s\n%.0f studs", ticketModel.Name, dist)
                    end
                end
                task.wait(0.3)
            end
        end)
        coroutine.resume(getgenv().espThreads["ticket"])
    end
})

EspTab:Toggle({
    Title = "ESP Nextbot",
    Value = false,
    Callback = function(state)
        getgenv().featureStates.EspNextbot = state
        if getgenv().espThreads["nextbot"] then coroutine.close(getgenv().espThreads["nextbot"]) getgenv().espThreads["nextbot"] = nil end
        for part, data in pairs(getgenv().espObjects) do if data and data.type == "Nextbot" then removeESP(part) end end
        if not state then return end
        getgenv().espThreads["nextbot"] = coroutine.create(function()
            local function getDistance(pos)
                local char = LocalPlayer.Character
                local hrp = char and char:FindFirstChild("HumanoidRootPart")
                return hrp and (pos - hrp.Position).Magnitude or 0
            end
            while getgenv().featureStates.EspNextbot do
                local folder = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Players")
                if folder then
                    for _, npc in ipairs(folder:GetChildren()) do
                        if not npc:IsA("Model") then continue end
                        if npc:GetAttribute("Team") ~= "Nextbot" then continue end
                        local part = npc:FindFirstChild("Root") or npc:FindFirstChild("Head") or npc:FindFirstChild("HumanoidRootPart") or npc:FindFirstChildWhichIsA("BasePart")
                        if not part then continue end
                        local espData = getgenv().espObjects[part]
                        if not espData then espData = createESP(part, "Nextbot", Color3.fromRGB(200, 100, 255)) end
                        if not espData or not espData.billboard or not espData.billboard.Parent then getgenv().espObjects[part] = nil continue end
                        local dist = getDistance(part.Position)
                        local color = Color3.fromRGB(200, 100, 255)
                        if dist <= 60 then
                            local t = math.clamp((dist - 12) / 48, 0, 1)
                            color = Color3.new(1, 0.235 + 0.765 * t, 0.235)
                        end
                        espData.label.Text = string.format("%s\n%.0f studs", npc.Name, dist)
                        espData.label.TextColor3 = color
                    end
                end
                task.wait(0.3)
            end
        end)
        coroutine.resume(getgenv().espThreads["nextbot"])
    end
})

getgenv().tracerThread = nil

local function cleanupTracers()
    for _, line in pairs(getgenv().tracerLines) do
        if line then pcall(function() line:Remove() end) end
    end
    getgenv().tracerLines = {}
end

EspTab:Toggle({
    Title = "Tracer Downed Players",
    Value = false,
    Callback = function(state)
        getgenv().featureStates.TracerDowned = state
        if getgenv().tracerThread then coroutine.close(getgenv().tracerThread) getgenv().tracerThread = nil end
        cleanupTracers()
        if not state then return end
        getgenv().tracerThread = coroutine.create(function()
            while getgenv().featureStates.TracerDowned do
                cleanupTracers()
                local folder = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Players")
                if folder and camera then
                    for _, char in ipairs(folder:GetChildren()) do
                        if not char:IsA("Model") then continue end
                        if char:GetAttribute("Team") == "Nextbot" then continue end
                        if char.Name == LocalPlayer.Name then continue end
                        if char:GetAttribute("Downed") ~= true then continue end
                        local hrp = char:FindFirstChild("HumanoidRootPart")
                        if not hrp then continue end
                        local pos, onScreen = camera:WorldToViewportPoint(hrp.Position)
                        if onScreen then
                            local tracer = Drawing.new("Line")
                            tracer.Color = Color3.fromRGB(255, 60, 60)
                            tracer.Thickness = 1.5
                            tracer.From = Vector2.new(camera.ViewportSize.X / 2, camera.ViewportSize.Y)
                            tracer.To = Vector2.new(pos.X, pos.Y)
                            tracer.Visible = true
                            table.insert(getgenv().tracerLines, tracer)
                        end
                    end
                end
                task.wait(0.1)
            end
        end)
        coroutine.resume(getgenv().tracerThread)
    end
})

local LP = Players.LocalPlayer
local CURRENT_AVATAR = nil
local ORIGINAL_DESC = nil

if LP.Character then
    local HUM = LP.Character:FindFirstChild("Humanoid")
    if HUM then ORIGINAL_DESC = HUM:GetAppliedDescription() end
end

local function applyAvatarToCharacter(char, name, id, desc)
    task.spawn(function()
        local HUM = char:WaitForChild("Humanoid", 10)
        if not HUM then return end
        for _, v in char:GetDescendants() do
            if v:IsA("Accessory") or v:IsA("Hat") then v:Destroy() end
        end
        for _, v in char:GetChildren() do
            if v:IsA("Shirt") or v:IsA("Pants") or v:IsA("ShirtGraphic") or v:IsA("CharacterMesh") then v:Destroy() end
        end
        local BC = HUM:FindFirstChildOfClass("BodyColors")
        if BC then BC:Destroy() end
        for _, partName in {"Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg"} do
            local part = char:FindFirstChild(partName)
            if part then
                for _, v in part:GetChildren() do if v:IsA("SpecialMesh") then v:Destroy() end end
            end
        end
        local HEAD = char:FindFirstChild("Head")
        if HEAD then
            local mesh = HEAD:FindFirstChildOfClass("SpecialMesh")
            if mesh then mesh.MeshId = "" mesh.TextureId = "" end
        end
        task.wait(0.1)
        HUM:ApplyDescriptionClientServer(desc)
        HUM.DisplayName = name
    end)
end

local function changeAvatar(input)
    local userId = tonumber(input)
    local userName = nil
    if userId then
        local success, name = pcall(function() return Players:GetNameFromUserIdAsync(userId) end)
        if not success or not name then return end
        userName = name
    else
        local success, id = pcall(function() return Players:GetUserIdFromNameAsync(input) end)
        if not success or not id then return end
        userId = id
        local success2, name = pcall(function() return Players:GetNameFromUserIdAsync(userId) end)
        if not success2 or not name then return end
        userName = name
    end
    local success, desc = pcall(function() return Players:GetHumanoidDescriptionFromUserId(userId) end)
    if not success or not desc then return end
    CURRENT_AVATAR = { id = userId, name = userName, desc = desc }
    if LP.Character then applyAvatarToCharacter(LP.Character, userName, userId, desc) end
    LP.CharacterAdded:Connect(function(char)
        if CURRENT_AVATAR and CURRENT_AVATAR.id == userId then applyAvatarToCharacter(char, userName, userId, desc) end
    end)
end

local function resetToOriginal()
    if ORIGINAL_DESC and LP.Character then
        local HUM = LP.Character:FindFirstChild("Humanoid")
        if HUM then
            HUM:ApplyDescriptionClientServer(ORIGINAL_DESC)
            HUM.DisplayName = LP.DisplayName
        end
    end
    CURRENT_AVATAR = nil
end

Window:Divider()

VisualTab:Input({
    Title = "Username / User ID",
    Description = "Enter Roblox username or user ID",
    Placeholder = "Roblox or 123456789",
    Callback = function(value) if value and value ~= "" then changeAvatar(value) end end
})

VisualTab:Button({
    Title = "Apply Avatar",
    Description = "Apply avatar from entered username/ID",
    Callback = function()
        local input = VisualTab:GetInput and VisualTab:GetInput("Username / User ID")
        if input and input ~= "" then changeAvatar(input) end
    end
})

VisualTab:Button({
    Title = "Reset Avatar",
    Description = "Reset back to your original avatar",
    Callback = function() resetToOriginal() end
})

local HEADLESS_MESH_ID = "rbxassetid://1095708"
local KORBLOX_MESH_ID = "rbxassetid://101851696"
local KORBLOX_COLOR = Color3.fromRGB(50, 50, 50)
local headlessEnabled = false
local korbloxEnabled = false
local headlessMesh
local originalLegColor

local function applyHeadless(head)
    if not head then return end
    head.Transparency = 1
    head.CanCollide = false
    local face = head:FindFirstChild("face")
    if face then face:Destroy() end
    headlessMesh = Instance.new("SpecialMesh")
    headlessMesh.MeshType = Enum.MeshType.FileMesh
    headlessMesh.MeshId = HEADLESS_MESH_ID
    headlessMesh.Scale = Vector3.new(0.001, 0.001, 0.001)
    headlessMesh.Parent = head
end

local function removeHeadless(head)
    if not head then return end
    if headlessMesh then headlessMesh:Destroy() headlessMesh = nil end
    head.Transparency = 0
    head.CanCollide = true
end

local function applyKorblox(character)
    local rightLeg = character:FindFirstChild("Right Leg") or character:FindFirstChild("RightUpperLeg")
    if not rightLeg then return end
    if not originalLegColor then originalLegColor = rightLeg.Color end
    for _, child in ipairs(rightLeg:GetChildren()) do
        if child:IsA("SpecialMesh") or child:IsA("CharacterMesh") then child:Destroy() end
    end
    rightLeg.Color = KORBLOX_COLOR
    local mesh = Instance.new("SpecialMesh")
    mesh.MeshType = Enum.MeshType.FileMesh
    mesh.MeshId = KORBLOX_MESH_ID
    mesh.Scale = Vector3.new(1, 1, 1)
    mesh.Parent = rightLeg
end

local function removeKorblox(character)
    local rightLeg = character:FindFirstChild("Right Leg") or character:FindFirstChild("RightUpperLeg")
    if not rightLeg then return end
    for _, child in ipairs(rightLeg:GetChildren()) do
        if child:IsA("SpecialMesh") or child:IsA("CharacterMesh") then child:Destroy() end
    end
    if originalLegColor then rightLeg.Color = originalLegColor originalLegColor = nil end
end

LocalPlayer.CharacterAdded:Connect(function(char)
    task.wait(0.5)
    local head = char:FindFirstChild("Head")
    if headlessEnabled then applyHeadless(head) end
    if korbloxEnabled then applyKorblox(char) end
end)

VisualTab:Toggle({
    Title = "Headless",
    Value = false,
    Callback = function(state)
        headlessEnabled = state
        local char = LocalPlayer.Character
        if not char then return end
        local head = char:FindFirstChild("Head")
        if state then applyHeadless(head) else removeHeadless(head) end
    end
})

VisualTab:Toggle({
    Title = "Korblox",
    Value = false,
    Callback = function(state)
        korbloxEnabled = state
        local char = LocalPlayer.Character
        if not char then return end
        if state then applyKorblox(char) else removeKorblox(char) end
    end
})

local cosmetic1 = ""
local cosmetic2 = ""
local originalCosmetic1 = ""
local originalCosmetic2 = ""
local isSwapped = false

VisualTab:Section({ Title = "Cosmetics Changer", TextSize = 20 })
VisualTab:Divider()

VisualTab:Input({
    Title = "Current Cosmetics",
    Placeholder = "Enter current cosmetic name",
    Callback = function(v)
        cosmetic1 = v
        if not isSwapped then originalCosmetic1 = v end
    end
})

VisualTab:Input({
    Title = "Select Cosmetics",
    Placeholder = "Enter cosmetic to swap with",
    Callback = function(v)
        cosmetic2 = v
        if not isSwapped then originalCosmetic2 = v end
    end
})

VisualTab:Button({
    Title = "Apply Cosmetics",
    Callback = function()
        pcall(function()
            if cosmetic1 == "" or cosmetic2 == "" or cosmetic1 == cosmetic2 then return end
            local Cosmetics = ReplicatedStorage:WaitForChild("Items"):WaitForChild("Cosmetics")
            function normalize(str) return str:gsub("%s+", ""):lower() end
            function levenshtein(s, t)
                local m, n = #s, #t
                local d = {}
                for i = 0, m do d[i] = {[0] = i} end
                for j = 0, n do d[0][j] = j end
                for i = 1, m do
                    for j = 1, n do
                        local cost = (s:sub(i,i) == t:sub(j,j)) and 0 or 1
                        d[i][j] = math.min(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + cost)
                    end
                end
                return d[m][n]
            end
            function similarity(s, t)
                local nS, nT = normalize(s), normalize(t)
                local dist = levenshtein(nS, nT)
                return 1 - dist / math.max(#nS, #nT)
            end
            function findSimilar(name)
                local bestMatch = name
                local bestScore = 0.5
                for _, c in ipairs(Cosmetics:GetChildren()) do
                    local score = similarity(name, c.Name)
                    if score > bestScore then
                        bestScore = score
                        bestMatch = c.Name
                    end
                end
                return bestMatch
            end
            cosmetic1 = findSimilar(cosmetic1)
            cosmetic2 = findSimilar(cosmetic2)
            local a = Cosmetics:FindFirstChild(cosmetic1)
            local b = Cosmetics:FindFirstChild(cosmetic2)
            if not a or not b then return end
            if not isSwapped then originalCosmetic1 = cosmetic1 originalCosmetic2 = cosmetic2 end
            local tempRoot = Instance.new("Folder", Cosmetics)
            tempRoot.Name = "__temp_swap_" .. tostring(tick()):gsub("%.", "_")
            local tempA = Instance.new("Folder", tempRoot)
            local tempB = Instance.new("Folder", tempRoot)
            for _, c in ipairs(a:GetChildren()) do c.Parent = tempA end
            for _, c in ipairs(b:GetChildren()) do c.Parent = tempB end
            for _, c in ipairs(tempA:GetChildren()) do c.Parent = b end
            for _, c in ipairs(tempB:GetChildren()) do c.Parent = a end
            tempRoot:Destroy()
            isSwapped = true
        end)
    end
})

VisualTab:Button({
    Title = "Reset Cosmetics",
    Callback = function()
        pcall(function()
            if not isSwapped then return end
            if originalCosmetic1 == "" or originalCosmetic2 == "" then return end
            local Cosmetics = ReplicatedStorage:WaitForChild("Items"):WaitForChild("Cosmetics")
            function normalize(str) return str:gsub("%s+", ""):lower() end
            function findSimilar(name)
                for _, c in ipairs(Cosmetics:GetChildren()) do
                    if normalize(c.Name) == normalize(name) then return c.Name end
                end
                return name
            end
            local resetCosmetic1 = findSimilar(originalCosmetic1)
            local resetCosmetic2 = findSimilar(originalCosmetic2)
            local a = Cosmetics:FindFirstChild(cosmetic1)
            local b = Cosmetics:FindFirstChild(cosmetic2)
            if a and b then
                local tempRoot = Instance.new("Folder", Cosmetics)
                tempRoot.Name = "__temp_reset_" .. tostring(tick()):gsub("%.", "_")
                local tempA = Instance.new("Folder", tempRoot)
                local tempB = Instance.new("Folder", tempRoot)
                for _, c in ipairs(a:GetChildren()) do c.Parent = tempA end
                for _, c in ipairs(b:GetChildren()) do c.Parent = tempB end
                for _, c in ipairs(tempA:GetChildren()) do c.Parent = b end
                for _, c in ipairs(tempB:GetChildren()) do c.Parent = a end
                tempRoot:Destroy()
                isSwapped = false
            end
        end)
    end
})

local emoteModelScript = nil
local originalEmoteData = {}
local replacementEnabled = false
local emoteDataSaved = false
local emoteFrame = nil
local currentTag = nil
local currentEmotes = {}
local selectEmotes = {}
local currentEmoteInputs = {}
local selectEmoteInputs = {}
local allEmotesList = {}

for i = 1, 12 do currentEmotes[i] = "" selectEmotes[i] = "" end

local Events = ReplicatedStorage:WaitForChild("Events", 10)
local CharacterFolder = Events and Events:WaitForChild("Character", 10)
local EmoteRemote = CharacterFolder and CharacterFolder:WaitForChild("Emote", 10)
local PassCharacterInfo = CharacterFolder and CharacterFolder:WaitForChild("PassCharacterInfo", 10)
local remoteSignal = PassCharacterInfo and PassCharacterInfo.OnClientEvent

local emoteNameCache = {}
local normalizedCache = {}

local function loadAllEmotes()
    local emotesFolder = ReplicatedStorage:FindFirstChild("Items")
    if emotesFolder then
        emotesFolder = emotesFolder:FindFirstChild("Emotes")
        if emotesFolder then
            for _, emoteModule in ipairs(emotesFolder:GetChildren()) do
                if emoteModule:IsA("ModuleScript") then
                    local success, emoteData = pcall(require, emoteModule)
                    if success and emoteData and emoteData.AppearanceInfo then
                        local displayName = emoteData.AppearanceInfo.NameShorted or emoteData.AppearanceInfo.Name
                        table.insert(allEmotesList, {
                            moduleName = emoteModule.Name,
                            displayName = displayName,
                            normalized = string.lower(displayName:gsub("%s+", ""))
                        })
                    end
                end
            end
        end
    end
end
loadAllEmotes()

local function searchEmote(partialName)
    if partialName == "" then return nil end
    local normalizedSearch = string.lower(partialName:gsub("%s+", ""))
    for _, emote in ipairs(allEmotesList) do
        if emote.normalized:find(normalizedSearch, 1, true) then return emote.displayName end
    end
    return nil
end

function fireSelect(emoteName)
    if not currentTag then return end
    local tagNumber = tonumber(currentTag)
    if not tagNumber or tagNumber < 0 or tagNumber > 255 then return end
    if not emoteName or emoteName == "" then return end
    local bufferData = buffer.create(2)
    buffer.writeu8(bufferData, 0, tagNumber)
    buffer.writeu8(bufferData, 1, 17)
    if remoteSignal then firesignal(remoteSignal, bufferData, {emoteName}) end
end

function setupAnimationListener()
    local function normalizeText(text) return string.lower(text:gsub("%s+", "")) end
    local function setupHumanoidListeners(char)
        local isR15 = char:GetAttribute("R15") == true
        local humanoid
        if isR15 then
            local r15Visual = char:WaitForChild("R15Visual", 5)
            if r15Visual then humanoid = r15Visual:WaitForChild("Visual_Humanoid", 5) end
        else
            humanoid = char:WaitForChild("Humanoid", 5)
        end
        if humanoid then
            humanoid.AnimationPlayed:Connect(function(track)
                local animation = track.Animation
                if animation and animation:IsDescendantOf(ReplicatedStorage.Items.Emotes) then
                    local emoteModule = animation:FindFirstAncestorWhichIsA("ModuleScript")
                    if emoteModule then
                        local currentEmoteName = emoteModule.Name
                        for i = 1, 12 do
                            if currentEmotes[i] ~= "" and selectEmotes[i] ~= "" then
                                if normalizeText(currentEmotes[i]) == normalizeText(currentEmoteName) then
                                    fireSelect(selectEmotes[i])
                                    break
                                end
                            end
                        end
                    end
                end
            end)
        end
    end
    if player.Character then setupHumanoidListeners(player.Character) end
    player.CharacterAdded:Connect(function(newChar) task.wait(0.5) setupHumanoidListeners(newChar) end)
end
setupAnimationListener()

function findEmoteModelScript()
    if emoteModelScript then return emoteModelScript end
    for _, script in pairs(player.PlayerScripts:GetDescendants()) do
        if script.Name == "EmoteModel" then emoteModelScript = script return script end
    end
    for _, script in pairs(ReplicatedStorage:GetDescendants()) do
        if script.Name == "EmoteModel" then emoteModelScript = script return script end
    end
    return nil
end

function normalizeText(text)
    if not text then return "" end
    if not normalizedCache[text] then normalizedCache[text] = string.lower(text:gsub("%s+", "")) end
    return normalizedCache[text]
end

function findEmoteModuleByDisplayName(displayName)
    if displayName == "NONE" or not displayName then return nil end
    if emoteNameCache[displayName] ~= nil then return emoteNameCache[displayName] end
    local emotesFolder = ReplicatedStorage:FindFirstChild("Items")
    if not emotesFolder then emoteNameCache[displayName] = nil return nil end
    emotesFolder = emotesFolder:FindFirstChild("Emotes")
    if not emotesFolder then emoteNameCache[displayName] = nil return nil end
    local normalizedDisplayName = normalizeText(displayName)
    for _, emoteModule in pairs(emotesFolder:GetChildren()) do
        local success, emoteData = pcall(require, emoteModule)
        if success and emoteData then
            local emoteDisplayName = emoteData.AppearanceInfo.NameShorted or emoteData.AppearanceInfo.Name
            if normalizeText(emoteDisplayName) == normalizedDisplayName then
                emoteNameCache[displayName] = emoteModule.Name
                return emoteModule.Name
            end
        end
    end
    emoteNameCache[displayName] = nil
    return nil
end

function getEmoteFrame()
    local playerGui = player.PlayerGui
    local shared = playerGui and playerGui:FindFirstChild("Shared")
    local hud = shared and shared:FindFirstChild("HUD")
    local interactors = hud and hud:FindFirstChild("Interactors")
    local popups = interactors and interactors:FindFirstChild("Popups")
    return popups and popups:FindFirstChild("Emote")
end

function cleanUpLastEmoteFrame() emoteFrame = nil end

function restoreOriginalEmotes()
    if not emoteModelScript then findEmoteModelScript() end
    if not emoteModelScript or not emoteFrame then return end
    local emoteModelFunction = require(emoteModelScript)
    local emoteWheel = emoteFrame:FindFirstChild("Wheel")
    local emoteWheel2 = emoteFrame:FindFirstChild("Wheel2")
    if not emoteWheel then return end
    local function processSlot(emoteSlot, key)
        if not emoteSlot then return end
        local textLabel = emoteSlot:FindFirstChild("TextLabel")
        local viewportFrame = emoteSlot:FindFirstChild("ViewportFrame")
        if textLabel and viewportFrame then
            local original = originalEmoteData[key]
            if original then
                if viewportFrame:FindFirstChild("WorldModel") then viewportFrame.WorldModel:Destroy() end
                if original.displayText ~= "NONE" and original.emoteName then
                    local emoteModule = ReplicatedStorage.Items.Emotes:FindFirstChild(original.emoteName)
                    if emoteModule then emoteModelFunction(viewportFrame, original.emoteName) end
                    textLabel.Text = original.displayText
                else
                    textLabel.Text = "NONE"
                end
            end
        end
    end
    for i = 1, 6 do processSlot(emoteWheel:FindFirstChild("Emote"..i), "Wheel_Emote"..i) end
    if emoteWheel2 then
        for i = 1, 6 do processSlot(emoteWheel2:FindFirstChild("Emote"..i), "Wheel2_Emote"..i) end
    end
end

function replaceEmotesFrame()
    if not replacementEnabled or not emoteDataSaved or not emoteFrame then return false end
    if not emoteModelScript then findEmoteModelScript() end
    if not emoteModelScript then return false end
    local emoteModelFunction = require(emoteModelScript)
    local emoteWheel = emoteFrame:FindFirstChild("Wheel")
    local emoteWheel2 = emoteFrame:FindFirstChild("Wheel2")
    if not emoteWheel then return false end
    local anyPairsConfigured = false
    local function processEmoteSlot(emoteSlot, wheelName, i)
        if not emoteSlot then return end
        local textLabel = emoteSlot:FindFirstChild("TextLabel")
        if not textLabel then return end
        local currentText = textLabel.Text
        local normalizedCurrent = normalizeText(currentText)
        for j = 1, 12 do
            local searchEmote = currentEmotes[j]
            local replaceEmote = selectEmotes[j]
            if searchEmote ~= "" and replaceEmote ~= "" then
                anyPairsConfigured = true
                if normalizedCurrent == normalizeText(searchEmote) then
                    local viewportFrame = emoteSlot.ViewportFrame
                    if viewportFrame then
                        local replacementModule = ReplicatedStorage.Items.Emotes:FindFirstChild(replaceEmote)
                        if replacementModule then
                            local key = wheelName.."_Emote"..i
                            if not originalEmoteData[key] then
                                originalEmoteData[key] = {
                                    displayText = currentText,
                                    emoteName = findEmoteModuleByDisplayName(currentText) or currentText
                                }
                            end
                            if viewportFrame:FindFirstChild("WorldModel") then viewportFrame.WorldModel:Destroy() end
                            local targetEmoteModule = require(replacementModule)
                            local targetDisplayName = targetEmoteModule.AppearanceInfo.NameShorted or targetEmoteModule.AppearanceInfo.Name
                            emoteModelFunction(viewportFrame, replaceEmote)
                            textLabel.Text = targetDisplayName
                        end
                    end
                    break
                end
            end
        end
    end
    for i = 1, 6 do processEmoteSlot(emoteWheel:FindFirstChild("Emote"..i), "Wheel", i) end
    if emoteWheel2 then
        for i = 1, 6 do processEmoteSlot(emoteWheel2:FindFirstChild("Emote"..i), "Wheel2", i) end
    end
    return anyPairsConfigured
end

function saveOriginalEmoteData(frame)
    if not frame then return end
    originalEmoteData = {}
    local emoteWheel = frame:FindFirstChild("Wheel")
    local emoteWheel2 = frame:FindFirstChild("Wheel2")
    if not emoteWheel then return end
    local function saveSlot(emoteSlot, key)
        if not emoteSlot then return end
        local textLabel = emoteSlot:FindFirstChild("TextLabel")
        if textLabel then
            originalEmoteData[key] = {
                displayText = textLabel.Text,
                emoteName = findEmoteModuleByDisplayName(textLabel.Text) or textLabel.Text
            }
        end
    end
    for i = 1, 6 do saveSlot(emoteWheel:FindFirstChild("Emote"..i), "Wheel_Emote"..i) end
    if emoteWheel2 then
        for i = 1, 6 do saveSlot(emoteWheel2:FindFirstChild("Emote"..i), "Wheel2_Emote"..i) end
    end
    emoteDataSaved = true
end

function readTagFromFolder(folder)
    if not folder then return nil end
    local attributeValue = folder:GetAttribute("Tag")
    if attributeValue ~= nil then return attributeValue end
    local tagValue = folder:FindFirstChild("Tag")
    if tagValue and tagValue:IsA("ValueBase") then return tagValue.Value end
    return nil
end

local respawnInProgress = false
local lastRespawnTime = 0
local reapplyThread = nil

function cleanupOnRespawn()
    currentTag = nil
    emoteFrame = nil
    if reapplyThread then task.cancel(reapplyThread) reapplyThread = nil end
end

function handleSingleRespawn()
    local now = tick()
    if respawnInProgress and (now - lastRespawnTime) < 2 then return end
    respawnInProgress = true
    lastRespawnTime = now
    cleanupOnRespawn()
    task.spawn(function()
        local startTime = tick()
        while tick() - startTime < 10 do
            if Workspace:FindFirstChild("Game") and Workspace.Game:FindFirstChild("Players") then
                local playerFolder = Workspace.Game.Players:FindFirstChild(player.Name)
                if playerFolder then
                    currentTag = readTagFromFolder(playerFolder)
                    if currentTag then
                        local tagNumber = tonumber(currentTag)
                        if tagNumber and tagNumber >= 0 and tagNumber <= 255 then break end
                    end
                end
            end
            task.wait(0.1)
        end
        respawnInProgress = false
    end)
    if replacementEnabled and emoteDataSaved then
        if reapplyThread then task.cancel(reapplyThread) reapplyThread = nil end
        reapplyThread = task.delay(1.5, function()
            if not replacementEnabled or not emoteDataSaved then return end
            for attempts = 1, 30 do
                emoteFrame = getEmoteFrame()
                if emoteFrame then
                    saveOriginalEmoteData(emoteFrame)
                    restoreOriginalEmotes()
                    replaceEmotesFrame()
                    break
                end
                task.wait(0.1)
            end
        end)
    end
end

VisualTab:Section({ Title = "Current Emotes", TextSize = 16 })
for i = 1, 12 do
    currentEmoteInputs[i] = VisualTab:Input({
        Title = "Current Emote " .. i,
        Placeholder = "Type emote name...",
        Value = currentEmotes[i],
        Callback = function(v) currentEmotes[i] = v end
    })
end

VisualTab:Divider()
VisualTab:Section({ Title = "Select Emotes", TextSize = 16 })
for i = 1, 12 do
    selectEmoteInputs[i] = VisualTab:Input({
        Title = "Select Emote " .. i,
        Placeholder = "Type emote name...",
        Value = selectEmotes[i],
        Callback = function(v) selectEmotes[i] = v end
    })
end

VisualTab:Divider()

VisualTab:Input({
    Title = "Emote Possible option",
    Desc = "Use 1-3 (0 or 'Random' for random)",
    Placeholder = "0",
    Value = "0",
    Callback = function(v)
        local currentNum = v:lower() == "random" and "Random" or tonumber(v) or 0
        local function setupCharacter(char)
            if char == player.Character then
                char:SetAttribute("EmoteNum", currentNum == "Random" and math.random(1, 3) or currentNum)
            end
        end
        local function monitorCharacter()
            while true do
                task.wait(1)
                local char = player.Character
                if char then
                    char:SetAttribute("EmoteNum", currentNum == "Random" and math.random(1, 3) or currentNum)
                end
            end
        end
        if player.Character then setupCharacter(player.Character) end
        player.CharacterAdded:Connect(function(char) task.wait(1) setupCharacter(char) end)
        coroutine.wrap(monitorCharacter)()
    end
})

VisualTab:Toggle({
    Title = "Enable replace emote wheel",
    Value = replacementEnabled,
    Callback = function(state)
        replacementEnabled = state
        if emoteFrame and emoteDataSaved then restoreOriginalEmotes() end
        if replacementEnabled and emoteFrame and emoteDataSaved then replaceEmotesFrame() end
    end
})

VisualTab:Button({
    Title = "Apply Emote Mappings",
    Icon = "refresh-cw",
    Callback = function()
        local hasAnyEmote = false
        for i = 1, 12 do if currentEmotes[i] ~= "" or selectEmotes[i] ~= "" then hasAnyEmote = true break end end
        if not hasAnyEmote then return end
        local function normalizeEmoteName(name) return name:gsub("%s+", ""):lower() end
        local function isValidEmote(emoteName)
            if emoteName == "" then return false, "" end
            local normalizedInput = normalizeEmoteName(emoteName)
            local emotesFolder = ReplicatedStorage:FindFirstChild("Items")
            if emotesFolder then
                emotesFolder = emotesFolder:FindFirstChild("Emotes")
                if emotesFolder then
                    for _, emoteModule in ipairs(emotesFolder:GetChildren()) do
                        if emoteModule:IsA("ModuleScript") then
                            if normalizeEmoteName(emoteModule.Name) == normalizedInput then
                                return true, emoteModule.Name
                            end
                        end
                    end
                end
            end
            return false, ""
        end
        local sameEmoteSlots, missingEmoteSlots, invalidEmoteSlots, successfulSlots = {}, {}, {}, {}
        for i = 1, 12 do
            if currentEmotes[i] ~= "" and selectEmotes[i] ~= "" then
                local currentValid, currentActual = isValidEmote(currentEmotes[i])
                local selectValid, selectActual = isValidEmote(selectEmotes[i])
                if not currentValid and not selectValid then
                    table.insert(invalidEmoteSlots, { slot = i, currentName = currentEmotes[i], selectName = selectEmotes[i] })
                elseif not currentValid then
                    table.insert(invalidEmoteSlots, { slot = i, currentName = currentEmotes[i] })
                elseif not selectValid then
                    table.insert(invalidEmoteSlots, { slot = i, selectName = selectEmotes[i] })
                elseif currentActual:lower() == selectActual:lower() then
                    table.insert(sameEmoteSlots, i)
                else
                    table.insert(successfulSlots, i)
                end
            elseif currentEmotes[i] ~= "" or selectEmotes[i] ~= "" then
                table.insert(missingEmoteSlots, i)
            end
        end
        emoteNameCache = {}
        normalizedCache = {}
        cleanUpLastEmoteFrame()
        emoteFrame = getEmoteFrame()
        if not emoteFrame then return end
        saveOriginalEmoteData(emoteFrame)
        restoreOriginalEmotes()
        if replacementEnabled then replaceEmotesFrame() end
    end
})

VisualTab:Button({
    Title = "Reset All Emotes",
    Icon = "trash-2",
    Callback = function()
        if emoteFrame then restoreOriginalEmotes() end
        for i = 1, 12 do
            currentEmotes[i] = ""
            selectEmotes[i] = ""
            if currentEmoteInputs[i] then currentEmoteInputs[i]:SetValue("") end
            if selectEmoteInputs[i] then selectEmoteInputs[i]:SetValue("") end
        end
        emoteNameCache = {}
        normalizedCache = {}
        cleanUpLastEmoteFrame()
    end
})

if player.Character then task.spawn(handleSingleRespawn) end
player.CharacterAdded:Connect(function() task.wait(1) handleSingleRespawn() end)
if Workspace:FindFirstChild("Game") and Workspace.Game:FindFirstChild("Players") then
    Workspace.Game.Players.ChildAdded:Connect(function(child)
        if child.Name == player.Name then task.wait(0.5) handleSingleRespawn() end
    end)
    Workspace.Game.Players.ChildRemoved:Connect(function(child)
        if child.Name == player.Name then currentTag = nil cleanUpLastEmoteFrame() end
    end)
end

local invisPartsFolder = workspace:FindFirstChild("Game") and workspace.Game:FindFirstChild("Map") and workspace.Game.Map:FindFirstChild("InvisParts")
local invisPartsState = false

GameTab:Toggle({
    Title = "Clear Invis Walls",
    Value = false,
    Callback = function(active)
        if invisPartsFolder then
            for _, obj in ipairs(invisPartsFolder:GetDescendants()) do
                if obj:IsA("BasePart") then obj.CanCollide = not active end
            end
        end
        invisPartsState = active
    end
})

GameTab:Button({
    Title = "Lower Chunks",
    Description = "Reduces the streaming radius for performance improvement",
    Callback = function() workspace.StreamingMinRadius = 200 workspace.StreamingTargetRadius = 500 end
})

GameTab:Button({
    Title = "Disable VSync",
    Description = "Disables vertical sync to allow higher FPS",
    Callback = function() setfpscap(9999) end
})

GameTab:Button({
    Title = "Set FPS Cap Max",
    Description = "Sets the FPS cap to a very high value",
    Callback = function() setfpscap(99999) end
})

GameTab:Button({
    Title = "Hide Skybox",
    Description = "Removes the skybox for better performance",
    Callback = function() game.Lighting.Sky = nil end
})

GameTab:Button({
    Title = "Disable Shadows",
    Description = "Disables all shadows in the workspace",
    Callback = function()
        for _, part in pairs(workspace:GetDescendants()) do
            if part:IsA("BasePart") then part.CastShadow = false end
        end
    end
})

GameTab:Button({
    Title = "Set Low Graphics",
    Description = "Sets the rendering quality to low",
    Callback = function() settings().Rendering.QualityLevel = 1 end
})

GameTab:Button({
    Title = "Disable Particles",
    Description = "Removes all particle effects from the game",
    Callback = function()
        for _, particle in pairs(workspace:GetDescendants()) do
            if particle:IsA("ParticleEmitter") then particle:Destroy() end
        end
    end
})

GameTab:Button({
    Title = "Night",
    Description = "Sets the time to night (00:00)",
    Callback = function() game.Lighting.TimeOfDay = "00:00:00" end
})

GameTab:Button({
    Title = "Day",
    Description = "Sets the time to morning (08:00)",
    Callback = function() game.Lighting.TimeOfDay = "08:00:00" end
})

GameTab:Button({
    Title = "Anti Fog",
    Description = "Removes fog for better visibility",
    Callback = function() game.Lighting.FogStart = 1e5 game.Lighting.FogEnd = 1e6 end
})

GameTab:Button({
    Title = "Low Poly Mode",
    Description = "Converts meshes to low-poly for performance",
    Callback = function()
        for _, p in pairs(workspace:GetDescendants()) do
            if p:IsA("MeshPart") or p:IsA("UnionOperation") then
                p.Material = Enum.Material.Plastic
                p.RenderFidelity = Enum.RenderFidelity.Performance
            end
        end
    end
})

GameTab:Button({
    Title = "Disable Simplify Lighting",
    Description = "Disables lighting simplifications for custom settings",
    Callback = function()
        local L = game.Lighting
        L.Technology = Enum.Technology.Compatibility
        L.ShadowSoftness = 0
        L.EnvironmentDiffuseScale = 0
        L.EnvironmentSpecularScale = 0
    end
})

GameTab:Button({
    Title = "Hide Useless Chunks",
    Description = "Reduces streaming of unnecessary chunks",
    Callback = function() workspace.StreamingMinRadius = 0 workspace.StreamingTargetRadius = 500 end
})

GameTab:Button({
    Title = "Reduce Anti-Aliasing",
    Description = "Reduces anti-aliasing for performance",
    Callback = function() game:GetService("Rendering"):SetCore("AntiAliasing", Enum.AntiAliasingLevel.Two) end
})

GameTab:Button({
    Title = "Low Quality",
    Callback = function()
        local ToDisable = { Textures = true, VisualEffects = true, Parts = true, Particles = true, Sky = true }
        for _, v in next, game:GetDescendants() do
            if ToDisable.Parts and (v:IsA("Part") or v:IsA("UnionOperation") or v:IsA("BasePart")) then v.Material = Enum.Material.SmoothPlastic end
            if ToDisable.Particles and (v:IsA("ParticleEmitter") or v:IsA("Smoke") or v:IsA("Explosion") or v:IsA("Sparkles") or v:IsA("Fire")) then v.Enabled = false end
            if ToDisable.VisualEffects and (v:IsA("BloomEffect") or v:IsA("BlurEffect") or v:IsA("DepthOfFieldEffect") or v:IsA("SunRaysEffect")) then v.Enabled = false end
            if ToDisable.Textures and (v:IsA("Decal") or v:IsA("Texture")) then v.Texture = "" end
            if ToDisable.Sky and v:IsA("Sky") then v.Parent = nil end
        end
        WindUI:Notify({ Title = "Low Quality", Content = "Graphics settings optimized for performance", Duration = 3, Icon = "eye-off" })
    end
})

GameTab:Button({
    Title = "Remove Texture",
    Callback = function()
        for _, part in ipairs(workspace:GetDescendants()) do
            if part:IsA("Part") or part:IsA("MeshPart") or part:IsA("UnionOperation") or part:IsA("WedgePart") or part:IsA("CornerWedgePart") then
                if part:IsA("Part") then part.Material = Enum.Material.SmoothPlastic end
                if part:FindFirstChildWhichIsA("Texture") then part:FindFirstChildWhichIsA("Texture").Texture = "rbxassetid://0" end
                if part:FindFirstChildWhichIsA("Decal") then part:FindFirstChildWhichIsA("Decal").Texture = "rbxassetid://0" end
            end
        end
        WindUI:Notify({ Title = "Remove Texture", Content = "All textures have been removed", Duration = 3, Icon = "image" })
    end
})

GameTab:Toggle({
    Title = "Remove fog",
    Value = false,
    Callback = function(state)
        local Lighting = game:GetService("Lighting")
        if state then
            getgenv().featureStates.originalFogEnd = Lighting.FogEnd
            getgenv().featureStates.originalAtmospheres = {}
            for _, atmosphere in ipairs(Lighting:GetChildren()) do
                if atmosphere:IsA("Atmosphere") then table.insert(getgenv().featureStates.originalAtmospheres, atmosphere:Clone()) end
            end
            Lighting.FogEnd = 1000000
            for _, v in pairs(Lighting:GetDescendants()) do if v:IsA("Atmosphere") then v:Destroy() end end
            WindUI:Notify({ Title = "Remove Fog", Content = "Fog has been removed", Duration = 2, Icon = "cloud-off" })
        else
            if getgenv().featureStates and getgenv().featureStates.originalFogEnd then Lighting.FogEnd = getgenv().featureStates.originalFogEnd end
            if getgenv().featureStates and getgenv().featureStates.originalAtmospheres then
                for _, atmosphere in ipairs(getgenv().featureStates.originalAtmospheres) do
                    if not atmosphere.Parent then
                        local newAtmosphere = Instance.new("Atmosphere")
                        for _, prop in pairs({"Density", "Offset", "Color", "Decay", "Glare", "Haze"}) do
                            if atmosphere[prop] then newAtmosphere[prop] = atmosphere[prop] end
                        end
                        newAtmosphere.Parent = Lighting
                    end
                end
            end
            WindUI:Notify({ Title = "Remove Fog", Content = "Fog has been restored", Duration = 2, Icon = "cloud" })
        end
    end
})

MiscTab:Button({
    Title = "Breacher (Portal Gun)",
    Callback = function()
        local success, result = pcall(function()
            local Breacher = require(game:GetService("ReplicatedStorage").Tools.Breacher)
            local portalTask
            for i, task in ipairs(Breacher.Tasks) do
                if task.ResourceInfo and task.ResourceInfo.Type == "Clip" then portalTask = task break end
            end
            if not portalTask then portalTask = Breacher.Tasks[2] end
            portalTask.ResourceInfo.Cap = 999999
            local blueShoot = portalTask.Functions[1].Activations[1].Methods[1]
            local yellowShoot = portalTask.Functions[2].Activations[1].Methods[1]
            blueShoot.Info.Range = 999999
            yellowShoot.Info.Range = 999999
            blueShoot.Info.SpreadIncrease = 0
            yellowShoot.Info.SpreadIncrease = 0
            portalTask.MethodReferences.Portal.Info.SpreadInfo.MaxSpread = 0
            portalTask.MethodReferences.Portal.Info.SpreadInfo.MinSpread = 0
            portalTask.MethodReferences.Portal.Info.SpreadInfo.ReductionRate = 100
            blueShoot.Info.Cooldown = 0.1
            yellowShoot.Info.Cooldown = 0.1
            blueShoot.CooldownInfo = {}
            yellowShoot.CooldownInfo = {}
            blueShoot.Requirements = {}
            yellowShoot.Requirements = {}
            Breacher.Actions.ADS.Enabled = false
            local unequipMethod = Breacher.Tasks[1].AutomaticFunctions[2].Methods[1]
            unequipMethod.CooldownInfo = {}
            if blueShoot.CooldownInfo and blueShoot.CooldownInfo.DisabledActions then
                local newDisabled = {}
                for _, action in ipairs(blueShoot.CooldownInfo.DisabledActions) do if action ~= "ADS" then table.insert(newDisabled, action) end end
                blueShoot.CooldownInfo.DisabledActions = newDisabled
            end
            if yellowShoot.CooldownInfo and yellowShoot.CooldownInfo.DisabledActions then
                local newDisabled = {}
                for _, action in ipairs(yellowShoot.CooldownInfo.DisabledActions) do if action ~= "ADS" then table.insert(newDisabled, action) end end
                yellowShoot.CooldownInfo.DisabledActions = newDisabled
            end
            blueShoot.GlobalPriority = 500
            yellowShoot.GlobalPriority = 500
            blueShoot.Priority = 1
            yellowShoot.Priority = 1
            blueShoot.ResourceAboveZero = false
            yellowShoot.ResourceAboveZero = false
            portalTask.Functions[1].Activations[1].CanHoldDown = true
            portalTask.Functions[2].Activations[1].CanHoldDown = true
            if not blueShoot.Info.Speed then blueShoot.Info.Speed = 5000 yellowShoot.Info.Speed = 5000 end
            local baseTask = Breacher.Tasks[1]
            baseTask.AutomaticFunctions[1].Methods[1].Info.Cooldown = 0.1
            baseTask.AutomaticFunctions[2].Methods[1].Info.Cooldown = 0.1
            Breacher.Actions.LookBack.Enabled = true
            Breacher.Adjustments.ToolViewbob = true
            Breacher.Adjustments.AnimationRootStraight = true
            Breacher.Adjustments.TurnWaist = true
            Breacher.HUD.CrosshairType = "Accurate"
            Breacher.HUD.Colored = true
            if Breacher.Actions.ADS.Zoom then Breacher.Actions.ADS.Zoom = nil end
            return true
        end)
        if success then
            WindUI:Notify({ Title = "Breacher (Portal Gun)", Content = "Portal Gun Successfully upgraded! \n✓ Infinite charges \n✓ Maximum range \n✓ Instant reload", Duration = 6, Icon = "check-circle" })
        else
            WindUI:Notify({ Title = "Breacher Error", Content = "Error: " .. tostring(result), Duration = 5, Icon = "x-circle" })
        end
    end
})

MiscTab:Button({
    Title = "Grapplehook",
    Callback = function()
        local success, result = pcall(function()
            local GrappleHook = require(game:GetService("ReplicatedStorage").Tools["GrappleHook"])
            local grappleTask = GrappleHook.Tasks[2]
            local shootMethod = grappleTask.Functions[1].Activations[1].Methods[1]
            shootMethod.Info.Speed = 10000
            shootMethod.Info.Lifetime = 10.0
            shootMethod.Info.Gravity = Vector3.new(0, 0, 0)
            shootMethod.Info.SpreadIncrease = 0
            shootMethod.Info.Cooldown = 0.1
            grappleTask.MethodReferences.Projectile.Info.SpreadInfo.MaxSpread = 0
            grappleTask.MethodReferences.Projectile.Info.SpreadInfo.MinSpread = 0
            grappleTask.MethodReferences.Projectile.Info.SpreadInfo.ReductionRate = 100
            local checkMethod = grappleTask.AutomaticFunctions[1].Methods[1]
            checkMethod.Info.Cooldown = 0.1
            checkMethod.CooldownInfo.TestCooldown = 0.1
            grappleTask.ResourceInfo.Cap = 999999
            grappleTask.ResourceInfo.Reserve = 999999
            GrappleHook.Adjustments.ToolViewbob = false
            GrappleHook.Actions.LookBack.Enabled = true
            GrappleHook.Actions.ADS.Enabled = true
            GrappleHook.Actions.ADS.Zoom = 0.5
            shootMethod.GlobalPriority = 500
            return true
        end)
        if success then
            WindUI:Notify({ Title = "Grapplehook", Content = "Grapplehook Successfully upgraded! \n✓ Infinite charges \n✓ Super speed \n✓ No gravity", Duration = 6, Icon = "check-circle" })
        else
            WindUI:Notify({ Title = "Grapplehook Error", Content = "Error: " .. tostring(result), Duration = 5, Icon = "x-circle" })
        end
    end
})

MiscTab:Button({
    Title = "Smoke Grenade",
    Callback = function()
        local success, result = pcall(function()
            local SmokeGrenade = require(game:GetService("ReplicatedStorage").Tools["SmokeGrenade"])
            SmokeGrenade.RequiresOwnedItem = false
            local throwMethod = SmokeGrenade.Tasks[1].Functions[1].Activations[1].Methods[1]
            throwMethod.ItemUseIncrement = {"SmokeGrenade", 0}
            throwMethod.Info.Cooldown = 0.05
            throwMethod.Info.ThrowVelocity = 200
            SmokeGrenade.Tasks[1].Functions[1].Activations[1].CanHoldDown = true
            throwMethod.Info.SmokeDuration = 999
            throwMethod.Info.SmokeRadius = 100
            throwMethod.Info.FadeTime = 60
            local equipMethod = SmokeGrenade.Tasks[1].AutomaticFunctions[1].Methods[1]
            local unequipMethod = SmokeGrenade.Tasks[1].AutomaticFunctions[2].Methods[1]
            equipMethod.Info.Cooldown = 0.1
            unequipMethod.Info.Cooldown = 0.1
            throwMethod.GlobalPriority = 500
            throwMethod.CooldownInfo = {}
            SmokeGrenade.HUD.ShowAmount = false
            throwMethod.Info.Density = 0.9
            throwMethod.Info.Color = Color3.new(0.7, 0.7, 0.7)
            throwMethod.Info.ExplosionRadius = 20
            throwMethod.CooldownInfo.ActivatePhrase = nil
            SmokeGrenade.KeybindInfo.UnequipKeybind = "Backspace"
            local args = { [1] = 0, [2] = 20 }
            game:GetService("ReplicatedStorage").Events.Character.ToolAction:FireServer(unpack(args))
            return true
        end)
        if success then
            WindUI:Notify({ Title = "Smoke Grenade", Content = "Smoke Grenade Improved! \n✓ Infinite Grenades \n✓ Instant Reload \n✓ Massive Smoke Cloud", Duration = 6, Icon = "cloud" })
        else
            WindUI:Notify({ Title = "Smoke Grenade Error", Content = "Error: " .. tostring(result), Duration = 5, Icon = "x-circle" })
        end
    end
})

WindTab:Button({
    Title = "Toggle UI",
    Callback = function() Window:Toggle() end
})
