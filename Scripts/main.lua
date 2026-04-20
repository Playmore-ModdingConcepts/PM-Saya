local json = require("json")
local UEHelpers = require("UEHelpers")
local Utils = require("utils")
local Settings = require("Settings")
local DataProcessor = require("DataProcessor")
local PlayerPlaneDataTable = require("PlayerPlaneDataTable")

local DataTableClasses = {
    PlayerPlaneDataTable = PlayerPlaneDataTable,
}

local DataTableHandlers = {
    PlayerPlaneDataTable = {},
}


local DataCollections = {
    PlayerPlaneDataTable = {},
}

local function Log(message, funcName)
    Utils.Log(message, "main", funcName)
end

local function GetModDir()
    local dirs = IterateGameDirectories()
    return dirs.Game.Content.Paks["~DefaultMods"]["SAYA-R2"]
end

local function FindOrCreateModDir()
    local dirs = IterateGameDirectories()
    local modDir = GetModDir()

    if modDir then
        return modDir
    else
        Utils.Log(string.format("No such directory %s/SAYA-R2\n", dirs.Game.Content.Paks.__absolute_path),
            "main")
        Utils.Log(string.format("Creating directory %s/SAYA-R2\n", dirs.Game.Content.Paks.__absolute_path),
            "main")
        local success, result, code = os.execute(string.format("mkdir \"%s\\SAYA-R2\"",
            dirs.Game.Content.Paks.__absolute_path))
        if not success then
            Utils.Log(string.format("Failed to create directory: %s - %d\n", result, code))
            return nil
        else
            return GetModDir()
        end
    end
end

local function CreateModChildDirs(modDir)
    local modDirPath = modDir.__absolute_path

    for parent, children in pairs(Settings.ModChildDirs) do
        local parentDirPath = string.format("%s/%s", modDirPath, parent)
        if not modDir[parent] then
            Log(string.format("Creating directory %s\n", parentDirPath), "CreateModChildDirs")

            local success, result, code = os.execute(string.format("mkdir \"%s\"", parentDirPath))
            if not success then
                Log(string.format("Failed to create directory: %s - %d\n", result, code), "CreateModChildDirs")
                parentDirPath = ""
            end
        end

        if parentDirPath then
            for _, child in ipairs(children) do
                local childDirPath = string.format("%s/%s", parentDirPath, child)
                local failed, result, code = os.execute(
                    string.format("if exist \"%s\" (true)", childDirPath))
                if failed then
                    Log(string.format("Creating directory %s\n", childDirPath), "CreateModChildDirs")
                    local success, result, code = os.execute(string.format("mkdir \"%s\"", childDirPath))
                    if not success then
                        Log(string.format("Failed to create directory: %s - %d\n", result, code), "CreateModChildDirs")
                    end
                end
            end
        end
    end
end

