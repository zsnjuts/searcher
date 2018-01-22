#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>
#include <direct.h>
#include <windows.h>
#include "rid.h"
#include "crawler.h"

void Widget::slotInitRidComplete(const QString &filename, int state)
{
    working = false;
    qDebug() << "slot got";
    ui->labelStatus->setText("倒排索引建立完成");
    QMessageBox::information(this, tr("倒排索引"),
          tr("倒排索引建立完成，保存在%1文件中，状态为%2").arg(filename).arg(QString::fromStdString(RID_NOTICE[state])));
}
