#include "Unreal/FProperty.hpp"
#include "Unreal/Property/FArrayProperty.hpp"
#include "Unreal/Property/FBoolProperty.hpp"
#include "Unreal/Property/FNameProperty.hpp"
#include "Unreal/Property/FNumericProperty.hpp"
#include "Unreal/Property/FEnumProperty.hpp"
#include "Unreal/Property/FMapProperty.hpp"
#include "Unreal/Property/FStructProperty.hpp"
#include "Unreal/Property/FStrProperty.hpp"
#include "Unreal/Property/FClassProperty.hpp"
#include "Unreal/Property/FSoftClassProperty.hpp"
#include "Unreal/Property/FSoftObjectProperty.hpp"
#include "Unreal/Property/FTextProperty.hpp"
#include "Unreal/FString.hpp"
#include "Unreal/NameTypes.hpp"
#include "Unreal/UClass.hpp"
#include "Unreal/UEnum.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/GameplayStatics.hpp"
#include "Unreal/World.hpp"
#include "Utility/DataTableHelper.h"
#include "Utility/Logging.h"
#include "SDK/Classes/TSoftObjectPtr.h"
#include "SDK/Classes/Texture2D.h"
#include "SDK/Classes/TSoftClassPtr.h"
#include "SDK/Structs/FLinearColor.h"
#include "SDK/Classes/KismetSystemLibrary.h"

using namespace RC;
using namespace RC::Unreal;

