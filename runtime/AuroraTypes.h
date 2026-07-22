#pragma once

#include <QObject>

namespace Aurora
{
Q_NAMESPACE

// Shared presentation contexts.
enum Context {
    Home,
    Moment,
    MemoryFlow,
    Recall,
    MusicSpace,
    Archive,
    Search,
    Queue
};
Q_ENUM_NS(Context)

enum AccessibilityMode {
    AccessibilityNormal,
    ReducedMotion,
    HighContrast,
    CognitiveMinimal
};
Q_ENUM_NS(AccessibilityMode)

enum QualityMode {
    Eco,
    Balanced,
    Immersive,
    Adaptive
};
Q_ENUM_NS(QualityMode)

enum MotionMode {
    MotionNormal,
    MotionReduced,
    MotionStatic
};
Q_ENUM_NS(MotionMode)

enum TransitionMode {
    TransitionNone,
    Expand,
    Collapse,
    Merge,
    Restore
};
Q_ENUM_NS(TransitionMode)

enum CoreExperienceState {
    CoreIdle,
    CorePlaying,
    CorePaused,
    CoreGathering,
    CoreTransition,
    CoreRecall
};
Q_ENUM_NS(CoreExperienceState)

enum CoreRuntimeState {
    CoreInitializing,
    CoreAnalyzing,
    CoreRendering,
    CoreTransitioning,
    CoreSleeping,
    CoreFailure,
    CoreRecovery
};
Q_ENUM_NS(CoreRuntimeState)

enum MomentLifecycle {
    MomentCreated,
    MomentCandidate,
    MomentObserved,
    MomentRememberedLifecycle,
    MomentMeaningfulLifecycle,
    MomentRecalled,
    MomentArchivedLifecycle
};
Q_ENUM_NS(MomentLifecycle)

enum MomentExperienceState {
    MomentDormant,
    MomentPresent,
    MomentRemembered,
    MomentMeaningful,
    MomentRecalling,
    MomentArchived,
    MomentDetached
};
Q_ENUM_NS(MomentExperienceState)

enum MomentRuntimeState {
    MomentUnbound,
    MomentBinding,
    MomentRendering,
    MomentRestoring,
    MomentReady,
    MomentFailure,
    MomentSleeping
};
Q_ENUM_NS(MomentRuntimeState)

enum MomentAvailability {
    AvailabilityActive,
    AvailabilityArchived,
    AvailabilityDetached,
    AvailabilityDeleted,
    AvailabilityUnavailable
};
Q_ENUM_NS(MomentAvailability)

enum CrystalExperienceState {
    CrystalDormant,
    CrystalPresent,
    CrystalFocused,
    CrystalImmersive,
    CrystalTransitioning,
    CrystalDetached,
    CrystalUnavailable
};
Q_ENUM_NS(CrystalExperienceState)

} // namespace Aurora
