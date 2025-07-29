#pragma once

#include "Unreal/Property/FArrayProperty.hpp"
#include "SDK/Structs/Custom/FManagedValue.h"

namespace UECustom {
    struct FScriptArrayHelper {
    public:
        FScriptArrayHelper(void* InScriptArray, RC::Unreal::FArrayProperty* InArrayProperty);

        FScriptArrayHelper(RC::Unreal::FScriptArray* InScriptArray, RC::Unreal::FArrayProperty* InArrayProperty);

        void Add(void* Value);

        void Add(UECustom::FManagedValue& ValuePtr);

        // Returns true if an item was deleted, otherwise returns false in cases where a key didn't exist inside the TArray.
        bool RemoveAtIndex(RC::Unreal::int32 Index);

        void Empty();

        void InitializeValue(UECustom::FManagedValue& OutValuePtr);

        RC::Unreal::int32 GetElementSize();

        RC::Unreal::int32 GetMinAlignment();

        RC::Unreal::FProperty* GetInner();

        void ForEachElement(const std::function<void(void*)> Callback);
    private:
        RC::Unreal::FScriptArray* ScriptArray{};
        RC::Unreal::FArrayProperty* ArrayProperty{};
    };
}