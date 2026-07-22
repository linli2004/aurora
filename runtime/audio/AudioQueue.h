#pragma once

#include <QList>
#include <QUrl>
#include <QVariantList>

class AudioQueue
{
public:
    void setUrls(const QList<QUrl> &urls);
    void appendUrls(const QList<QUrl> &urls);
    void clear();

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] int count() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] QUrl currentUrl() const;
    [[nodiscard]] const QList<QUrl> &urls() const;

    bool moveTo(int index);
    bool moveNext();
    bool movePrevious();

    static QList<QUrl> fromVariantList(const QVariantList &values);

private:
    QList<QUrl> m_urls;
    int m_currentIndex = -1;
};
