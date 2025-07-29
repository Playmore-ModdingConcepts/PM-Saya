#pragma once

#include "Unreal/UObject.hpp"
#include "Core/Containers/Array.hpp"
#include "SDK/Classes/Custom/USCS_Node.h"

namespace UECustom {
    class USimpleConstructionScript : public RC::Unreal::UObject {
    public:
        RC::Unreal::TArray<USCS_Node*>& GetAllNodes();
    };
}