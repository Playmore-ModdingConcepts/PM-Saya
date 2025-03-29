#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

namespace Palworld {
    class SignatureManager {
    public:
        static void Initialize();
        
        // Example of expected parameter: UBlueprintGeneratedClass::PostLoadDefaultObject
        static void* GetSignature(const std::string& ClassAndFunction);
    private:
        static inline std::unordered_map<std::string, void*> SignatureMap;

        static inline std::unordered_map<std::string, std::string> Signatures {
            { "UBlueprintGeneratedClass::PostLoadDefaultObject", "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8D 99 58 03 00 00" },
            { "UDataTable::HandleDataTableChanged", "48 89 54 24 10 55 53 56 48 8D 6C 24 B9" },
            { "FPakPlatformFile::GetPakFolders", "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC 40 48 8D 4D F0 48 8B DA E8" },
            { "AsyncTask", "48 8B C4 41 54 41 57 48 81 EC B8 00 00 00 48 89 58 08" },
            { "AGameModeBase::InitGameState", "40 53 48 83 EC 20 48 8B 41 10 48 8B D9 48 8B 91 F0 02 00 00" },
        };
        static inline std::unordered_map<std::string, std::string> SignaturesCallResolve {
        };
    };
}