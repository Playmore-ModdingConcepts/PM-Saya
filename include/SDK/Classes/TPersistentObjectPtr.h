#pragma once

#include "SDK/Structs/FWeakObjectPtr.h"

namespace UECustom {
    template<typename TObjectID>
    class TPersistentObjectPtr
    {
    public:
        explicit TPersistentObjectPtr()
        {
        }

        explicit TPersistentObjectPtr(const TObjectID& InObjectID)
            : WeakPtr()
            , ObjectID(InObjectID)
        {
        }
    public:
        UECustom::FWeakObjectPtr WeakPtr;
        RC::Unreal::int32 TagAtLastTest;
        TObjectID ObjectID;
    public:
        class UObject* Get() const
        {
            return WeakPtr.Get();
        }
        class UObject* operator->() const
        {
            return WeakPtr.Get();
        }
    };
}