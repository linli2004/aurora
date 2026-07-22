#include <QtTest>

#include <QAudioBuffer>
#include <QAudioFormat>
#include <QByteArray>
#include <QVector>

#include <cmath>

#include "runtime/audio/AudioFeatureAnalyzer.h"

namespace {
QAudioBuffer sineBuffer(qreal frequency, qreal amplitude = 0.55, int sampleRate = 48000)
{
    constexpr int frameCount = 4096;
    QVector<float> samples(frameCount);
    for (int frame = 0; frame < frameCount; ++frame) {
        const qreal phase = 2.0 * 3.14159265358979323846 * frequency * frame / sampleRate;
        samples[frame] = static_cast<float>(std::sin(phase) * amplitude);
    }

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);

    const QByteArray bytes(reinterpret_cast<const char *>(samples.constData()),
                           samples.size() * static_cast<qsizetype>(sizeof(float)));
    return QAudioBuffer(bytes, format);
}
}

class AudioFeatureAnalyzerTest final : public QObject
{
    Q_OBJECT

private slots:
    void lowFrequencyProducesBassPresence();
    void highFrequencyProducesHighPresence();
    void inactivePlaybackDecaysToQuiet();
};

void AudioFeatureAnalyzerTest::lowFrequencyProducesBassPresence()
{
    AudioFeatureAnalyzer analyzer;
    analyzer.setPlaybackActive(true);
    analyzer.processBuffer(sineBuffer(110.0));

    QTRY_VERIFY_WITH_TIMEOUT(analyzer.available(), 300);
    QTRY_VERIFY_WITH_TIMEOUT(analyzer.level() > 0.15, 500);
    QVERIFY(analyzer.bassEnergy() > analyzer.highEnergy());
}

void AudioFeatureAnalyzerTest::highFrequencyProducesHighPresence()
{
    AudioFeatureAnalyzer analyzer;
    analyzer.setPlaybackActive(true);
    analyzer.processBuffer(sineBuffer(5200.0));

    QTRY_VERIFY_WITH_TIMEOUT(analyzer.level() > 0.15, 500);
    QVERIFY(analyzer.highEnergy() > analyzer.bassEnergy());
}

void AudioFeatureAnalyzerTest::inactivePlaybackDecaysToQuiet()
{
    AudioFeatureAnalyzer analyzer;
    analyzer.setPlaybackActive(true);
    analyzer.processBuffer(sineBuffer(440.0));
    QTRY_VERIFY_WITH_TIMEOUT(analyzer.level() > 0.15, 500);

    analyzer.setPlaybackActive(false);
    QTRY_VERIFY_WITH_TIMEOUT(analyzer.level() < 0.04, 1200);
    QVERIFY(analyzer.bassEnergy() < 0.05);
    QVERIFY(analyzer.midEnergy() < 0.05);
    QVERIFY(analyzer.highEnergy() < 0.05);
}

// AudioFeatureAnalyzer uses a QTimer to smooth feature values.  The app-less
// Qt Test entry point does not create a QCoreApplication/event dispatcher, so
// the timer never advances and every asynchronous assertion times out.  Use
// the guiless entry point: it provides an event loop without requiring a GUI.
QTEST_GUILESS_MAIN(AudioFeatureAnalyzerTest)
#include "test_audio_feature_analyzer.moc"
