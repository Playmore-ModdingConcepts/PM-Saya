#pragma once

namespace RC::Unreal {
    class UScriptStruct;
}

namespace Palworld {
    // A wrapper for UScriptStruct that automatically allocates and deallocates memory for the struct data in constructor/destructor.
    // Should only be passed to functions that make a copy of the internal data, like UDataTable::AddRow.
    class FManagedStruct {
    public:
        FManagedStruct(RC::Unreal::UScriptStruct* Struct);

        ~FManagedStruct();

        void* GetData();
    private:
        void* m_data;
        RC::Unreal::UScriptStruct* m_struct;
    };
}