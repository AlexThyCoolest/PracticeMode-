#include "main.hpp"

#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

MOD_EXTERN_FUNC void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();

    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    PaperLogger.info("Practice Mode+ setup complete!");
}

MOD_EXTERN_FUNC void late_load() noexcept {
    PaperLogger.info("Practice Mode+ loaded!");
}
