local DataTable = require("DataTable")
local Parser = require("DataTableParser")
local Utils = require("utils")


local function Log(message, funcName)
    Utils.Log(message, "SkinDataTable", funcName)
end

local SkinDataTable = setmetatable({}, {__index = DataTable})
SkinDataTable.__index = SkinDataTable

function SkinDataTable.new(dataTable)
    local self = DataTable.new(dataTable)
    setmetatable(self, SkinDataTable)
    return self
end

---@param data FInventoryDangly
function SkinDataTable:ParseRowData(data)
    local lib = self.__kismetlib
    return {
        SkinID = Parser.ToJson(data.SkinID, lib),
		PlaneStringID = Parser.ToJson(data.PlaneStringID, lib),
		SkinNo = Parser.ToJson(data.SkinNo, lib),
		SortNumber = Parser.ToJson(data.SortNumber, lib),
		bNoseEmblem = Parser.ToJson(data.bNoseEmblem, lib),
		bWingEmblem = Parser.ToJson(data.bWingEmblem, lib),
		bTailEmblem = Parser.ToJson(data.bTailEmblem, lib),
		PlaneReference = Parser.ToJson(data.PlaneReference, lib),
		DLCID = Parser.ToJson(data.DLCID, lib)
    }
end

function SkinDataTable:AddRow(name, data)
    ---@class FInventoryDangly
    local rowData = {
		SkinID = data["SkinID"],
		PlaneStringID = data["PlaneStringID"],
		SkinNo = data["SkinNo"],
		SortNumber = data["SortNumber"],
		bNoseEmblem = data["bNoseEmblem"],
		bWingEmblem = data["bWingEmblem"],
		bTailEmblem  = data["bTailEmblem"],
		PlaneReference = data["PlaneReference"],
		DLCID = data["DLCID"]
    }

    local success = pcall(function() AddDataTableRow("SkinDataTable", name, rowData) end)
    if not success then
        Log(string.format("Failed to add row %s\n", name), "AddRow")
    else
        Log(string.format("Added row %s\n", name), "AddRow")
    end
end

return SkinDataTable