#pragma once

#include <QObject>

class AuroraStateMapper final : public QObject
{
    Q_OBJECT

public:
    explicit AuroraStateMapper(QObject *parent = nullptr);

    Q_INVOKABLE int momentExperienceState(
        int lifecycle,
        double memoryConfidence,
        int availability) const;

    Q_INVOKABLE bool ambientMotionAllowed(
        int accessibilityMode,
        int motionMode,
        int qualityMode) const;
};
