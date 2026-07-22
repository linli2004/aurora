#include "AuroraStateMapper.h"
#include "AuroraTypes.h"

#include <algorithm>

AuroraStateMapper::AuroraStateMapper(QObject *parent)
    : QObject(parent)
{
}

int AuroraStateMapper::momentExperienceState(
    int lifecycle,
    double memoryConfidence,
    int availability) const
{
    using namespace Aurora;

    if (availability == AvailabilityDetached || availability == AvailabilityUnavailable) {
        return MomentDetached;
    }

    if (availability == AvailabilityArchived || lifecycle == MomentArchivedLifecycle) {
        return MomentArchived;
    }

    const double confidence = std::clamp(memoryConfidence, 0.0, 1.0);

    switch (lifecycle) {
    case MomentCreated:
        return MomentDormant;
    case MomentCandidate:
        return confidence >= 0.5 ? MomentPresent : MomentDormant;
    case MomentObserved:
        return MomentPresent;
    case MomentRememberedLifecycle:
        return MomentRemembered;
    case MomentMeaningfulLifecycle:
        return MomentMeaningful;
    case MomentRecalled:
        return MomentRecalling;
    case MomentArchivedLifecycle:
        return MomentArchived;
    default:
        return MomentDormant;
    }
}

bool AuroraStateMapper::ambientMotionAllowed(
    int accessibilityMode,
    int motionMode,
    int qualityMode) const
{
    using namespace Aurora;

    if (accessibilityMode == ReducedMotion || accessibilityMode == CognitiveMinimal) {
        return false;
    }

    if (motionMode == MotionReduced || motionMode == MotionStatic) {
        return false;
    }

    return qualityMode != Eco;
}
