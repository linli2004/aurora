#include <QtTest>

#include "runtime/AuroraStateMapper.h"
#include "runtime/AuroraTypes.h"

class StateMapperTest final : public QObject
{
    Q_OBJECT

private slots:
    void candidateBelowThresholdIsDormant();
    void candidateAtThresholdIsPresent();
    void confirmedMeaningIsMeaningful();
    void detachedOverridesLifecycle();
    void reducedMotionDisablesAmbientMotion();
};

void StateMapperTest::candidateBelowThresholdIsDormant()
{
    AuroraStateMapper mapper;
    QCOMPARE(
        mapper.momentExperienceState(
            Aurora::MomentCandidate,
            0.49,
            Aurora::AvailabilityActive),
        static_cast<int>(Aurora::MomentDormant));
}

void StateMapperTest::candidateAtThresholdIsPresent()
{
    AuroraStateMapper mapper;
    QCOMPARE(
        mapper.momentExperienceState(
            Aurora::MomentCandidate,
            0.50,
            Aurora::AvailabilityActive),
        static_cast<int>(Aurora::MomentPresent));
}

void StateMapperTest::confirmedMeaningIsMeaningful()
{
    AuroraStateMapper mapper;
    QCOMPARE(
        mapper.momentExperienceState(
            Aurora::MomentMeaningfulLifecycle,
            0.90,
            Aurora::AvailabilityActive),
        static_cast<int>(Aurora::MomentMeaningful));
}

void StateMapperTest::detachedOverridesLifecycle()
{
    AuroraStateMapper mapper;
    QCOMPARE(
        mapper.momentExperienceState(
            Aurora::MomentMeaningfulLifecycle,
            1.0,
            Aurora::AvailabilityDetached),
        static_cast<int>(Aurora::MomentDetached));
}

void StateMapperTest::reducedMotionDisablesAmbientMotion()
{
    AuroraStateMapper mapper;
    QVERIFY(!mapper.ambientMotionAllowed(
        Aurora::ReducedMotion,
        Aurora::MotionNormal,
        Aurora::Balanced));
    QVERIFY(!mapper.ambientMotionAllowed(
        Aurora::AccessibilityNormal,
        Aurora::MotionNormal,
        Aurora::Eco));
    QVERIFY(mapper.ambientMotionAllowed(
        Aurora::AccessibilityNormal,
        Aurora::MotionNormal,
        Aurora::Balanced));
}

QTEST_APPLESS_MAIN(StateMapperTest)

#include "test_state_mapper.moc"
