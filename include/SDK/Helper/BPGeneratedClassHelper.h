#pragma once

#include "Unreal/NameTypes.hpp"

namespace RC::Unreal {
    class UObject;
}

namespace UECustom::BPGeneratedClassHelper {
    RC::Unreal::UObject* FindComponentTemplateByName(RC::Unreal::UObject* BPGeneratedClass, RC::Unreal::FName TemplateName);

    bool GetGeneratedClassesHierarchy(RC::Unreal::UClass* InClass, RC::Unreal::TArray<RC::Unreal::UObject*>& OutBPGClasses);
}