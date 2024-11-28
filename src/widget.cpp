#include "widget.h"
#include "qicohandler.h"
#include "ui_widget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    setAcceptDrops(true);

    m_validImageTypes << "png"
                      << "jpg"
                      << "jpeg"
                      << "bmp";

    m_valildSizes = {QSize(12, 12),
                     QSize(16, 16),
                     QSize(32, 32),
                     QSize(48, 48),
                     QSize(64, 64),
                     QSize(72, 72),
                     QSize(128, 128),
                     QSize(256, 256),
                     QSize(512, 512),
                     QSize(1024, 1024)};
}

Widget::~Widget()
{
    delete ui;
}

void Widget::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}

void Widget::addItemToList()
{
    foreach (auto imgName, m_imgageNames) {
        QListWidgetItem *imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon(imgName));

        imageItem->setSizeHint(QSize(120, 120));
        ui->listWidgetImages->addItem(imageItem);
    }
}

void Widget::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    foreach (auto url, urls) {
        QString imgName = url.toLocalFile();

        QFileInfo fi(imgName);
        QString suffix = fi.suffix();

        if (!m_validImageTypes.contains(suffix)) {
            continue;
        }

        if (!m_imgageNames.contains(imgName)) {
            m_imgageNames.push_back(imgName);

            QListWidgetItem *imageItem = new QListWidgetItem;
            imageItem->setIcon(QIcon(imgName));

            imageItem->setSizeHint(QSize(120, 120));
            ui->listWidgetImages->addItem(imageItem);
        }
    }
}

void Widget::on_toolButtonConvert_clicked()
{
    foreach (auto name, m_imgageNames) {
        QImage img(name);

        QString icoName = name;
        //将所有的文件后缀改为.ico
        QRegExp regex("\\.[a-zA-Z0-9]+$", Qt::CaseInsensitive);
        icoName.replace(regex, ".ico");

        // 打开文件用于写入
        QFile iconFile(icoName);

        if (iconFile.exists()) {
            iconFile.remove();
        }

        if (iconFile.open(QFile::WriteOnly)) {
            // 定义要包含在ICO文件中的图像大小列表

            int maxSize = img.size().width();
            QVector<QImage> imgs;
            // 为每个大小添加图像到QIcon
            for (const QSize &size : m_valildSizes) {
                if (size.width() > maxSize) {
                    break;
                }

                QImage scaledImage = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imgs.push_back(scaledImage);
            }

            ICOWriter::write(&iconFile, imgs);
            iconFile.close();
        } else {
            qDebug() << iconFile.errorString() << icoName;
        }
    }
}

void Widget::on_toolButtonClearAllImg_clicked()
{
    ui->listWidgetImages->clear();
    m_imgageNames.clear();
}

void Widget::on_toolButtonAddImgs_clicked()
{
    QString filter = "Images (";
    foreach (auto type, m_validImageTypes) {
        filter += QString("*.%1 ").arg(type);
    }
    filter += ")";

    QStringList imgFiles = QFileDialog::getOpenFileNames(this, "选择文件", "", filter);

    foreach (auto imgName, imgFiles) {
        if (!m_imgageNames.contains(imgName)) {
            m_imgageNames.push_back(imgName);
        }
    }
	
    addItemToList();
}
