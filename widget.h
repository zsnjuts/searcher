#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextCharFormat>
#include "cppjieba-master/include/cppjieba/Jieba.hpp"
#include <string>
#include <vector>
#include "crawlerthread.h"
#include "initridthread.h"
using namespace std;
using namespace cppjieba;
using namespace limonp;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void sendCrawlParameter(const QString &str, int maxNumber);

    void sendInitRidParameter(const QString &str, int pageNumber);

private slots:
    //void on_pushButtonShowData_clicked();

    //void on_textBrowser_anchorClicked(const QUrl &arg1);

    void on_pushButtonGet_clicked();

    void on_pushButtonSearch_clicked();

    //void on_pushButtonDivideWords_clicked();

    //void on_pushButtonInitRid_clicked();

    //void on_pushButtonInitUrls_clicked();

    void slotCrawlQuestion(const QString &title, const QString &content, int &ret);

    void slotCrawlWarning(const QString &title, const QString &content);

    void slotShowProgress(int p);

    void slotCrawlComplete(int pageCount);

    void slotCrawlBrowserAppend(const QString &str);

    void slotInitRidComplete(const QString &filename, int state);

    void on_pushButtonLoad_clicked();

private:
    Ui::Widget *ui;
    int pageNumber;//爬取网页总数
    bool working;//是否有子线程正在运行
    Jieba jieba;//jieba分词
    struct WordCount
    {
        std::string word;
        int count;
        WordCount(){count=0;}
        WordCount(std::string inword,int incount){word=inword;count=incount;}
    };

    /* crawler */
    bool initCrawler;//是否已初始化
    QString entry;//爬取网页起始位置
    int MAXFILECOUNT;//爬取文件数
    CrawlerThread *crawlThread;

    /* 分词 */
    void divide_text();

    /* 初始化倒排索引 */
    InitRidThread *initRidThread;
    void init_rid();

    //显示搜索结果
    //enum{AND,OR};
    enum{TITLETYPE, BODYTYPE};
    //int bool_search(QString &expr);
    void show_result(const vector<string> &targetWords, int fileNum);//显示单个网页的结果
    void show_results(int *resultPages, int szResultPages, vector<string> targetWords);//显示交集、对称差
    QString str_to_html(const QString& str, const vector<QString>& keywords, int fileNum, int type);//将普通字符串转为HTML格式
};

#endif // WIDGET_H
