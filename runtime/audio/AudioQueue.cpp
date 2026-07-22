#include "runtime/audio/AudioQueue.h"

#include <QSet>

void AudioQueue::setUrls(const QList<QUrl> &urls)
{
    m_urls = urls;
    m_currentIndex = m_urls.isEmpty() ? -1 : 0;
}

void AudioQueue::appendUrls(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    const bool wasEmpty = m_urls.isEmpty();
    m_urls.append(urls);
    if (wasEmpty)
        m_currentIndex = 0;
}

void AudioQueue::clear()
{
    m_urls.clear();
    m_currentIndex = -1;
}

bool AudioQueue::isEmpty() const
{
    return m_urls.isEmpty();
}

int AudioQueue::count() const
{
    return m_urls.size();
}

int AudioQueue::currentIndex() const
{
    return m_currentIndex;
}

QUrl AudioQueue::currentUrl() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_urls.size())
        return {};
    return m_urls.at(m_currentIndex);
}

const QList<QUrl> &AudioQueue::urls() const
{
    return m_urls;
}

bool AudioQueue::moveTo(int index)
{
    if (index < 0 || index >= m_urls.size() || index == m_currentIndex)
        return false;

    m_currentIndex = index;
    return true;
}

bool AudioQueue::moveNext()
{
    if (m_urls.size() < 2)
        return false;

    if (m_currentIndex < 0 || m_currentIndex >= m_urls.size()) {
        m_currentIndex = 0;
        return true;
    }

    m_currentIndex = (m_currentIndex + 1) % m_urls.size();
    return true;
}

bool AudioQueue::movePrevious()
{
    if (m_urls.size() < 2)
        return false;

    if (m_currentIndex < 0 || m_currentIndex >= m_urls.size()) {
        m_currentIndex = m_urls.size() - 1;
        return true;
    }

    m_currentIndex = (m_currentIndex - 1 + m_urls.size()) % m_urls.size();
    return true;
}

QList<QUrl> AudioQueue::fromVariantList(const QVariantList &values)
{
    QList<QUrl> result;
    QSet<QString> seen;

    for (const QVariant &value : values) {
        const QUrl url = value.toUrl();
        if (!url.isValid() || url.isEmpty())
            continue;

        const QString key = url.adjusted(QUrl::NormalizePathSegments).toString();
        if (seen.contains(key))
            continue;

        seen.insert(key);
        result.append(url);
    }

    return result;
}
