#include "SDK/Classes/KismetSystemLibrary.h"
#include "Unreal/UFunction.hpp"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace UECustom {
	FSoftObjectPath UKismetSystemLibrary::MakeSoftObjectPath(const FString& Path)
	{
		static auto Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, TEXT("/Script/Engine.KismetSystemLibrary:MakeSoftObjectPath"));

        if (!Function)
        {
            PS::Log<LogLevel::Verbose>(STR("Function /Script/Engine.KismetSystemLibrary:MakeSoftObjectPath was invalid.\n"));
            return {};
        }

		struct {
			FString Path;
			FSoftObjectPath ReturnValue;
		}params;

		params.Path = Path;

		GetDefaultObj()->ProcessEvent(Function, &params);

		return params.ReturnValue;
	}

    RC::Unreal::UObject* UKismetSystemLibrary::LoadAsset_Blocking(UECustom::TSoftObjectPtr<UObject> Asset)
    {
        static auto Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, TEXT("/Script/Engine.KismetSystemLibrary:LoadAsset_Blocking"));

        if (!Function)
        {
            PS::Log<LogLevel::Verbose>(STR("Function /Script/Engine.KismetSystemLibrary:LoadAsset_Blocking was invalid.\n"));
            return nullptr;
        }

        struct {
            UECustom::TSoftObjectPtr<UObject> Asset;
            UObject* ReturnValue;
        }params;

        params.Asset = Asset;

        GetDefaultObj()->ProcessEvent(Function, &params);

        return params.ReturnValue;
    }

	UKismetSystemLibrary* UKismetSystemLibrary::GetDefaultObj()
	{
		static auto Self = UObjectGlobals::StaticFindObject<UKismetSystemLibrary*>(nullptr, nullptr, TEXT("/Script/Engine.Default__KismetSystemLibrary"));
		return Self;
	}
}