#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

namespace Palworld {
    class SignatureManager {
    public:
        static void Initialize();
        
        // Expected parameter format: [CLASS]::[FUNCTION] or [FUNCTION], for example AGameModeBase::InitGameState or AsyncTask
        static void* GetSignature(const std::string& ClassAndFunction);
    private:
        static inline std::unordered_map<std::string, void*> SignatureMap;

        static inline std::unordered_map<std::string, std::string> Signatures {
            { "FPakPlatformFile::GetPakFolders", "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC ?0 48 8D 4D ?0 48 8B DA E8" },
            // Important so we can easily run things on the Game Thread
            { "AsyncTask", "48 8B C4 41 54 41 57 48 81 EC B8 00 00 00 48 89 58 08" },
        };
        static inline std::unordered_map<std::string, std::string> SignaturesCallResolve {
            // FMemory::Free for GMalloc
            { "FMemory::Free", "E8 ?? ?? ?? ?? 48 8D ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B 4C 24 70 48 33 CC E8 ?? ?? ?? ?? 4C 8D 9C 24 80 00 00 00" },
        };
    };
}