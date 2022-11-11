#ifndef GALLERY_H
#define GALLERY_H

#include <QWidget>

#include <QStandardItemModel>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QDir>
class ThumbnailLoader;

namespace Ui {
class Gallery;
}

class Gallery : public QWidget
{
    Q_OBJECT

public:
    explicit Gallery(QWidget *parent = nullptr);
    ~Gallery();

    std::unique_ptr<QFutureWatcher<bool> >  updateGallery();
    void stopThumbnailLoading();
    void waitThumbnailLoading();
    void addImageFile(const QString& path);
    void setGalleryPath(const QString& path);

private slots:
    void on_toolButton_clicked();
    void on_pushButton_clicked();
    void onRequiresItemRefresh(int index,const QIcon& icon);

    void on_toolButton_2_clicked();

    void on_pushButton_2_clicked();
    void onItemDoubleClicked(const QModelIndex &index);

private:
    Ui::Gallery *ui;

    QStandardItemModel* m_model;
    std::unique_ptr<QFutureWatcher<bool> > m_watcher;
    ThumbnailLoader* m_loader;
    QList<std::pair<int, QFileInfo>> m_index_locations;
    int m_lastScroolVal;
    QDir m_galleryDir;
};

#endif // GALLERY_H
