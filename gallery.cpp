#include "gallery.h"
#include "ui_gallery.h"
#include <QDebug>
#include <QFileDialog>
#include <QtConcurrent>
#include "ThumbnailLoader.h"
#include <QEvent>
#include <QKeyEvent>
#include <QScrollBar>

Gallery::Gallery(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Gallery),
    m_lastScroolVal(0)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(0, 0);
    m_loader = new ThumbnailLoader();
    m_watcher.reset(new QFutureWatcher<bool>);
    ui->view_images->setModel(m_model);
    ui->horizontalSlider->setValue(200);
    ui->spinBox->setValue(200);
    connect(ui->spinBox, SIGNAL(valueChanged(int)), ui->horizontalSlider, SLOT(setValue(int)));
    connect(ui->horizontalSlider, &QSlider::valueChanged, [&](int val)
    {
        ui->spinBox->setValue(val);
        qDebug() << "wid:" << ui->view_images->width();
        int gridSize = (ui->view_images->width() - 30) / val;
        qDebug() << "grid" << gridSize << " val" << val << " wid" << ui->view_images->width();
//        int gridSize = (ui->view_images->width() ) / val;
//        qDebug()<<"width"<<ui->view_images->width()<<"grid:"<<gridSize;
//        ui->view_images->setGridSize(QSize(gridSize - 10 , gridSize - 10));
//        ui->view_images->setIconSize(QSize(gridSize - 30, gridSize - 30));
        ui->view_images->setGridSize(QSize(gridSize, gridSize ));
        ui->view_images->setIconSize(QSize(gridSize - 20, gridSize - 20 ));

//        qDebug() << "val" << val;
//        qDebug()<<" width: "<<ui->view_images->width()<<" gridSize "<< gridSize;

    });
    connect(ui->view_images, &QListView::doubleClicked, this, &Gallery::onItemDoubleClicked);
}

Gallery::~Gallery()
{
    delete ui;
}


void Gallery::stopThumbnailLoading(void)
{
    if (m_watcher)
    {
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
    QApplication::processEvents();
}

void Gallery::waitThumbnailLoading()
{
    if (m_watcher)
    {

        m_watcher->waitForFinished();
    }
    QApplication::processEvents();
}

void Gallery::addImageFile(const QString& path)
{
//    waitThumbnailLoading();
    QImage img(path);
    QIcon ico = m_loader->createStandardIcon(img);
    QFileInfo info(path);

    QStandardItem* standard_item(new QStandardItem(ico, info.fileName()));
    standard_item->setData(QVariant(QString::number(m_model->rowCount())));
    standard_item->setData(QVariant(info.filePath()), Qt::UserRole + 2);
    m_model->insertRow(0, standard_item);
    ui->view_images->update();
}

void Gallery::on_toolButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Icon Dir"));
    if (!path.isEmpty())
    {
        ui->lineEdit->setText(path);
        setGalleryPath(path);
    }
}

void Gallery::on_pushButton_clicked()
{
    QString icoDir = ui->lineEdit->text();
    if (!icoDir.isEmpty())
    {
        setGalleryPath(icoDir);
    }
}

void Gallery::onRequiresItemRefresh(int index, const QIcon& icon)
{
    if (index >= 0)
    {
        m_model->item(index)->setIcon(icon);
    }
    else
    {
        m_model->layoutChanged();
    }
}


std::unique_ptr<QFutureWatcher<bool> > Gallery::updateGallery()
{
    m_model->removeRows(0, m_model->rowCount());
    // Creates placeholder items
    m_model->setRowCount(m_index_locations.size());
    QImage image(200, 200, QImage::Format::Format_BGR30);
    image.fill(Qt::white);
    QIcon placeholder = QIcon(QPixmap::fromImage(image));
    for (size_t item = 0; item < m_index_locations.size(); ++item)
    {
        QStandardItem* standard_item(new QStandardItem(placeholder, m_index_locations[item].second.fileName()));
        standard_item->setData(QVariant(QString::number(m_index_locations[item].first)));
        standard_item->setData(QVariant(m_index_locations[item].second.filePath()), Qt::UserRole + 2);
        m_model->setItem(item, 0, standard_item);
    }
    auto icon_connection = QObject::connect(m_loader,
                                            &ThumbnailLoader::requiresItemRefresh,
                                            this,
                                            &Gallery::onRequiresItemRefresh);
    std::function<bool(const std::pair<int, QFileInfo> & index_location)> create_icons = [creator = m_loader](const std::pair<int, QFileInfo>& index_location) -> bool
    {
        bool valid = creator->insertThumbnail(index_location);
        return valid;
    };
    QFuture<bool> future = QtConcurrent::mapped(m_index_locations, create_icons);
    std::unique_ptr<QFutureWatcher<bool> > future_watcher(new QFutureWatcher<bool>);
//    QObject::connect(&(*future_watcher), &QFutureWatcher<void>::progressValueChanged, [ = ](int pv)
//    {
//        this->updateStatusBar("Loading thumbnail " + QString::fromStdString(std::to_string(pv)) + " of " + QString::fromStdString(std::to_string(items.size())));
//    });
    QObject::connect(&(*future_watcher), &QFutureWatcher<void>::finished, [ = ]()
    {
//        window->updateStatusBar("Finished loading thumbnails.");
        QSignalBlocker blocker0(ui->horizontalSlider);
        QSignalBlocker blocker(ui->spinBox);
        ui->spinBox->setValue(ui->view_images->width() / 200);
        ui->horizontalSlider->setValue(ui->view_images->width() / 200);
        QObject::disconnect(icon_connection);
    });
    future_watcher->setFuture(future);
    return future_watcher;
}


void Gallery::on_toolButton_2_clicked()
{
    QString image_path = QFileDialog::getOpenFileName(nullptr, tr("Open Image File"), QString(), tr("Images (*.png *.bmp *.jpg *.tiff *.tif *.ico)"));
    if (!image_path.isEmpty())
    {
        ui->lineEdit_2->setText(image_path);
    }
}

void Gallery::on_pushButton_2_clicked()
{
    if (!ui->lineEdit_2->text().isEmpty())addImageFile(ui->lineEdit_2->text());
}


void Gallery::setGalleryPath(const QString& path)
{
    QDir dir(path);
    if (dir.exists())
    {
        m_galleryDir = dir;
        QStringList filters;
        filters.append("*.bmp");
        filters.append("*.jpeg");
        filters.append("*.jpg");
        filters.append("*.png");
        filters.append("*.tiff");
        filters.append("*.tif");
        filters.append("*.mvd");
        filters.append("*.ico");

        QStringList deleteFilters;
        QFileInfoList delInfoList = m_galleryDir.entryInfoList(filters, QDir::NoFilter, QDir::Time);

        m_index_locations.clear();
        for (int idx = 0; idx < delInfoList.size(); idx++)
        {
            QFileInfo info = delInfoList[idx];
            m_index_locations.append(std::pair<int, QFileInfo>(idx, info));
        }
        stopThumbnailLoading();
        m_watcher = updateGallery();

        qDebug() << QStandardPaths::writableLocation(QStandardPaths::StandardLocation::CacheLocation);
    }
}

void Gallery::onItemDoubleClicked(const QModelIndex& index)
{
    QStandardItem* item = m_model->itemFromIndex(index);
    qDebug() << "onItemDoubleClicked: " << item->data(Qt::UserRole + 2).toString();
}
