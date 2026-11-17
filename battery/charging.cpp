/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "peridot-charging"

#include <android-base/properties.h>
#include <android/log.h>

#include "ClampRecoveryPolicy.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace {

constexpr const char* kModeNode = "/sys/class/qcom-battery/fastcharge_enable";
constexpr const char* kBypassNode = "/sys/class/qcom-battery/bypass_charging_enable";
constexpr const char* kCurrentNode = "/sys/class/power_supply/battery/constant_charge_current";
constexpr const char* kCurrentNowNode = "/sys/class/power_supply/battery/current_now";
constexpr const char* kStatusNode = "/sys/class/power_supply/battery/status";
constexpr const char* kTempNode = "/sys/class/power_supply/battery/temp";
constexpr const char* kModeProperty = "persist.vendor.peridot.charge_mode";

constexpr int kModeCount = 3;
constexpr long kRequestedCurrentUa[kModeCount] = {
    3000000L,
    6000000L,
    10000000L,
};
constexpr long kRecoveryTempDeciC = 390;
constexpr long kWarmTempDeciC = 420;
constexpr long kHotTempDeciC = 450;
constexpr int kTickSeconds = 2;
constexpr int kReapplyTicks = 5;

std::atomic_bool gStop(false);

void signalHandler(int) {
    gStop.store(true);
}

bool readString(const char* path, std::string* value) {
    std::ifstream file(path);
    if (!file.is_open() || !std::getline(file, *value)) {
        return false;
    }
    while (!value->empty() &&
            (value->back() == '\n' || value->back() == '\r' || value->back() == ' ')) {
        value->pop_back();
    }
    return true;
}

bool readLong(const char* path, long* value) {
    std::string raw;
    if (!readString(path, &raw) || raw.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    long parsed = strtol(raw.c_str(), &end, 10);
    if (errno != 0 || end == raw.c_str()) {
        return false;
    }
    *value = parsed;
    return true;
}

bool writeLong(const char* path, long value) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << value;
    file.flush();
    return !file.fail();
}

class CachedNode {
  public:
    explicit CachedNode(const char* path) : path_(path) {}

    bool set(long target, long* readback, bool force = false) {
        long current;
        if (readLong(path_, &current)) {
            if (current == target ||
                    (!force && target == lastTarget_ && current == lastReadback_)) {
                *readback = current;
                return true;
            }
        }
        if (!writeLong(path_, target)) {
            LOGW("write %ld to %s failed: %s", target, path_, strerror(errno));
            return false;
        }
        long observed = target;
        readLong(path_, &observed);
        lastTarget_ = target;
        lastReadback_ = observed;
        *readback = observed;
        if (observed != target) {
            LOGI("%s requested %ld, firmware reports %ld", path_, target, observed);
        } else {
            LOGI("%s = %ld", path_, target);
        }
        return true;
    }

  private:
    const char* path_;
    long lastTarget_ = LONG_MIN;
    long lastReadback_ = LONG_MIN;
};

int sanitizeMode(int mode) {
    return mode >= 0 && mode < kModeCount ? mode : 1;
}

bool bypassActive() {
    std::string state;
    if (!readString(kBypassNode, &state)) {
        return false;
    }
    return state != "0";
}

bool chargingActive() {
    std::string status;
    return readString(kStatusNode, &status) &&
            (status == "Charging" || status == "Full");
}

long safeRequestedCurrent(int mode) {
    long target = kRequestedCurrentUa[sanitizeMode(mode)];
    long temp;
    if (!readLong(kTempNode, &temp)) {
        return std::min(target, kRequestedCurrentUa[1]);
    }
    if (temp >= kHotTempDeciC) {
        return std::min(target, kRequestedCurrentUa[0]);
    }
    if (temp >= kWarmTempDeciC) {
        return std::min(target, kRequestedCurrentUa[1]);
    }
    return target;
}

class ChargingGovernor {
  public:
    ChargingGovernor()
        : desiredMode_(sanitizeMode(android::base::GetIntProperty(kModeProperty, 1))),
          currentNode_(kCurrentNode),
          recoveryPolicy_(kRecoveryTempDeciC) {}

    void tick() {
        bool bypass = bypassActive();
        if (bypass) {
            if (!wasBypass_) {
                LOGI("bypass active; releasing charging-speed current request");
            }
            wasBypass_ = true;
            long ignored;
            currentNode_.set(-1, &ignored);
            recoveryPolicy_.reset();
            return;
        }
        if (wasBypass_) {
            LOGI("bypass inactive; restoring charging-speed mode %d", desiredMode_);
            restoredMode_ = false;
            wasBypass_ = false;
        }

        synchronizeMode();

        if (!chargingActive()) {
            if (wasCharging_) {
                LOGI("charging stopped; releasing current request");
            }
            wasCharging_ = false;
            reapplyCounter_ = 0;
            long ignored;
            currentNode_.set(-1, &ignored);
            recoveryPolicy_.reset();
            return;
        }

        if (!wasCharging_) {
            LOGI("charging started with mode %d", desiredMode_);
            wasCharging_ = true;
            reapplyCounter_ = kReapplyTicks;
        }

        if (++reapplyCounter_ >= kReapplyTicks) {
            reapplyCounter_ = 0;
            if (!writeLong(kModeNode, desiredMode_)) {
                LOGW("reapply mode %d failed: %s", desiredMode_, strerror(errno));
            }
        }

        long target = safeRequestedCurrent(desiredMode_);
        long temp;
        bool hasTemp = readLong(kTempNode, &temp);
        bool forceRetry = hasTemp && recoveryPolicy_.shouldForceRetry(target, temp);
        if (forceRetry) {
            LOGI("battery cooled to %.1fC; retrying clamped target %lduA once",
                    temp / 10.0, target);
        }

        long readback = LONG_MIN;
        if (currentNode_.set(target, &readback, forceRetry)) {
            recoveryPolicy_.recordReadback(target, readback);
        }

        long currentNow;
        if (readLong(kCurrentNowNode, &currentNow) && hasTemp) {
            LOGI("mode=%d target=%lduA readback=%lduA current_now=%lduA temp=%.1fC",
                    desiredMode_, target, readback, currentNow, temp / 10.0);
        }
    }

    void release() {
        long ignored;
        currentNode_.set(-1, &ignored);
        recoveryPolicy_.reset();
    }

  private:
    void synchronizeMode() {
        if (!restoredMode_) {
            if (writeLong(kModeNode, desiredMode_)) {
                restoredMode_ = true;
                LOGI("restored persisted mode %d", desiredMode_);
            }
            return;
        }

        long observed;
        if (!readLong(kModeNode, &observed) || observed < 0 || observed >= kModeCount) {
            return;
        }
        if (observed == desiredMode_) {
            return;
        }

        desiredMode_ = static_cast<int>(observed);
        if (!android::base::SetProperty(kModeProperty, std::to_string(desiredMode_))) {
            LOGW("failed to persist mode %d", desiredMode_);
        }
        LOGI("user selected mode %d", desiredMode_);
        reapplyCounter_ = kReapplyTicks;
    }

    int desiredMode_;
    CachedNode currentNode_;
    peridot_charging::ClampRecoveryPolicy recoveryPolicy_;
    bool restoredMode_ = false;
    bool wasCharging_ = false;
    bool wasBypass_ = false;
    int reapplyCounter_ = 0;
};

}  // namespace

int main() {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    ChargingGovernor governor;
    while (!gStop.load()) {
        governor.tick();
        std::this_thread::sleep_for(std::chrono::seconds(kTickSeconds));
    }
    governor.release();
    return 0;
}
