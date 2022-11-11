#include "ThumbnailLoader.h"
#include <QIcon>
#include <QPainter>
#include "ThumbnailCache.h"
#include <QStandardPaths>
#include <QDebug>
const int ICON_SIZE = 200;

ThumbnailLoader::ThumbnailLoader()
{
    m_fillColor = Qt::gray;
    QImage invalidImg(QSize(ICON_SIZE, ICON_SIZE), QImage::Format_BGR888);
    invalidImg.fill(Qt::gray);
    m_invalidIcon = QIcon(QPixmap::fromImage(invalidImg));
    m_thumbnail_cache = new ThumbnailCache(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::CacheLocation));
}

ThumbnailLoader::~ThumbnailLoader()
{
    if (m_thumbnail_cache)
    {
        delete m_thumbnail_cache;
    }
}

QIcon ThumbnailLoader::createStandardIcon(const QImage& img, const QString& path)
{
    if (img.isNull())
    {
        return m_invalidIcon;
    }

    QImage image = img.scaled(ICON_SIZE, ICON_SIZE, Qt::AspectRatioMode::KeepAspectRatio);
    float bili = float(image.width()) / float(image.height());
    //将图像绘制到统一尺寸的缩略图中
    QImage paint_image(QSize(ICON_SIZE, ICON_SIZE), image.format());
    paint_image.fill(m_fillColor);

    QPoint st(0, 0);
    if (bili > 1.0f)
    {
        st.setY((ICON_SIZE - image.height()) / 2);
    }
    else
    {
        st.setX((ICON_SIZE - image.width()) / 2);
    }

    QPainter p(&paint_image);
    p.drawImage(st, image);
    p.drawRect(0, 0, ICON_SIZE - 1, ICON_SIZE - 1);
    if (!path.isEmpty())
    {
        m_thumbnail_cache->addThumbnailToCache(path, paint_image);
    }

    QPixmap pixmap(QPixmap::fromImage(paint_image));
    return  QIcon(pixmap);
}

bool ThumbnailLoader::insertThumbnail(std::pair<int, QFileInfo> index_location)
{
    QString filepath = index_location.second.filePath();
    QImage img = m_thumbnail_cache->getThumbnailFromCache(filepath);
    QIcon ico;
    if (!img.isNull())
    {
        QPixmap pixmap(QPixmap::fromImage(img));
        ico =  QIcon(pixmap);
    }
    else
    {
        img = QImage(filepath);
        ico = createStandardIcon(img,filepath);
    }
    emit requiresItemRefresh(index_location.first, ico);
    return true;
}
