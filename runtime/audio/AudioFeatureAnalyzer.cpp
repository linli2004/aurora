#include "runtime/audio/AudioFeatureAnalyzer.h"

#include <QAudioFormat>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double pi = 3.14159265358979323846;
constexpr int smoothingIntervalMilliseconds = 33;
constexpr int staleBufferMilliseconds = 180;

bool materiallyDifferent(qreal first, qreal second)
{
    return std::abs(first - second) > 0.0025;
}
}

AudioFeatureAnalyzer::AudioFeatureAnalyzer(QObject *parent)
    : QObject(parent)
{
    m_smoothingTimer.setInterval(smoothingIntervalMilliseconds);
    m_smoothingTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_smoothingTimer, &QTimer::timeout,
            this, &AudioFeatureAnalyzer::advanceSmoothing);
    m_smoothingTimer.start();
}

qreal AudioFeatureAnalyzer::level() const
{
    return m_level;
}

qreal AudioFeatureAnalyzer::bassEnergy() const
{
    return m_bassEnergy;
}

qreal AudioFeatureAnalyzer::midEnergy() const
{
    return m_midEnergy;
}

qreal AudioFeatureAnalyzer::highEnergy() const
{
    return m_highEnergy;
}

qreal AudioFeatureAnalyzer::transientEnergy() const
{
    return m_transientEnergy;
}

bool AudioFeatureAnalyzer::available() const
{
    return m_available;
}

void AudioFeatureAnalyzer::processBuffer(const QAudioBuffer &buffer)
{
    if (!buffer.isValid() || buffer.frameCount() <= 0) {
        setTargets(0.0, 0.0, 0.0, 0.0, 0.0);
        return;
    }

    const QAudioFormat format = buffer.format();
    const int channelCount = format.channelCount();
    const int sampleRate = format.sampleRate();
    if (channelCount <= 0 || sampleRate <= 0)
        return;

    if (!m_available) {
        m_available = true;
        emit availabilityChanged();
    }

    if (!m_lastBuffer.isValid())
        m_lastBuffer.start();
    else
        m_lastBuffer.restart();

    switch (format.sampleFormat()) {
    case QAudioFormat::UInt8:
        analyzeInterleaved(buffer.constData<quint8>(),
                           buffer.frameCount(),
                           channelCount,
                           sampleRate,
                           1.0 / 128.0,
                           -128.0);
        break;
    case QAudioFormat::Int16:
        analyzeInterleaved(buffer.constData<qint16>(),
                           buffer.frameCount(),
                           channelCount,
                           sampleRate,
                           1.0 / 32768.0);
        break;
    case QAudioFormat::Int32:
        analyzeInterleaved(buffer.constData<qint32>(),
                           buffer.frameCount(),
                           channelCount,
                           sampleRate,
                           1.0 / 2147483648.0);
        break;
    case QAudioFormat::Float:
        analyzeInterleaved(buffer.constData<float>(),
                           buffer.frameCount(),
                           channelCount,
                           sampleRate,
                           1.0);
        break;
    default:
        setTargets(0.0, 0.0, 0.0, 0.0, 0.0);
        break;
    }
}

void AudioFeatureAnalyzer::setPlaybackActive(bool active)
{
    m_playbackActive = active;
    if (!active)
        setTargets(0.0, 0.0, 0.0, 0.0, 0.0);
}

void AudioFeatureAnalyzer::reset()
{
    m_level = 0.0;
    m_bassEnergy = 0.0;
    m_midEnergy = 0.0;
    m_highEnergy = 0.0;
    m_transientEnergy = 0.0;
    m_targetLevel = 0.0;
    m_targetBassEnergy = 0.0;
    m_targetMidEnergy = 0.0;
    m_targetHighEnergy = 0.0;
    m_targetTransientEnergy = 0.0;
    m_lowFilter = 0.0;
    m_midFilter = 0.0;
    m_previousSample = 0.0;
    m_lastBuffer.invalidate();
    emit featuresChanged();
}

