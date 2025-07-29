#include "SDK/Classes/Custom/UInheritableComponentHandler.h"

using namespace RC;
using namespace RC::Unreal;

namespace UECustom {
    TArray<FComponentOverrideRecord> UECustom::UInheritableComponentHandler::GetRecords()
    {
        return *this->GetValuePtrByPropertyNameInChain<TArray<FComponentOverrideRecord>>(STR("Records"));
    }
}