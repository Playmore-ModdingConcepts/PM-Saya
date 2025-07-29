#pragma once

#include "Unreal/UClass.hpp"

namespace UECustom {
    class UInheritableComponentHandler;

    class UBlueprintGeneratedClass : public RC::Unreal::UClass {
    public:
        UInheritableComponentHandler* GetInheritableComponentHandler();
    };
}