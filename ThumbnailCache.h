#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QString>
#include <QImage>
#include <QFileInfo>
#include <QMutex>
#include <map>

class ThumbnailCache {
public:
    ThumbnailCache(const QString& cacheFolder);
    ~ThumbnailCache();

    bool addThumbnailToCache(const QString& imageFilePath, QImage thumbnail);
    QImage getThumbnailFromCache(const QString& imageFilePath);

private:
    QMutex m_mutex;
    QString m_cacheFolder;
    QList<QString> m_cacheHashes;
    std::map<QString, QFileInfo> m_hashToFileInfo;
    QFileInfoList m_fileInfoLists;
    unsigned int m_nrCachedFiles;
    static const unsigned int MaxCachedFiles;
};

#endif // THUMBNAILCACHE_H
