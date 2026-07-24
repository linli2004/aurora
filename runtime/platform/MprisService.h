#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class AudioRuntime;

class MprisService final : public QObject
{
    Q_OBJECT

public:
    explicit MprisService(AudioRuntime *audioRuntime, QObject *parent = nullptr);

    [[nodiscard]] bool registered() const;
    [[nodiscard]] QString errorString() const;

private:
    void registerService();
    void emitPropertiesChanged(const QString &interfaceName,
                               const QVariantMap &changedProperties);

    AudioRuntime *m_audioRuntime = nullptr;
    bool m_registered = false;
    QString m_errorString;
};
