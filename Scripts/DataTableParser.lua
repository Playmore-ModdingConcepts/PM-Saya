local Utils = require("utils")


local function Log(message, funcName)
    Utils.Log(message, "DataTableParser", funcName)
end

-- PPDT(PlayerPlaneDataTable) Enums

local EPlaneTargetMode = {
    CampaignAndOnline = 0,
    VR = 1,
    FreeFlight = 2,
    EPlaneTargetMode_MAX = 3
}

local function ToJsonEPlaneTargetMode(targetMode)
    local planetargetmode = {
        [0] = "CampaignAndOnline",
        [1] = "VR",
        [2] = "FreeFlight",
        [3] = "EPlaneTargetMode_MAX"
    }
    Log(string.format("TargetMode: %s - %d\n", planetargetmode[targetMode], targetMode), "ToJsonEPlaneTargetMode")
    return planetargetmode[targetMode]
end

local EPlaneCategory = {
    Fighter = 0,
    Multirole = 1,
    Attacker = 2,
    Bomber = 3,
    EPlaneCategory_MAX = 4
}

local function ToJsonEPlaneCategory(planeCategory)
    local planecategories = {
        [0] = "Fighter",
        [1] = "Multirole",
        [2] = "Attacker",
        [3] = "Bomber",
        [4] = "EPlaneCategory_MAX"
    }
    Log(string.format("Category: %s - %d\n", planecategories[planeCategory], planeCategory), "ToJsonEPlaneCategory")
    return planecategories[planeCategory]
end

local EPlaneHangarSize = {
    PHS_Small = 0,
    PHS_Medium = 1,
    PHS_Large = 2,
    PHS_Giant = 3,
    PHS_SU30_X = 4,
    PHS_F15_X = 5,
    PHS_Mig31B = 6,
    PHS_Extra = 7,
    PHS_Dark = 8,
    PHS_ASFX = 9,
	PHS_FA27 = 10,
    PHS_FA44 = 11,
    PHS_Count = 12,
    PHS_MAX = 13
}

local function ToJsonEPlaneHangarSize(hangarSize)
    local planehangarsize = {
        [0] = "PHS_Small",
        [1] = "PHS_Medium",
        [2] = "PHS_Large",
        [3] = "PHS_Giant",
        [4] = "PHS_SU30_X",
		[5] = "PHS_F15_X",
        [6] = "PHS_Mig31B",
        [7] = "PHS_Extra",
        [8] = "PHS_Dark",
        [9] = "PHS_ASFX",
		[10] = "PHS_FA27",
        [11] = "PHS_FA44",
        [12] = "PHS_Count",
        [13] = "PHS_MAX"
    }
    Log(string.format("HangarSize: %s - %d\n", planehangarsize[hangarSize], hangarSize), "ToJsonEPlaneHangarSize")
    return planehangarsize[hangarSize]
end

local EPlaneIGCSize = {
    PIS_Small = 0,
    PIS_Medium = 1,
    PIS_Large = 2,
    PIS_Giant = 3,
    PIS_Special = 4,
    PIS_Dark = 5,
    PIS_ASFX = 6,
    PIS_FA27 = 7,
    PIS_FA44 = 8,
    PIS_MAX = 9
}

local function ToJsonEPlaneIGCSize(cutsceneSize)
    local planecgsize = {
        [0] = "PIS_Small",
        [1] = "PIS_Medium",
        [2] = "PIS_Large",
        [3] = "PIS_Giant",
        [4] = "PIS_Special",
		[5] = "PIS_Dark",
        [6] = "PIS_ASFX",
        [7] = "PIS_FA27",
        [8] = "PIS_FA44",
        [9] = "PIS_MAX"
    }
    Log(string.format("IGCSize: %s - %d\n", planecgsize[cutsceneSize], cutsceneSize), "ToJsonEPlaneIGCSize")
    return planecgsize[cutsceneSize]
end

