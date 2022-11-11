#ifndef THUMBNAILLOADER_H
#define THUMBNAILLOADER_H
#include <QObject>
#include <QIcon>
#include <QFileInfo>

class ThumbnailCache;

class ThumbnailLoader : public QObject
{
    Q_OBJECT
public:
    ThumbnailLoader();
    ~ThumbnailLoader();

    bool insertThumbnail(std::pair<int, QFileInfo> index_location);
    void setFillColor(QColor cr)
    {
        m_fillColor = cr;
    }
    QIcon createStandardIcon(const QImage &img,const QString& path = QString());
signals:
    void requiresItemRefresh(int index, const QIcon& icon);

private:
    QColor m_fillColor;
    QIcon m_invalidIcon;
    ThumbnailCache* m_thumbnail_cache;
};

#endif // THUMBNAILLOADER_H
