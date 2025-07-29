#pragma once

#include "Unreal/UClass.hpp"
#include "SDK/Classes/Custom/USimpleConstructionScript.h"

namespace RC::Unreal {
    class UObject;
}

namespace UECustom {
    class UInheritableComponentHandler;

    class UBlueprintGeneratedClass : public RC::Unreal::UClass {
    public:
        UInheritableComponentHandler* GetInheritableComponentHandler();

        USimpleConstructionScript* GetSimpleConstructionScript();
    };
}