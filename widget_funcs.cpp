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
#include "initridthread.h"

/* 对text目录下的文本文件进行分词 */
void Widget::divide_text()
{
    qDebug() << "pageNumber=" << pageNumber;
    ui->textBrowser->clear();
    ui->labelStatus->setText("正在分词...");

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(pageNumber);
    ui->progressBar->setValue(0);

    string indexDir = entry.toStdString();
    indexDir += "/text-index";
    if (_access(indexDir.c_str(), 0) != -1){
        int ret = QMessageBox::question(this,tr("文件已存在"),tr("%1目录已存在，您是否要覆盖它？").arg(QString::fromStdString(indexDir)),QMessageBox::Yes|QMessageBox::No);
        if(ret==QMessageBox::Yes)
        {
            _rmdir(indexDir.c_str());
            _mkdir(indexDir.c_str());
        }
        else
            return;
    }
    else //文件夹不存在
        _mkdir(indexDir.c_str());

    /* 对text目录下的所有文件进行分词 */
    for(int fileNum=0;fileNum<pageNumber;fileNum++)
    {
        ui->textBrowser->append(QObject::tr("解析%1/text/%2.txt...\n").arg(entry).arg(fileNum));

        /* 打开网页文本文件 */
        QString filename = QObject::tr("%1/text/%2.txt").arg(entry).arg(fileNum);
        QFile f(filename);
        if(!f.open(QIODevice::Text|QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("打开网页文本"), tr("打开网页文本文件%1错误：").arg(filename) + f.errorString());
            qDebug() << tr("Open failed.") << endl;
            return;
        }

        /* 读入文件并分词，存在wordcounts容器中 */
        QTextStream txtInput(&f);
        txtInput.setCodec("UTF-8");
        QString lineStr;
        vector<WordCount> wordcounts;
        while(!txtInput.atEnd())
        {
            lineStr = txtInput.readLine();
            //ui->textBrowser->append(QObject::tr("%1<Line End>\n").arg(lineStr));

            vector<string> words;
            jieba.CutForSearch(lineStr.toStdString(),words,true);

            int sz = words.size();
            //qDebug() << tr("words.size()=%1").arg(words.size()) << endl;
            for(int i=0;i<sz;i++)
            {
                if(words[i]==" ")
                    continue;

                bool flag = false;
                for(int k=0;k<wordcounts.size();k++)
                    if(wordcounts[k].word==words[i])
                    {
                        wordcounts[k].count++;
                        flag = true;
                    }

                if(!flag)
                {
                    WordCount wc(words[i],1);
                    wordcounts.push_back(wc);
                }
            }
            //qDebug() << tr("wordcounts.size()=%1").arg(wordcounts.size()) << endl;
        }
        f.close();

        ui->textBrowser->append(QObject::tr("Cut Finished\n"));

        /* 打开分词目标文件 */
        QString indexfilename = QObject::tr("%1/text-index/%2-index.txt").arg(entry).arg(fileNum);
        QFile indexfile(indexfilename);
        if(!indexfile.open(QIODevice::Text | QIODevice::WriteOnly)) //若要输出换行，需使用QIODevice::Text
        {
            QMessageBox::warning(this, tr("打开分词目标文件"), tr("打开分词目标文件%1错误：").arg(indexfilename) + indexfile.errorString());
            qDebug() << tr("Open failed.") << endl;
            return;
        }

        /* 将wordcounts写入分词目标文件 */
        //qDebug() << tr("wordcountSize=%1").arg(wordcounts.size()) << endl;
        int wordcountsSize = wordcounts.size();
        QTextStream wcOutput(&indexfile);
        wcOutput.setCodec("UTF-8");
        for(int i=0;i<wordcountsSize;i++)
        {
            QString result = QObject::tr("%1\t\t\t%2\n").arg(QString::fromStdString(wordcounts[i].word)).arg(wordcounts[i].count);
            wcOutput << result;
            //ui->textBrowser->append(result);
        }
        indexfile.close();

        ui->progressBar->setValue(fileNum+1);
    }

    ui->labelStatus->setText("分词完成");
    QMessageBox::information(this,tr("分词"),tr("分词已完成"));
}

