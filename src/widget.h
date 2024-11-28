#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private slots:
    void on_toolButtonConvert_clicked();
    void on_toolButtonClearAllImg_clicked();

    void on_toolButtonAddImgs_clicked();

private:
    Ui::Widget *ui;

    QStringList m_imgageNames;

    QStringList m_validImageTypes;
    QList<QSize> m_valildSizes;

private:
    void addItemToList();
};
#endif // WIDGET_H
