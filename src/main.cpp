#include "main.hpp"

#include <algorithm>

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/PracticeViewController.hpp"
#include "GlobalNamespace/StandardLevelFailedController.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupData.hpp"
#include "HMUI/PercentSlider.hpp"
#include "scotland2/shared/modloader.h"

using namespace GlobalNamespace;

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

namespace {
constexpr float kMinimumPracticeSpeed = 0.05f;
constexpr float kMaximumPracticeSpeed = 2.00f;
constexpr int kPracticeSpeedSteps = 195;
constexpr float kSmartRetryRewindSeconds = 10.0f;
}

MAKE_HOOK_MATCH(
    PracticeViewControllerDidActivate,
    &PracticeViewController::DidActivate,
    void,
    PracticeViewController* self,
    bool firstActivation,
    bool addedToHierarchy,
    bool screenSystemEnabling
) {
    PracticeViewControllerDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if (!self || !self->_speedSlider) {
        return;
    }

    // Extend Beat Saber's native Practice speed control without replacing the UI.
    // 5% is slow enough for pattern study while 200% still leaves room for speed training.
    self->_speedSlider->minValue = kMinimumPracticeSpeed;
    self->_speedSlider->maxValue = kMaximumPracticeSpeed;
    self->_speedSlider->numberOfSteps = kPracticeSpeedSteps;

    PaperLogger.info("Practice speed range set to 5%-200%");
}

MAKE_HOOK_MATCH(
    StandardLevelFailedControllerHandleLevelFailed,
    &StandardLevelFailedController::HandleLevelFailed,
    void,
    StandardLevelFailedController* self
) {
    if (self && self->_standardLevelSceneSetupData) {
        auto* practiceSettings = self->_standardLevelSceneSetupData->get_practiceSettings();

        // A non-null PracticeSettings object means this attempt was launched from Practice mode.
        if (practiceSettings && self->_gameSongController && self->_gameSongController->_audioTimeSyncController) {
            const float failureTime = self->_gameSongController->_audioTimeSyncController->get_songTime();
            const float retryTime = std::max(0.0f, failureTime - kSmartRetryRewindSeconds);

            practiceSettings->set_startSongTime(retryTime);

            // Beat Saber already owns the restart/reset pipeline. Enabling its existing auto-restart
            // path lets it rebuild notes, scoring, callbacks, lighting, and song state normally.
            if (self->_initData) {
                self->_initData->autoRestart = true;
            }

            PaperLogger.info("Smart Retry: failure at {:.2f}s, restarting from {:.2f}s", failureTime, retryTime);
        }
    }

    StandardLevelFailedControllerHandleLevelFailed(self);
}

MOD_EXTERN_FUNC void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();

    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    PaperLogger.info("Practice Mode+ setup complete!");
}

MOD_EXTERN_FUNC void late_load() noexcept {
    INSTALL_HOOK(PaperLogger, PracticeViewControllerDidActivate);
    INSTALL_HOOK(PaperLogger, StandardLevelFailedControllerHandleLevelFailed);

    PaperLogger.info("Practice Mode+ loaded!");
}
