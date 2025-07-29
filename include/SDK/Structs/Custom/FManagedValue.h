#pragma once

namespace UECustom {
    // This struct takes in a void pointer for an allocated value and automatically disposes of it in the destructor.
    // This means that the value contained is not permanent and you should utilize UE functions that make a copy of the value.
    // e.g FMemory::Memcpy
    struct FManagedValue {
    public:
        FManagedValue() {};

        FManagedValue(void* InData);

        ~FManagedValue();

        void Copy(void* InData);

        void* GetData();
    private:
        void* Data = nullptr;
    };
}