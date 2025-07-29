#pragma once

#include "Unreal/UObject.hpp"

namespace UECustom {
    class USCS_Node : public RC::Unreal::UObject {
    public:
        RC::Unreal::UObject* GetComponentTemplate();
    };
}