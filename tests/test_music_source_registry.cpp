#include <QtTest/QtTest>

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QUuid>

#include "runtime/sources/MusicSourceRegistry.h"

class MusicSourceRegistryTest final : public QObject
{
    Q_OBJECT

private slots:
    void splitsSpaceSeparatedScriptUrls();
    void parsesOneLineLxScriptMetadata();
    void importsLocalScriptFileOnce();
};

void MusicSourceRegistryTest::splitsSpaceSeparatedScriptUrls()
{
    const QString text = QStringLiteral(
        "https://raw.githubusercontent.com/pdone/lx-music-source/main/flower/latest.js "
        "https://raw.githubusercontent.com/pdone/lx-music-source/main/huibq/latest.js "
        "https://ghproxy.net/raw.githubusercontent.com/pdone/lx-music-source/main/ikun/latest.js");

    const QStringList entries = MusicSourceRegistry::entriesFromText(text);

    QCOMPARE(entries.size(), 3);
    QVERIFY(entries.at(0).endsWith(QStringLiteral("/flower/latest.js")));
    QVERIFY(entries.at(1).endsWith(QStringLiteral("/huibq/latest.js")));
    QVERIFY(entries.at(2).contains(QStringLiteral("/ikun/latest.js")));
}

void MusicSourceRegistryTest::parsesOneLineLxScriptMetadata()
{
    const QString script = QStringLiteral(
        "/** * @name 野花 * @version 1 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "send(EVENT_NAMES.inited, { sources: {} });");
    const QUrl scriptUrl(
        QStringLiteral("https://raw.githubusercontent.com/pdone/lx-music-source/main/flower/latest.js"));

    const std::optional<MusicSourceRecord> record =
        MusicSourceRegistry::parseScriptMetadata(script, scriptUrl);

    QVERIFY(record.has_value());
    QCOMPARE(record->name, QStringLiteral("野花"));
    QCOMPARE(record->version, QStringLiteral("1"));
    QVERIFY(record->sourceId.startsWith(QStringLiteral("source:lx-script:v1:")));
    QCOMPARE(record->scriptUrl, scriptUrl);
}

void MusicSourceRegistryTest::importsLocalScriptFileOnce()
{
    QCoreApplication::setOrganizationName(QStringLiteral("AuroraTests"));
    QCoreApplication::setApplicationName(
        QStringLiteral("MusicSourceRegistry-%1").arg(QUuid::createUuid().toString(QUuid::Id128)));
    QSettings().clear();

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString filePath = directory.filePath(QStringLiteral("latest.js"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(
        "/** * @name 回聲 * @version 2 */ "
        "const { EVENT_NAMES, on, send } = globalThis.lx; "
        "send(EVENT_NAMES.inited, { sources: {} });");
    file.close();

    MusicSourceRegistry registry;
    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);
    QCOMPARE(registry.sourceNames(), QStringList { QStringLiteral("回聲") });

    registry.importFromText(filePath);
    QCOMPARE(registry.sourceCount(), 1);
}

QTEST_MAIN(MusicSourceRegistryTest)
#include "test_music_source_registry.moc"