local function CollectData(dir)
    Utils.Log(string.format("Collecting data from %s\n", dir.__absolute_path), "main", "CollectData")

    local collection = {
        Add = {},
        AddTo = {},
        ModifyIn = {},
        Replace = {},
        Remove = {},
        RemoveFrom = {}
    }

    for _, file in pairs(dir.__files) do
        local fp = io.open(file.__absolute_path, "r")
        local success, content = pcall(function() return fp:read("a") end)
        if not success then
            Utils.Log(string.format("Failed to read %s\n", file.__absolute_path))
            fp:close()
        else
            fp:close()
            local success, elementList = pcall(function() return json.decode(content) end)
            if not success then
                Utils.Log(string.format("Failed to parse contents of %s\n", file.__absolute_path), "main", "CollectData")
            else
                for _, element in pairs(elementList) do
                    if element["Action"] == "Add" then
                        Utils.Log(
                            string.format("Add %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "main",
                            "CollectData")
                        table.insert(collection.Add, { Name = element["Name"], Data = element["Data"] })
                    elseif element["Action"] == "Replace" then
                        Utils.Log(
                            string.format("Replace %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "main", "CollectData")
                        table.insert(collection.Replace, { Name = element["Name"], Data = element["Data"] })
                    elseif element["Action"] == "Remove" then
                        Utils.Log(
                            string.format("Remove %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "main", "CollectData")
                        table.insert(collection.Remove, { Name = element["Name"], Data = element["Data"] })
                    elseif element["Action"] == "AddTo" then
                        Log(
                            string.format("AddTo (Property) %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "CollectData")
                        table.insert(collection.AddTo, { Name = element["Name"], Data = element["Data"] })
                    elseif element["Action"] == "ModifyIn" then
                        Log(
                            string.format("ModifyIn (Property) %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "CollectData")
                        table.insert(collection.ModifyIn, { Name = element["Name"], Data = element["Data"] })
                    elseif element["Action"] == "RemoveFrom" then
                        Log(
                            string.format("RemoveFrom (Property) %s - Name: %s\tFile: %s\n", dir.__name, element["Name"], file.__name),
                            "CollectData")
                        table.insert(collection.RemoveFrom, { Name = element["Name"], Data = element["Data"] })
                    end
                end
            end
        end
    end

    return collection
end

local function IsDataTableValid(dataTable)
    return dataTable and dataTable:IsValid()
end

-- Some data tables aren't loaded when the game launches. But only via in-game
-- triggers.
-- Tables the need to be loaded on mod startup:
-- - DT_ManufactoringGroups
local function LoadDataTableAssets()
    local assetRegistryHelpers = StaticFindObject("/Script/AssetRegistry.Default__AssetRegistryHelpers")
    if not assetRegistryHelpers:IsValid() then
        Utils.Log("AssetRegistryHelpers is not valid\n", "main", "LoadDataTableAssets")
    end

    local assetRegistry = nil
    if assetRegistryHelpers then
        assetRegistry = assetRegistryHelpers:GetAssetRegistry()
        if not assetRegistry:IsValid() then
            assetRegistry = StaticFindObject("/Script/AssetRegistry.Default__AssetRegistryImpl")
        end
    end

    if not assetRegistry or not assetRegistry:IsValid() then
        Utils.Log("AssetRegistry is not valid. Can't load data table assets!\n", "main", "LoadDataTableAssets")
        return
    end

    -- Table of DataTables that need to be loaded at the beginning of the game
    local assetData = {
        PlayerPlaneDataTable = {
            ["PackageName"] = UEHelpers.FindOrAddFName("/Game/Blueprint/Information/PlayerPlaneDataTable"),
            ["AssetName"] = UEHelpers.FindOrAddFName("PlayerPlaneDataTable")
        }
    }

    for tableName, data in pairs(assetData) do
        local obj = StaticFindObject(Settings.DataTableClassNames.PlayerPlaneDataTable)
        if obj and obj:IsValid() then
            Utils.Log(string.format("Asset already loaded, skipping: %s\n", Utils.PrintTable(data)), "main",
                "LoadDataTableAssets")
        else
            Utils.Log(string.format("Loading data table asset for %s\n", tableName), "main", "LoadDataTableAssets")
            local assetClass = assetRegistryHelpers:GetAsset(data)
            if not assetClass:IsValid() then
                Utils.Log(string.format("Failed to load data table asset for %s\n", tableName), "main",
                    "LoadDataTableAssets")
            else
                Utils.Log(
                    string.format("Successfully loaded data table asset for %s: %s\n", tableName,
                        assetClass:GetFullName()),
                    "main",
                    "LoadDataTableAssets")
                -- Adding to the GameInstance's ReferencedObjects so it doesn't get gargabe colllected
                local gameInstance = UEHelpers.GetGameInstance()
                local numRefObjects = gameInstance.ReferencedObjects:GetArrayNum()
                gameInstance.ReferencedObjects[numRefObjects + 1] = assetClass
            end
        end
    end
end

-- Providing the DLL part of the mod with the required data table data
local function InitDataTables()
    for tableName, details in pairs(Settings.DataTables) do
        ConfigureDataTables(tableName, details.Path, details.SourceRow)
    end
end

RegisterKeyBind(Key.D, {ModifierKey.CONTROL}, function()
    Utils.Log("Dumping data tables")

    --NOTE: Using async calls here to don't lock up the game thread
    for name, handler in pairs(DataTableHandlers) do
        if name ~= "ValueHandler" then
            ExecuteAsync(function() handler:DumpDataTable() end)
        end
    end

    for _, handler in pairs(DataTableHandlers.ValueHandler or {}) do
        if IsDataTableValid(handler.__table) then
            ExecuteAsync(function() handler:DumpDataTable() end)
        end
    end

    return true
end)

ExecuteInGameThread(function()
    -- Create mod dir and mod child dirs if not present and collect data from mod child dirs
    local modDir = FindOrCreateModDir()
    CreateModChildDirs(modDir)
    for dirName, dir in pairs(modDir.DataTable or {}) do
        if not (dirName == "Dumps") then
            DataCollections[dirName] = CollectData(dir)
        end
    end

    LoadDataTableAssets()
    InitDataTables()

    -- Initialize data table handlers
    for name, class in pairs(DataTableClasses) do
        if name ~= "ValueHandler" then
            local dataTable = StaticFindObject(Settings.DataTableClassNames[name])
            if IsDataTableValid(dataTable) then
                DataTableHandlers[name] = class.new(dataTable)
            end
        end
    end

    for _, path in ipairs(Settings.ValueTables) do
        local dataTable = StaticFindObject(path)
        if IsDataTableValid(dataTable) then
            local dataTableName = Utils.GetDataTableName(dataTable)
            DataTableHandlers.ValueHandler[dataTableName] = ValueHandler.new(dataTable)
        end
    end

    -- NOTE: These Add/Replace/Remove function calls could potentially also be called asynchronously
    -- if there are ever any "performance" concerns. Though it's more a user experience thing.
    -- As a large amount of items could cause the game thread to be locked up. Meaning the game would
    -- take a bit longer to load into the main menu.

    DataProcessor.ProcessFull(
        DataCollections.PlayerPlaneDataTable,
        DataTableHandlers.PlayerPlaneDataTable
    )
end)
