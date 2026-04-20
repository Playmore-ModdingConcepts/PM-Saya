local json = require("json")
local RowData = require("DataTableRowData")
local UEHelpers = require("UEHelpers")
local Utils = require("utils")


local function Log(message, funcName)
    Utils.Log(message, "DataTable", funcName)
end

local DataTable = {
    __table = nil,
    __name = "",
    __dumpFile = "",
    __kismetlib = nil,
    RowData = {}
}
DataTable.__index = DataTable

function DataTable.new(dataTable)
    local self = setmetatable({}, DataTable)
    self.__table = dataTable
    self.__name = Utils.GetDataTableName(dataTable)
    self.__kismetlib = UEHelpers.GetKismetSystemLibrary()
    self.RowData = RowData.new(self)

    local dirs = IterateGameDirectories()
    local modDirs = dirs.Game.Content.Paks["~DefaultMods"]["SAYA-R2"].DataTable
    if not modDirs then
        Log("No such directory Contents/Paks/~DefaultMods/SAYA-R2", "new")
    else
        self.__dumpFile = string.format("%s/Dumps/%s.json", modDirs.__absolute_path, self.__name)
        Log(string.format("DumpFile: %s\n", self.__dumpFile), "new")
    end

    return self
end

function DataTable:ParseRowData(data)
    Log("Function not implemented", "ParseRowData")
end

function DataTable:DumpDataTable()
    ---@class UDataTable
    local dataTable = self.__table
    local output = {}
    local file = io.open(self.__dumpFile, "w")

    dataTable:ForEachRow(function(rowName, rowData)
        output[rowName] = self:ParseRowData(rowData)
    end)

    if file then
        local success, encodedJson = pcall(function() return json.encode(output) end)
        if success then
            file:write(encodedJson)
            file:close()
            Log("Successfully wrote JSON file", "DumpDataTable")
        else
            file:close()
            Log("Failed to encode JSON", "DumpDataTable")
        end
    end
end

function DataTable:AddRow(name, data)
    Log("Function not implemented", "AddRow")
end

function DataTable:ReplaceRow(name, data)
    local row = self.__table:FindRow(name)
    if not row then
        Log(string.format("Failed to find row %s\n", name), "ReplaceRow")
        return
    end

    local parsedRow = self:ParseRowData(row)
    for k, v in pairs(data) do
        parsedRow[k] = v
    end

    self:AddRow(name, parsedRow)
    Log(string.format("Replaced row %s\n", name), "ReplaceRow")
end

function DataTable:RemoveRow(name)
    self.__table:RemoveRow(name)
    Log(string.format("Remove row %s\n", name), "RemoveRow")
end

return DataTable