#include "initridthread.h"
#include "rid.h"

InitRidThread::InitRidThread(QObject *parent) :
    QThread(parent)
{
}


void InitRidThread::run()
{
    INDEX_DIR_PATH = string((const char *)QObject::tr("%1/text-index/").arg(entry).toLocal8Bit());
    filenameRID = string((const char *)QObject::tr("%1/RID.dat").arg(entry).toLocal8Bit());
    compulsory_write_RID = 1;//强制覆盖
    //qDebug() << INDEX_DIR_PATH;
    //qDebug() << filenameRID;

    RID_initiation(webNum);

    qDebug() << "initRid Complete";
    emit initRidComplete(QString::fromStdString(filenameRID), init_stat);
    //emit initRidComplete(filenameRID,init_stat);
}

void InitRidThread::slotInitRidParameter(const QString &str, int pageNumber)
{
    entry = str;
    webNum = pageNumber;
}
