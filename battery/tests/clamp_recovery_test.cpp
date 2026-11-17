#include "../ClampRecoveryPolicy.h"

#include <cassert>

using peridot_charging::ClampRecoveryPolicy;

int main() {
    ClampRecoveryPolicy policy(390);
    constexpr long kSuperFastUa = 10000000L;

    policy.observeTarget(kSuperFastUa);
    policy.recordReadback(kSuperFastUa, 1000000L);

    assert(!policy.shouldForceRetry(kSuperFastUa, 404));
    assert(!policy.shouldForceRetry(kSuperFastUa, 391));
    assert(policy.shouldForceRetry(kSuperFastUa, 390));
    assert(!policy.shouldForceRetry(kSuperFastUa, 380));

    policy.recordReadback(kSuperFastUa, kSuperFastUa);
    assert(!policy.shouldForceRetry(kSuperFastUa, 380));

    policy.recordReadback(kSuperFastUa, 1000000L);
    assert(policy.shouldForceRetry(kSuperFastUa, 380));
    policy.recordReadback(kSuperFastUa, 1000000L);
    assert(!policy.shouldForceRetry(kSuperFastUa, 370));

    policy.reset();
    policy.observeTarget(kSuperFastUa);
    policy.recordReadback(kSuperFastUa, 1000000L);
    assert(policy.shouldForceRetry(kSuperFastUa, 380));

    return 0;
}
