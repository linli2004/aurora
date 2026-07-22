#pragma once

#include <QObject>
#include <QAudioBuffer>
#include <QElapsedTimer>
#include <QTimer>

class AudioFeatureAnalyzer final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal level READ level NOTIFY featuresChanged)
    Q_PROPERTY(qreal bassEnergy READ bassEnergy NOTIFY featuresChanged)
    Q_PROPERTY(qreal midEnergy READ midEnergy NOTIFY featuresChanged)
    Q_PROPERTY(qreal highEnergy READ highEnergy NOTIFY featuresChanged)
    Q_PROPERTY(qreal transientEnergy READ transientEnergy NOTIFY featuresChanged)
    Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)

public:
    explicit AudioFeatureAnalyzer(QObject *parent = nullptr);

    [[nodiscard]] qreal level() const;
    [[nodiscard]] qreal bassEnergy() const;
    [[nodiscard]] qreal midEnergy() const;
    [[nodiscard]] qreal highEnergy() const;
    [[nodiscard]] qreal transientEnergy() const;
    [[nodiscard]] bool available() const;

public slots:
    void processBuffer(const QAudioBuffer &buffer);
    void setPlaybackActive(bool active);
    void reset();

signals:
    void featuresChanged();
    void availabilityChanged();

private slots:
    void advanceSmoothing();

private:
    template<typename Sample>
    void analyzeInterleaved(const Sample *samples,
                            qsizetype frameCount,
                            int channelCount,
                            int sampleRate,
                            qreal normalization,
                            qreal offset = 0.0);

    void setTargets(qreal level,
                    qreal bass,
                    qreal mid,
                    qreal high,
                    qreal transient);
    static qreal smoothValue(qreal current, qreal target);
    static qreal normalizedFeature(qreal value);

    QTimer m_smoothingTimer;
    QElapsedTimer m_lastBuffer;

    qreal m_level = 0.0;
    qreal m_bassEnergy = 0.0;
    qreal m_midEnergy = 0.0;
    qreal m_highEnergy = 0.0;
    qreal m_transientEnergy = 0.0;

    qreal m_targetLevel = 0.0;
    qreal m_targetBassEnergy = 0.0;
    qreal m_targetMidEnergy = 0.0;
    qreal m_targetHighEnergy = 0.0;
    qreal m_targetTransientEnergy = 0.0;

    double m_lowFilter = 0.0;
    double m_midFilter = 0.0;
    double m_previousSample = 0.0;

    bool m_playbackActive = false;
    bool m_available = false;
};
