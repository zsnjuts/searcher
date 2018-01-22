#ifndef CRAWLERTHREAD_H
#define CRAWLERTHREAD_H

#include <qthread>
#include <qdebug>
using namespace std;

class CrawlerThread : public QThread
{
    Q_OBJECT
private:
    //string trdUrl;
    //string trdBuf;
    QString entry;
    int MAXFILECOUNT;
    int crawler();
    void crawler_start();
    void run();
    void SaveData(const string &url, const string &buf); //显示进度

public:
    explicit CrawlerThread(QObject *parent = 0);

signals:
    void crawlQuestion(const QString &title, const QString &content, int &ret);
    void crawlWarning(const QString &title, const QString &content);
    void crawlProgress(int p);
    void crawlComplete(int pageNumber);
    void crawlBrowserAppend(const QString &str);
public slots:
    void slotCrawlParameter(const QString &str, int maxNumber);
    //void slotSaveData(const string &url, const string &buf);
};

#endif // CRAWLERTHREAD_H