local EGunCaliber = {
    GunCaliber20mm = 0,
    GunCaliber27mm = 1,
    GunCaliber30mm = 2,
    GunCaliberAvenger = 3,
    GunCaliberPGun = 4,
    EGunCaliber_MAX = 5
}

local function ToJsonEGunCaliber(caliberType)
    local guncaliber = {
        [0] = "GunCaliber20mm",
        [1] = "GunCaliber27mm",
        [2] = "GunCaliber30mm",
        [3] = "GunCaliberAvenger",
        [4] = "GunCaliberPGun",
		[5] = "EGunCaliber_MAX"
    }
    Log(string.format("GunCaliber: %s - %d\n", guncaliber[caliberType], caliberType), "ToJsonEGunCaliber")
    return guncaliber[caliberType]
end

-- PPDT(PlayerPlaneDataTable) Enums end

local function ToJson(value, kismetLib)
    -- That needs to be refactored, because this ToJson function will be pulled out into
    -- a dedicated modules for common parsing functionality
    if value == nil then
        Log("value == nil", "ToJson")
        return nil
    end

    local valueType = type(value)

    if valueType == "string" or valueType == "number" or valueType == "boolean" then
        Log(string.format("non-custom type: %s\n", valueType), "ToJson")
        return value
    elseif valueType == "userdata" then
        local str = tostring(value)
        Log(string.format("userdata type: %s\n", str), "ToJson")

        if str:match("TSoftClassPtr") then
            local success, result = pcall(function()
                return kismetLib:Conv_SoftClassReferenceToString(value)
            end)
            if success then
                Log(string.format("TSoftClassPtr: %s\n", result:ToString()), "ToJson")
            end
        elseif str:match("TSoftObjectPtr") then
            local success, result = pcall(function()
                return kismetLib:Conv_SoftObjectReferenceToString(value)
            end)
            if success then
                Log(string.format("TSoftObjectPtr: %s\n", result:ToString()), "ToJson")
                return result:ToString()
            end
        elseif str:match("UScriptStruct") then
            local result = {}
            value:ForEachProperty(function(property)
                local propertyName = property:GetFName():ToString()
                Log(string.format("UScriptStruct Property: %s\n", propertyName), "ToJson")
                result[propertyName] = ToJson(value[propertyName], kismetLib)
            end)
            return result
        elseif str:match("TArray") then
            local result = {}
            value:ForEach(function(index, element)
                result[index] = ToJson(element:get(), kismetLib)
                Log(string.format("TArray: %d - %s\n", index, tostring(result[index])), "ToJson")
            end)
            return result
        elseif str:match("TMap") then
            local result = {}
            value:ForEach(function(key, value)
                local k = key:get():ToString()
                result[k] = ToJson(value:get(), kismetLib)
                Log(string.format("TMap: %s - %s\n", k, value:get()))
            end)
            return result
        elseif str:match("UDataTable") then
            Log(string.format("UDataTable: %s\n", string.match(value:GetFullName(), "^DataTable%s+(.*)")), "ToJson")
            return string.match(value:GetFullName(), "^DataTable%s+(.*)")
        end

        --- Try ToString method for FName, FText, and FString
        local success, result = pcall(function() return value:ToString() end)
        if success and result then
            Log(string.format("ToString(): %s\n", result), "ToJson")
            return result
        end
    end
end

return {
	EPlaneTargetMode = EPlaneTargetMode,
    EPlaneCategory = EPlaneCategory,
	EPlaneHangarSize = EPlaneHangarSize,
	EPlaneIGCSize = EPlaneIGCSize,
	EGunCaliber = EGunCaliber,
    ToJson = ToJson,
	ToJsonEPlaneTargetMode = ToJsonEPlaneTargetMode,
    ToJsonEPlaneCategory = ToJsonEPlaneCategory,
	ToJsonEPlaneHangarSize = ToJsonEPlaneHangarSize,
	ToJsonEPlaneIGCSize = ToJsonEPlaneIGCSize,
	ToJsonEGunCaliber = ToJsonEGunCaliber
}