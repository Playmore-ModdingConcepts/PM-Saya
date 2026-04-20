--- Global settings and configuration for TFWData mod
--- This module provides centralized configuration for debug flags and data table paths
--- 
--- TODO: See if it makes sense to remove the duplicated data table names/paths

local Settings = {}

-- Data table class names/paths
Settings.DataTableClassNames = {
	PlayerPlaneDataTable = "/Game/Blueprint/Information/PlayerPlaneDataTable.PlayerPlaneDataTable",
}
Settings.ValueTables = {
}

-- Required data of data tables who need to call the AddDataTableRow of the mod's DLL
Settings.DataTables = {
	PlayerPlaneDataTable = {
        Path = "/Game/Blueprint/Information/PlayerPlaneDataTable.PlayerPlaneDataTable",
		SourceRow = "Row_100"
    }
}

Settings.ModChildDirs = {
    DataTable = {
		"PlayerPlaneDataTable",
        "Dumps"
    }
}

return Settings
