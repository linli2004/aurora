#include <QtTest>

#include "runtime/lyrics/LyricsService.h"

class LyricsServiceTest final : public QObject
{
    Q_OBJECT

private slots:
    void parsesAndAdvancesTimedLyrics();
    void supportsMultipleTimestampsAndOffset();
    void clearsInvalidLyrics();
};

void LyricsServiceTest::parsesAndAdvancesTimedLyrics()
{
    LyricsService lyrics;
    QVERIFY(lyrics.loadFromText(QStringLiteral(
        "[00:01.00]First line\n"
        "[00:03.50]Second line\n"
        "[00:06.00]Third line\n")));

    QVERIFY(lyrics.hasLyrics());
    lyrics.setPosition(900);
    QCOMPARE(lyrics.currentLine(), QString());
    QCOMPARE(lyrics.nextLine(), QStringLiteral("First line"));

    lyrics.setPosition(1200);
    QCOMPARE(lyrics.currentLine(), QStringLiteral("First line"));
    QCOMPARE(lyrics.nextLine(), QStringLiteral("Second line"));

    lyrics.setPosition(3600);
    QCOMPARE(lyrics.currentLine(), QStringLiteral("Second line"));
    QCOMPARE(lyrics.nextLine(), QStringLiteral("Third line"));
}

void LyricsServiceTest::supportsMultipleTimestampsAndOffset()
{
    LyricsService lyrics;
    QVERIFY(lyrics.loadFromText(QStringLiteral(
        "[offset:250]\n"
        "[00:01.00][00:02.00]Echo\n")));

    lyrics.setPosition(1100);
    QCOMPARE(lyrics.currentLine(), QString());
    lyrics.setPosition(1300);
    QCOMPARE(lyrics.currentLine(), QStringLiteral("Echo"));
    lyrics.setPosition(2300);
    QCOMPARE(lyrics.currentLine(), QStringLiteral("Echo"));
}

void LyricsServiceTest::clearsInvalidLyrics()
{
    LyricsService lyrics;
    QVERIFY(!lyrics.loadFromText(QStringLiteral("No timestamps here")));
    QVERIFY(!lyrics.hasLyrics());
    QCOMPARE(lyrics.currentLine(), QString());
    QCOMPARE(lyrics.nextLine(), QString());
}

QTEST_GUILESS_MAIN(LyricsServiceTest)
#include "test_lyrics_service.moc"
