#include <QtTest>

#include "runtime/audio/AudioQueue.h"

class AudioQueueTest final : public QObject
{
    Q_OBJECT

private slots:
    void startsAtFirstTrack();
    void loopsAcrossQueueBoundaries();
    void singleTrackDoesNotReportMovement();
    void removesDuplicateVariantUrls();
};

void AudioQueueTest::startsAtFirstTrack()
{
    AudioQueue queue;
    queue.setUrls({QUrl("file:///music/a.mp3"), QUrl("file:///music/b.mp3")});

    QCOMPARE(queue.count(), 2);
    QCOMPARE(queue.currentIndex(), 0);
    QCOMPARE(queue.currentUrl(), QUrl("file:///music/a.mp3"));
}

void AudioQueueTest::loopsAcrossQueueBoundaries()
{
    AudioQueue queue;
    queue.setUrls({
        QUrl("file:///music/a.mp3"),
        QUrl("file:///music/b.mp3"),
        QUrl("file:///music/c.mp3")
    });

    QVERIFY(queue.moveNext());
    QCOMPARE(queue.currentIndex(), 1);
    QVERIFY(queue.moveNext());
    QCOMPARE(queue.currentIndex(), 2);

    // Last -> first.
    QVERIFY(queue.moveNext());
    QCOMPARE(queue.currentIndex(), 0);
    QCOMPARE(queue.currentUrl(), QUrl("file:///music/a.mp3"));

    // First -> last.
    QVERIFY(queue.movePrevious());
    QCOMPARE(queue.currentIndex(), 2);
    QCOMPARE(queue.currentUrl(), QUrl("file:///music/c.mp3"));
}

void AudioQueueTest::singleTrackDoesNotReportMovement()
{
    AudioQueue queue;
    queue.setUrls({QUrl("file:///music/a.mp3")});

    QVERIFY(!queue.moveNext());
    QVERIFY(!queue.movePrevious());
    QCOMPARE(queue.currentIndex(), 0);
}

void AudioQueueTest::removesDuplicateVariantUrls()
{
    const QVariantList values {
        QUrl("file:///music/a.mp3"),
        QUrl("file:///music/a.mp3"),
        QUrl("file:///music/b.mp3"),
        QVariant()
    };

    const QList<QUrl> urls = AudioQueue::fromVariantList(values);
    QCOMPARE(urls.size(), 2);
}

QTEST_MAIN(AudioQueueTest)
#include "test_audio_queue.moc"