void Palworld::DataTableHelper::CopyJsonValueToTableRow(void* TableRow, RC::Unreal::FProperty* Property, const nlohmann::json& Value)
{
	auto PropertyName = Property->GetName();
	auto PropertyValue = Property->ContainerPtrToValuePtr<void>(TableRow);
	auto Type = Property->GetCPPType();
	auto Class = Property->GetClass();
	auto ClassName = Class.GetName();

	if (auto EnumProperty = CastField<FEnumProperty>(Property))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));

		auto Enum = EnumProperty->GetEnum();
		if (!Enum)
		{
			throw std::runtime_error(std::format("EnumProperty {} had an invalid Enum value", RC::to_string(PropertyName)));
		}

		auto ParsedValue = Value.get<std::string>();
		if (!ParsedValue.contains("::"))
		{
			ParsedValue = std::format("{}::{}", RC::to_string(Type.GetCharArray()), ParsedValue);
		}

		auto EnumName = FName(RC::to_generic_string(ParsedValue));

		bool WasEnumFound = false;
		int64_t EnumValue = 0;

		for (const auto& EnumPair : Enum->GetNames())
		{
			if (EnumPair.Key == EnumName)
			{
				WasEnumFound = true;
				EnumValue = EnumPair.Value;
			}
		}

		if (!WasEnumFound)
		{
			throw std::runtime_error(std::format("Enum '{}' doesn't exist", ParsedValue));
		}

		FMemory::Memcpy(PropertyValue, &EnumValue, sizeof(uint8_t));
	}
	else if (auto NumProperty = CastField<FNumericProperty>(Property))
	{
        if (!NumProperty->IsEnum())
        {
            if (!Value.is_number()) throw std::runtime_error(std::format("Property {} must be a number", RC::to_string(PropertyName)));
        }

        if (NumProperty->IsEnum())
        {
            auto Enum = NumProperty->GetIntPropertyEnum();
            if (!Enum)
            {
                throw std::runtime_error(std::format("EnumProperty {} had an invalid Enum value", RC::to_string(PropertyName)));
            }

            auto ParsedValue = Value.get<std::string>();
            if (!ParsedValue.contains("::"))
            {
                ParsedValue = std::format("{}::{}", RC::to_string(Type.GetCharArray()), ParsedValue);
            }

            auto EnumName = FName(RC::to_generic_string(ParsedValue));

            bool WasEnumFound = false;
            int64_t EnumValue = 0;

            for (const auto& EnumPair : Enum->GetNames())
            {
                if (EnumPair.Key == EnumName)
                {
                    WasEnumFound = true;
                    EnumValue = EnumPair.Value;
                }
            }

            if (!WasEnumFound)
            {
                throw std::runtime_error(std::format("Enum '{}' doesn't exist", ParsedValue));
            }

            NumProperty->SetIntPropertyValue(PropertyValue, EnumValue);
        }
        else
        {
            if (NumProperty->IsInteger())
            {
                NumProperty->SetIntPropertyValue(PropertyValue, Value.get<int64>());
            }
            else if (NumProperty->IsFloatingPoint())
            {
                NumProperty->SetFloatingPointPropertyValue(PropertyValue, Value.get<double>());
            }
            else
            {
                PS::Log<RC::LogLevel::Warning>(STR("Unhandled Numeric Type: {}\n"), Property->GetName());
            }
        }
	}
	else if (auto BoolProperty = CastField<FBoolProperty>(Property))
	{
		if (!Value.is_boolean()) throw std::runtime_error(std::format("Property {} must be a boolean", RC::to_string(PropertyName)));
		BoolProperty->SetPropertyValue(PropertyValue, Value.get<bool>());
	}
	else if (auto NameProperty = CastField<FNameProperty>(Property))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<std::string>();
		auto Name = FName(RC::to_generic_string(ParsedValue), FNAME_Add);
		NameProperty->SetPropertyValue(PropertyValue, Name);
	}
	else if (auto StrProperty = CastField<FStrProperty>(Property))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<std::string>();
		auto String = FString(RC::to_generic_string(ParsedValue).c_str());
		StrProperty->SetPropertyValue(PropertyValue, String);
	}
	else if (auto TextProperty = CastField<FTextProperty>(Property))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<std::string>();
		auto Text = FText(RC::to_generic_string(ParsedValue).c_str());
		TextProperty->SetPropertyValue(PropertyValue, Text);
	}
	else if (auto ClassProperty = CastField<FClassProperty>(Property))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = RC::to_generic_string(Value.get<std::string>());
        auto SoftObjectPtr = UECustom::TSoftObjectPtr<UObject>(UECustom::FSoftObjectPath(ParsedValue));
        auto Asset = UECustom::UKismetSystemLibrary::LoadAsset_Blocking(SoftObjectPtr);

        if (!Asset)
        {
            throw std::runtime_error(std::format("Property {} was supplied an invalid class of {}", RC::to_string(PropertyName), RC::to_string(ParsedValue)));
        }

        // Not ideal, but right now I don't have a better solution for keeping the class loaded.
        Asset->SetRootSet();

        void* ValuePtr = ClassProperty->ContainerPtrToValuePtr<void>(TableRow);
        ClassProperty->SetPropertyValue(ValuePtr, Asset);
	}
	else if (auto ObjectProperty = CastField<FObjectProperty>(Property) && ClassName == STR("ObjectProperty"))
	{
		if (!Value.is_object()) throw std::runtime_error(std::format("Property {} must be an object", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<nlohmann::json>();

        auto ObjectValue = *Property->ContainerPtrToValuePtr<UObject*>(static_cast<void*>(TableRow));
        if (ObjectValue)
        {
            for (auto& [InnerKey, InnerValue] : Value.items())
            {
                auto ObjectValue_PropertyName = RC::to_generic_string(InnerKey);
                auto ObjectValue_Property = ObjectValue->GetPropertyByNameInChain(ObjectValue_PropertyName.c_str());
                if (ObjectValue_Property)
                {
                    Palworld::DataTableHelper::CopyJsonValueToTableRow(static_cast<void*>(ObjectValue), ObjectValue_Property, InnerValue);
                }
            }
        }
	}
	else if (auto SoftObjectProperty = CastField<FSoftObjectProperty>(Property) && ClassName == STR("SoftObjectProperty"))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<std::string>();
		auto String = RC::to_generic_string(ParsedValue);

		auto SoftObjectPtr = UECustom::TSoftObjectPtr<UObject>(UECustom::FSoftObjectPath(String));
		FMemory::Memcpy(PropertyValue, &SoftObjectPtr, sizeof(UECustom::TSoftObjectPtr<UObject>));
	}
	else if (auto SoftClassProperty = CastField<FSoftClassProperty>(Property) && ClassName == STR("SoftClassProperty"))
	{
		if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<std::string>();
		auto String = RC::to_generic_string(ParsedValue);

		if (!String.ends_with(STR("_C"))) String += STR("_C");

		auto SoftClassPtr = UECustom::TSoftClassPtr<UClass>(UECustom::FSoftObjectPath(String));
		FMemory::Memcpy(Property->ContainerPtrToValuePtr<void>(TableRow), &SoftClassPtr, sizeof(UECustom::TSoftClassPtr<UClass>));
	}
	else if (auto StructProperty = CastField<FStructProperty>(Property))
	{
		if (!Value.is_object()) throw std::runtime_error(std::format("Property {} must be an object", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<nlohmann::json>();

		auto StructMemory = StructProperty->ContainerPtrToValuePtr<void>(TableRow);
		if (!StructMemory)
		{
			throw std::runtime_error(std::format("Failed to access Struct Memory"));
		}

		auto Struct = StructProperty->GetStruct();
		if (!Struct)
		{
			throw std::runtime_error(std::format("Failed to get Struct"));
		}

		for (FProperty* InnerProperty : Struct->ForEachProperty())
		{
			auto StructPropertyName = RC::to_string(InnerProperty->GetName());
			if (Value.contains(StructPropertyName))
			{
				CopyJsonValueToTableRow(StructMemory, InnerProperty, Value.at(StructPropertyName));
			}
		}
	}
	else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
	{
		if (!Value.is_object() && !Value.is_array()) throw std::runtime_error(std::format("Property {} must be an object or array", RC::to_string(PropertyName)));
		auto ParsedValue = Value.get<nlohmann::json>();

		auto ArrayContainer = ArrayProperty->ContainerPtrToValuePtr<void>(TableRow);
		if (!ArrayContainer)
		{
			throw std::runtime_error(std::format("Failed to access Array Container"));
		}

		auto ScriptArray = static_cast<FScriptArray*>(ArrayContainer);
		auto InnerProperty = ArrayProperty->GetInner();
		auto InnerPropertyName = RC::to_string(InnerProperty->GetName());
		auto ElementSize = InnerProperty->GetElementSize();
		auto ElementAlignment = InnerProperty->GetMinAlignment();

        if (Value.is_object())
        {
            if (Value.contains("Action"))
            {
                auto Action = Value.at("Action").get<std::string>();
                if (Action == "Clear")
                {
                    for (int32 Index = 0; Index < ScriptArray->Num(); ++Index)
                    {
                        void* ElementPtr = static_cast<uint8*>(ScriptArray->GetData()) + Index * ElementSize;
                        InnerProperty->DestroyValue(ElementPtr);
                    }

                    ScriptArray->Empty(0, ElementSize, ElementAlignment);
                }
            }

            if (Value.contains("Items"))
            {
                if (!Value.at("Items").is_array())
                {
                    throw std::runtime_error(std::format("Field Items must be an array"));
                }

                auto Items = Value.at("Items").get<nlohmann::json::array_t>();
                auto NumItems = Items.size();

                int32 FirstIndex = ScriptArray->Add(NumItems, ElementSize, ElementAlignment);
                uint8* DataPtr = static_cast<uint8*>(ScriptArray->GetData());
                for (int32 i = 0; i < NumItems; ++i)
                {
                    void* NewElementPtr = DataPtr + (FirstIndex + i) * ElementSize;
                    InnerProperty->InitializeValue(NewElementPtr);
                    CopyJsonValueToTableRow(NewElementPtr, InnerProperty, Items.at(i));
                }
            }
        }
        else if (Value.is_array())
        {
            auto Items = Value.get<nlohmann::json::array_t>();
            auto NumItems = Items.size();

            for (int32 Index = 0; Index < ScriptArray->Num(); ++Index)
            {
                void* ElementPtr = static_cast<uint8*>(ScriptArray->GetData()) + Index * ElementSize;
                InnerProperty->DestroyValue(ElementPtr);
            }
            ScriptArray->Empty(0, ElementSize, ElementAlignment);

            int32 FirstIndex = ScriptArray->Add(NumItems, ElementSize, ElementAlignment);
            uint8* DataPtr = static_cast<uint8*>(ScriptArray->GetData());
            for (int32 i = 0; i < NumItems; ++i)
            {
                void* NewElementPtr = DataPtr + (FirstIndex + i) * ElementSize;
                InnerProperty->InitializeValue(NewElementPtr);
                CopyJsonValueToTableRow(NewElementPtr, InnerProperty, Items.at(i));
            }
        }
	}
	else
	{
		PS::Log<RC::LogLevel::Warning>(STR("Unhandled property '{}' with class of {} and type of {}\n"), PropertyName, ClassName, Type.GetCharArray());
	}
}