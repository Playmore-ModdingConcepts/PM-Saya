local Utils = require("utils")

local DataProcessor = {}

-- Standard actions that most handlers support
local function ProcessStandardActions(collectionData, handler, options)
    options = options or {}

    -- Add
    if options.AddFn then
        ExecuteWithDelay(200, function()
            options.AddFn(collectionData.Add or {}, handler)
        end)
    elseif options.SupportAdd ~= false then
        ExecuteWithDelay(200, function()
            for _, entry in ipairs(collectionData.Add or {}) do
                handler:AddRow(entry["Name"], entry["Data"])
            end
        end)
    end

    -- Replace
    if options.SupportReplace ~= false then
        ExecuteWithDelay(200, function()
            for _, entry in ipairs(collectionData.Replace or {}) do
                handler:ReplaceRow(entry["Name"], entry["Data"])
            end
        end)
    end

    -- Remove
    if options.RemoveFn then
        ExecuteWithDelay(200, function()
            options.RemoveFn(collectionData.Remove or {}, handler)
        end)
    elseif options.SupportRemove ~= false then
        ExecuteWithDelay(200, function()
            for _, entry in ipairs(collectionData.Remove or {}) do
                handler:RemoveRow(entry["Name"])
            end
        end)
    end
end

-- Row-level modification actions (AddTo, ModifyIn, RemoveFrom)
local function ProcessRowModifications(collectionData, handler)
    -- AddTo
    ExecuteWithDelay(200, function()
        for _, entry in ipairs(collectionData.AddTo or {}) do
            for propertyName, value in pairs(entry["Data"]) do
                handler.RowData:AddTo(entry["Name"], propertyName, value)
            end
        end
    end)

    -- ModifyIn
    ExecuteWithDelay(200, function()
        for _, entry in ipairs(collectionData.ModifyIn or {}) do
            for propertyName, value in pairs(entry["Data"]) do
                handler.RowData:ModifyIn(entry["Name"], propertyName, value)
            end
        end
    end)

    -- RemoveFrom
    ExecuteWithDelay(200, function()
        for _, entry in ipairs(collectionData.RemoveFrom or {}) do
            for propertyName, value in pairs(entry["Data"]) do
                handler.RowData:RemoveFrom(entry["Name"], propertyName, value)
            end
        end
    end)
end

-- Process a data collection that supports all actions
function DataProcessor.ProcessFull(collectionData, handler, options)
    if not collectionData then return end
    ProcessStandardActions(collectionData, handler, options)
    ProcessRowModifications(collectionData, handler)
end

-- Process a data collection that supports basic actions (Add, Replace, Remove)
function DataProcessor.ProcessBasic(collectionData, handler, options)
    if not collectionData then return end
    ProcessStandardActions(collectionData, handler, options)
end

-- Process with dynamic handler lookup (like ItemValue)
function DataProcessor.ProcessWithDynamicHandler(collectionData, handlerLookup)
    if not collectionData then return end

    local operations = {"Add", "Replace", "Remove"}
    local methods = {Add = "AddRow", Replace = "ReplaceRow", Remove = "RemoveRow"}

    for _, op in ipairs(operations) do
        ExecuteWithDelay(200, function()
            for _, entry in pairs(collectionData[op] or {}) do
                local dataTableName = Utils.GetDataTableName(entry["Data"]["DataTable"])
                local handler = handlerLookup[dataTableName]
                if handler then
                    if op == "Remove" then
                        handler[methods[op]](handler, entry["Name"])
                    else
                        handler[methods[op]](handler, entry["Name"], entry["Data"])
                    end
                end
            end
        end)
    end
end


return DataProcessor