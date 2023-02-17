#include "ThumbnailCache.h"
#include <QDir>
#include <QCryptographicHash>
#include <QFileInfoList>
#include <QFile>
#include <QDateTime>

const unsigned int ThumbnailCache::MaxCachedFiles = 1000;

ThumbnailCache::ThumbnailCache(const QString& cacheFolder) : m_cacheFolder(cacheFolder + "/thumbnails"), m_nrCachedFiles(0)
{
    QDir cfolder(m_cacheFolder);
    if (cfolder.exists())
    {
        m_fileInfoLists = cfolder.entryInfoList(QDir::Files, QDir::Time);
        m_nrCachedFiles = m_fileInfoLists.size();
        for (auto fileInfo : m_fileInfoLists)
        {
            QString hash = fileInfo.completeBaseName();
            m_hashToFileInfo.insert({ hash, fileInfo });
        }
    }
    else
    {
        QDir().mkpath(m_cacheFolder);
    }

    // BUG: this is a workaround for a bug in QDateTime.toString() not being reentrant or thread-safe: https://bugreports.qt.io/browse/QTBUG-85692
    QDateTime workaroundBug = QDateTime::currentDateTime();
    workaroundBug.toString(Qt::ISODate);
}

ThumbnailCache::~ThumbnailCache()
{

}

bool ThumbnailCache::addThumbnailToCache(const QString& imageFilePath, QImage thumbnail)
{
    //todo 多线程读写 枷锁变成单线程;
    QMutexLocker locker(&m_mutex);
    QFileInfo img_info(imageFilePath);
    thumbnail.setText("imgFilePath", imageFilePath);
    QString fileHash = QCryptographicHash::hash((imageFilePath + img_info.lastModified().toString(Qt::ISODate)).toUtf8(), QCryptographicHash::Algorithm::Md5).toHex();
    while (m_nrCachedFiles >= MaxCachedFiles)
    {
        QFileInfo thumbnail = m_fileInfoLists.first();
        bool succes = QFile::remove(thumbnail.canonicalFilePath());
        if (succes)
        {
//            m_mutex.lock();
            m_fileInfoLists.pop_front();
            m_nrCachedFiles--;
//            m_mutex.unlock();
        }
        else
        {
            break;
        }
    }
    QString thumbnailFilePath = m_cacheFolder + "/" + fileHash + ".png";
    thumbnail.save(thumbnailFilePath);
    QFileInfo thumbInfo(thumbnailFilePath);
//    m_mutex.lock();
    m_fileInfoLists.push_back(thumbInfo);
    m_hashToFileInfo.insert({ fileHash, thumbInfo });
    m_nrCachedFiles++;
//    m_mutex.unlock();
    return true;
}



QImage ThumbnailCache::getThumbnailFromCache(const QString& imageFilePath)
{
    QFileInfo img_info(imageFilePath);
    QString file_hash = QCryptographicHash::hash((imageFilePath + img_info.lastModified().toString(Qt::ISODate)).toUtf8(), QCryptographicHash::Algorithm::Md5).toHex();
    QString thumbnailFilePath = m_cacheFolder + "/" + file_hash + ".png";
    if (m_hashToFileInfo.find(file_hash) != m_hashToFileInfo.end())
    {
        QImage thumbnail(thumbnailFilePath);
        return thumbnail;
    }
    else
    {
        return QImage();
    }
}