void AudioFeatureAnalyzer::advanceSmoothing()
{
    if (!m_playbackActive
        || !m_lastBuffer.isValid()
        || m_lastBuffer.elapsed() > staleBufferMilliseconds) {
        setTargets(0.0, 0.0, 0.0, 0.0, 0.0);
    }

    const qreal nextLevel = smoothValue(m_level, m_targetLevel);
    const qreal nextBass = smoothValue(m_bassEnergy, m_targetBassEnergy);
    const qreal nextMid = smoothValue(m_midEnergy, m_targetMidEnergy);
    const qreal nextHigh = smoothValue(m_highEnergy, m_targetHighEnergy);
    const qreal nextTransient = smoothValue(m_transientEnergy, m_targetTransientEnergy);

    const bool changed = materiallyDifferent(nextLevel, m_level)
        || materiallyDifferent(nextBass, m_bassEnergy)
        || materiallyDifferent(nextMid, m_midEnergy)
        || materiallyDifferent(nextHigh, m_highEnergy)
        || materiallyDifferent(nextTransient, m_transientEnergy);

    m_level = nextLevel;
    m_bassEnergy = nextBass;
    m_midEnergy = nextMid;
    m_highEnergy = nextHigh;
    m_transientEnergy = nextTransient;

    if (changed)
        emit featuresChanged();
}

template<typename Sample>
void AudioFeatureAnalyzer::analyzeInterleaved(const Sample *samples,
                                              qsizetype frameCount,
                                              int channelCount,
                                              int sampleRate,
                                              qreal normalization,
                                              qreal offset)
{
    if (!samples || frameCount <= 0)
        return;

    const double lowAlpha = 1.0 - std::exp(-2.0 * pi * 180.0 / sampleRate);
    const double midAlpha = 1.0 - std::exp(-2.0 * pi * 2200.0 / sampleRate);

    double totalEnergy = 0.0;
    double lowEnergy = 0.0;
    double midEnergy = 0.0;
    double highEnergy = 0.0;
    double transientEnergy = 0.0;

    for (qsizetype frame = 0; frame < frameCount; ++frame) {
        double mono = 0.0;
        const qsizetype base = frame * channelCount;
        for (int channel = 0; channel < channelCount; ++channel) {
            const double raw = static_cast<double>(samples[base + channel]);
            mono += (raw + offset) * normalization;
        }
        mono /= channelCount;
        mono = std::clamp(mono, -1.0, 1.0);

        m_lowFilter += lowAlpha * (mono - m_lowFilter);
        m_midFilter += midAlpha * (mono - m_midFilter);

        const double low = m_lowFilter;
        const double mid = m_midFilter - m_lowFilter;
        const double high = mono - m_midFilter;
        const double delta = mono - m_previousSample;
        m_previousSample = mono;

        totalEnergy += mono * mono;
        lowEnergy += low * low;
        midEnergy += mid * mid;
        highEnergy += high * high;
        transientEnergy += delta * delta;
    }

    const double divisor = static_cast<double>(frameCount);
    const qreal rms = std::sqrt(totalEnergy / divisor);
    const qreal lowRms = std::sqrt(lowEnergy / divisor);
    const qreal midRms = std::sqrt(midEnergy / divisor);
    const qreal highRms = std::sqrt(highEnergy / divisor);
    const qreal transientRms = std::sqrt(transientEnergy / divisor);

    const qreal level = normalizedFeature(rms * 3.6);
    const qreal spectralSum = lowRms + midRms + highRms + 0.000001;
    const qreal lowRatio = lowRms / spectralSum;
    const qreal midRatio = midRms / spectralSum;
    const qreal highRatio = highRms / spectralSum;

    setTargets(
        level,
        normalizedFeature(level * (0.30 + lowRatio * 1.65)),
        normalizedFeature(level * (0.28 + midRatio * 1.70)),
        normalizedFeature(level * (0.24 + highRatio * 1.90)),
        normalizedFeature(transientRms * 4.2));
}

void AudioFeatureAnalyzer::setTargets(qreal level,
                                      qreal bass,
                                      qreal mid,
                                      qreal high,
                                      qreal transient)
{
    m_targetLevel = normalizedFeature(level);
    m_targetBassEnergy = normalizedFeature(bass);
    m_targetMidEnergy = normalizedFeature(mid);
    m_targetHighEnergy = normalizedFeature(high);
    m_targetTransientEnergy = normalizedFeature(transient);
}

qreal AudioFeatureAnalyzer::smoothValue(qreal current, qreal target)
{
    const qreal factor = target > current ? 0.38 : 0.11;
    const qreal next = current + (target - current) * factor;
    return std::abs(next) < 0.001 ? 0.0 : next;
}

qreal AudioFeatureAnalyzer::normalizedFeature(qreal value)
{
    if (!std::isfinite(value))
        return 0.0;
    return qBound<qreal>(0.0, value, 1.0);
}
