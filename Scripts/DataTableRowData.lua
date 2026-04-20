-- An abstraction to be able to modify nested properties of a row's data.
-- For example adding and element to a list/map. More concrete the game's
-- vendors have a set of maps for the different items they are selling.
-- This abstraction allows for adding new items to those, removing a specific
-- one or modifying one within them.

local Utils = require("utils")


local function Log(message, funcName)
    Utils.Log(message, "DataTableRowData", funcName)
end

local DataTableRowData = {
    Table = {}
}
DataTableRowData.__index = DataTableRowData

function DataTableRowData.new(dataTable)
    local self = setmetatable({}, DataTableRowData)
    self.Table = dataTable
    return self
end

local function GetPropertyPath(propertyName)
    local path = {}
    -- Split on '.'
    local pattern = string.format("([^%s]+)", ".")

    propertyName:gsub(pattern, function(c) table.insert(path, c) end)

    return path
end

local function WalkPropertyPath(data, path)
    local current = data
    for _, key in ipairs(path) do
        if type(current) ~= "table" or current[key] == nil then
            return nil
        end
        current = current[key]
    end

    return current
end

local function SetPropertyValue(object, value, path)
    local current = object

    -- Walk to the second-to-last key
    for i = 1, #path - 1 do
        local p = path[i]
        if current[p] ~= nil then
            current = current[p]
        end
    end

    current[path[#path]] = value

    return object
end

local function FindElementIndexByValue(t, value)
    if not Utils.IsArray(t) then
        Log("Table is not a list!", "FindElementIndexByValue")
        return nil
    end

    for i, v in ipairs(t) do
        -- Basic value types
        if type(v) ~= "table" then
            if v == value then
                Log(string.format("Element is basic value: %s\n", tostring(value)), "FindElementIndexByValue")
                return i
            end
            -- Value is a map
        elseif type(v) == "table" and #v == 0 then
            Log(string.format("Element is a map: %s\n", Utils.PrintTable(v)), "FindElementIndexByValue")
            local result = {}
            local propertyCount = 0
            for _, _ in pairs(v) do
                propertyCount = propertyCount + 1
            end

            for propertyName, propertyValue in pairs(v) do
                if propertyValue == value[propertyName] then
                    table.insert(result, true)
                end
            end

            local allMatch = true
            local resultCount = 0
            for _, result in ipairs(result) do
                resultCount = resultCount + 1
                if result == true and allMatch then
                    allMatch = true
                else
                    allMatch = false
                end
            end
            if allMatch and (resultCount == propertyCount) then
                return i
            end
        end
    end

    return nil
end

-- Function to add an element to a list/map
function DataTableRowData:AddTo(rowName, propertyName, newElements)
    local rowData = self.Table.__table:FindRow(rowName)
    if not rowData then
        Log(string.format("Failed to find row with name: %s\n", rowName), "AddTo")
        return
    end

    local propertyPath = GetPropertyPath(propertyName)
    local parsedRowData = self.Table:ParseRowData(rowData)

    Log(string.format("Property path: %s\n", Utils.PrintTable(propertyPath)), "ModifyIn")

    local propertyValue = nil
    if #propertyPath == 1 then
        propertyValue = parsedRowData[propertyPath[1]]
    else
        propertyValue = WalkPropertyPath(parsedRowData, propertyPath)
    end

    if Utils.IsArray(propertyValue) then
        for _, newValue in ipairs(newElements) do
            table.insert(propertyValue, newValue)
        end
    elseif Utils.IsMap(propertyValue) then
        for propertyName, newValue in pairs(newElements) do
            propertyValue[propertyName] = newValue
        end
    else
        return
    end

    parsedRowData = SetPropertyValue(parsedRowData, propertyValue, propertyPath)

    self.Table:AddRow(rowName, parsedRowData)
    Log(string.format("Added element(s) to %s - %s\n", rowName, propertyName), "AddTo")
end

-- Function to modify an element of a map
-- Doesn't support modifying lists (array) as currently the mod doesn't sort the output & input lists.
-- Once sorting of lists is done. It would be possible to provide the index of the element that should
-- be modified. Though not sure if it's worth the hassle. As the same result could be achieved via a
-- RemoveFrom + AddTo.
function DataTableRowData:ModifyIn(rowName, propertyName, newElement)
    local rowData = self.Table.__table:FindRow(rowName)
    if not rowData then
        Log(string.format("Failed to find row with name: %s\n", rowName), "ModifyIn")
        return
    end

    local propertyPath = GetPropertyPath(propertyName)
    local parsedRowData = self.Table:ParseRowData(rowData)

    Log(string.format("Property path: %s\n", Utils.PrintTable(propertyPath)), "ModifyIn")

    local propertyValue = nil
    if #propertyPath == 1 then
        propertyValue = parsedRowData[propertyPath[1]]
    else
        propertyValue = WalkPropertyPath(parsedRowData, propertyPath)
    end

    if Utils.IsMap(propertyValue) then
        for k, v in pairs(newElement) do
            propertyValue[k] = v
        end
    else
        return
    end

    parsedRowData = SetPropertyValue(parsedRowData, propertyValue, propertyPath)

    self.Table:AddRow(rowName, parsedRowData)
    Log(string.format("Modified element(s) from %s - %s (%s)\n", rowName, propertyName, Utils.PrintTable(newElement)),
        "ModifyIn")
end

function DataTableRowData:RemoveFrom(rowName, propertyName, elementIds)
    local rowData = self.Table.__table:FindRow(rowName)
    if not rowData then
        Log(string.format("Failed to find row with name: %s\n", rowName), "RemoveFrom")
        return
    end

    local propertyPath = GetPropertyPath(propertyName)
    local parsedRowData = self.Table:ParseRowData(rowData)

    Log(string.format("Property path: %s\n", Utils.PrintTable(propertyPath)), "RemoveFrom")

    local propertyValue = nil
    if #propertyPath == 1 then
        propertyValue = parsedRowData[propertyPath[1]]
    else
        propertyValue = WalkPropertyPath(parsedRowData, propertyPath)
    end

    if Utils.IsMap(propertyValue) then
        for id, _ in pairs(elementIds) do
            propertyValue[id] = nil
        end
    elseif Utils.IsArray(propertyValue) then
        for _, valueToRemove in ipairs(elementIds) do
            local index = FindElementIndexByValue(propertyValue, valueToRemove)
            if index then
                Log(string.format("Found value in list\n"), "RemoveFrom")
                table.remove(propertyValue, index)
            end
        end
    end

    parsedRowData = SetPropertyValue(parsedRowData, propertyValue, propertyPath)

    self.Table:AddRow(rowName, parsedRowData)
    Log(string.format("Removed element(s) from %s - %s (%s)\n", rowName, propertyName, Utils.PrintTable(elementIds)),
        "RemoveFrom")
end

return DataTableRowData
