#include "SDK/Classes/Custom/UBlueprintGeneratedClass.h"
#include "SDK/Classes/Custom/UInheritableComponentHandler.h"

namespace UECustom {
    UInheritableComponentHandler* UBlueprintGeneratedClass::GetInheritableComponentHandler()
    {
        auto InheritableComponentHandler = 
            this->GetValuePtrByPropertyNameInChain<RC::Unreal::TObjectPtr<UInheritableComponentHandler>>(STR("InheritableComponentHandler"));

        if (!InheritableComponentHandler)
        {
            return nullptr;
        }

        return (*InheritableComponentHandler).Get();
    }

    USimpleConstructionScript* UBlueprintGeneratedClass::GetSimpleConstructionScript()
    {
        auto SimpleConstructionScript =
            this->GetValuePtrByPropertyNameInChain<RC::Unreal::TObjectPtr<USimpleConstructionScript>>(STR("SimpleConstructionScript"));

        if (!SimpleConstructionScript)
        {
            return nullptr;
        }

        return (*SimpleConstructionScript).Get();
    }
}