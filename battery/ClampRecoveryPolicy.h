#pragma once

#include <climits>

namespace peridot_charging {

class ClampRecoveryPolicy {
  public:
    explicit ClampRecoveryPolicy(long recoveryTempDeciC)
        : recoveryTempDeciC_(recoveryTempDeciC) {}

    void reset() {
        lastTarget_ = LONG_MIN;
        clamped_ = false;
        retryConsumed_ = false;
    }

    void observeTarget(long target) {
        if (target == lastTarget_) {
            return;
        }
        lastTarget_ = target;
        clamped_ = false;
        retryConsumed_ = false;
    }

    void recordReadback(long target, long readback) {
        observeTarget(target);
        clamped_ = target > 0 && readback >= 0 && readback < target;
        if (!clamped_) {
            retryConsumed_ = false;
        }
    }

    bool shouldForceRetry(long target, long tempDeciC) {
        observeTarget(target);
        if (!clamped_ || retryConsumed_ || tempDeciC > recoveryTempDeciC_) {
            return false;
        }
        retryConsumed_ = true;
        return true;
    }

  private:
    const long recoveryTempDeciC_;
    long lastTarget_ = LONG_MIN;
    bool clamped_ = false;
    bool retryConsumed_ = false;
};

}  // namespace peridot_charging