/* 显示单个网页的搜索结果 */
void Widget::show_result(const vector<string> &targetWords, int fileNum)
{
    int leftpace = 20;
    int rightpace = 20;

    /* 打开网页并读入 */
    QString fileName = QObject::tr("%1/text/%2.txt").arg(entry).arg(fileNum);
    QFile f( fileName );
    if(!f.open(QIODevice::Text|QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("打开网页"), tr("打开网页文本文件%1错误：").arg(fileName) + f.errorString());
        qDebug() << tr("Open failed.") << endl;
        return;
    }
    QTextStream txtInput(&f);
    txtInput.setCodec("UTF-8");
    QString text = txtInput.readAll();
    //qDebug() << fileName << " " << QString::fromStdString(get_url(fileNum));
    //qDebug() << text;
    f.close();

    /* 将元素类型为string的targetWords转为元素为QString的keywords以便后续使用 */
    int szTargetWords = targetWords.size();
    vector<QString> keywords;
    for(int i=0;i<szTargetWords;i++)
        keywords.push_back(QString::fromStdString(targetWords[i]));

    /* 对本网页分词，结果在textWords中 */
    vector<Word> textWords;
    jieba.CutForSearch(text.toStdString(), textWords, true);
    int szTextWords = textWords.size();
    for(int i=0;i<szTextWords;i++)//按照offset先后排序
    {
        int least = i;
        for(int j=i;j<szTextWords;j++)
            if(textWords[j].unicode_offset<textWords[least].unicode_offset)
                least = j;
        if(least!=i)
            swap(textWords[i],textWords[least]);
    }

    /* 遍历textWords，将关键词相关的语句放到body中 */
    QString body;
    int begin = -1;
    int end = 0;
    //qDebug() << "targetWords.size()=" << targetWords.size();
    for(int j=0;j<textWords.size();j++)
    {
        //qDebug() << "textWords.size()=" << textWords.size();
        for(int i=0;i<targetWords.size();i++)
            if(targetWords[i]==textWords[j].word)
            {
                int offset = textWords[j].unicode_offset;
                int length = textWords[j].unicode_length;
                //qDebug() << "word:" << QString::fromStdString(targetWords[i]) << ",offset=" << offset << ",length=" << length;
                if(begin<=offset&&offset+length<=end)//若本词已经显示，则直接跳过
                    continue;
                begin = (offset-leftpace>=end)?(offset-leftpace):end;
                end = (offset+length+rightpace<text.size())?(offset+length+rightpace):text.size();

                body += text.mid(begin, end-begin);
                body += "...";
                body += "\n";
                //qDebug() << "begin=" << begin << "end:";
                //qDebug() << body << endl;
            }
    }

    /* 搜索结果的标题为该网页的URL */
    QString title = QObject::tr("http://%1").arg(QString::fromStdString(get_url(fileNum)));

    /* 向textBroswer输出标题和搜索结果 */
    ui->textBrowser->append( str_to_html(title, keywords, fileNum, TITLETYPE) );
    ui->textBrowser->append( str_to_html(body, keywords, fileNum, BODYTYPE) );
}

/* 将字符串根据其类型转为HTML语言 */
QString Widget::str_to_html(const QString& str, const vector<QString>& keywords, int fileNum, int type)
{
    QString html = QObject::tr("<p><pre>");
    if(type==TITLETYPE)
    {
        QString url = tr("http://%1").arg(QString::fromStdString(get_url(fileNum)));
        html += QObject::tr("<a href=\"%1\">").arg(url);
        for(int i=0;i<str.size();i++)
        {
            int flag = -1;
            for(int j=0;j<keywords.size();j++)
                for(int k=0;k<keywords[j].size();k++)
                    if(str[i]==keywords[j][k])
                    {
                        flag = j;
                        break;
                    }
            if(flag>=0)
                html += QObject::tr("<font family=\"Microsoft YaHei\" color=\"red\" size=\"5\">%1</font>").arg(str[i]);
            else
                html += QObject::tr("<font family=\"Microsoft YaHei\" color=\"blue\" size=\"5\">%1</font>").arg(str[i]);
        }
        html += QObject::tr("</a>");
        html += QObject::tr("<p><pre><font family=\"Microsoft YaHei\" color=\"green\" size=\"4\">PageRank=%1,Indegree=%2</font></pre></p>").arg(get_pr(fileNum)).arg(get_in_degree(fileNum));
    }
    else if(type==BODYTYPE)
    {
        for(int i=0;i<str.size();i++)
        {
            int flag = -1;
            for(int j=0;j<keywords.size();j++)
                for(int k=0;k<keywords[j].size();k++)
                    if(str[i]==keywords[j][k])
                    {
                        flag = j;
                        break;
                    }
            if(flag>=0)
                html += QObject::tr("<font family=\"Microsoft YaHei\" color=\"red\" size=\"3\">%1</font>").arg(str[i]);
            else
                html += QObject::tr("<font family=\"Microsoft YaHei\" color=\"black\" size=\"3\">%1</font>").arg(str[i]);
        }
    }
    else
        qDebug() << QObject::tr("No such type: type = %1").arg(type) << endl;

   html += QObject::tr("</pre></p>");
   return html;
}

/* 初始化倒排索引，调用initRidThread子线程 */
void Widget::init_rid()
{
    entry = ui->lineEditSource->text();
    ui->labelStatus->setText("正在建立倒排索引...");
    ui->progressBar->setRange(NOT_START,RID_COMPLETE);
    ui->progressBar->setValue(NOT_START);

    working = true;
    emit sendInitRidParameter(entry,pageNumber);
    initRidThread->start();

}

/* 显示所有搜索到的网页 */
void Widget::show_results(int *resultPages, int szResultPages, vector<string> targetWords)
{
    sort_by_pagerank(resultPages, szResultPages);//根据PageRank排序
    sort_by_in_degree(resultPages, szResultPages);//根据入度排序
    for(int i=0;i<szResultPages;i++)
    {
        //QString fileName = QObject::tr("%1/text/%2.txt").arg(entry).arg(resultPages[i]);
        //QString title = QObject::tr("http://%1").arg( QString::fromStdString(get_url(resultPages[i])) );
        //qDebug() << "title=" << title;
        show_result(targetWords, resultPages[i]);
        ui->progressBar->setValue( ui->progressBar->value()+1 );
    }
}

/*
// 布尔检索
int Widget::bool_search(QString &expr)
{
    if( expr.contains(" *") )
        return AND;
    else
        return OR;
}
*/
