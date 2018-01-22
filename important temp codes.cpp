void Widget::on_pushButtonSearch_clicked()
{
    if(working)
    {
        QMessageBox::warning(this, tr("请稍等"), tr("请等待当前进程结束"));
        return;
    }

    ui->textBrowser->clear();

    /* 判断布尔检索类型 */
    QString targetSentence = ui->lineEditTarget->text();
    int searchType = bool_search(targetSentence);
    targetSentence.remove(" *");
    targetSentence.remove(" +");

    /* 对目标进行分词 */
    vector<string> targetWords;
    string sentence = targetSentence.toStdString();
    jieba.CutForSearch(sentence, targetWords, true);
    for(vector<string>::iterator iter = targetWords.begin(); iter != targetWords.end(); iter++){
        if(*iter == " ")
            targetWords.erase(iter);
    }

    /* 将vector类型转为动态数组供倒排索引使用 */
    int szRequest = targetWords.size();
    string *request = new string [szRequest];
    for(int i=0;i<szRequest;i++)
        request[i] = targetWords[i];
    qDebug() << QString::fromStdString(request[0]);

    /* 倒排索引查找网页 */
    ui->labelStatus->setText("正在检索倒排索引...");
    int szTargetWords = targetWords.size();
    QString targets;
    for(int i=0;i<szTargetWords;i++)
    {
        qDebug() << QString::fromStdString(targetWords[i]) << ",";
        targets += QString::fromStdString(targetWords[i]);
        targets += ",";
    }
    cout << endl;
    QMessageBox::information(this, tr("关键词"),
                             tr("搜索目标分词完成，共得到关键词%1个\n%2").arg(szTargetWords).arg(targets));
    int *intersection;
    int szIntersection = 0;
    int *symDifference;
    int szSymdifference = 0;
    int szUnion = RI_itsc_symdif(request, szRequest, intersection, szIntersection, symDifference, szSymdifference);
    ui->labelStatus->setText("倒排索引检索完成");
    QMessageBox::information(this,tr("倒排索引"),
                             tr("倒排索引检索完成，交集%1个，对称差%2个，并集%3个\n").arg(szIntersection).arg(szSymdifference).arg(szUnion));

    /* 显示搜索结果 */
    ui->textBrowser->clear();
    ui->labelStatus->setText("正在搜索...");
    ui->progressBar->setValue(0);

    switch( searchType )
    {
    case AND:
        if(szIntersection<=0)
        {
            ui->progressBar->setRange(0,100);
            ui->progressBar->setValue(100);
            ui->labelStatus->setText("搜索完成");
            QMessageBox::information(this, tr("搜索完成"), tr("搜索完成，未找到相关结果"));
            break;
        }
        ui->progressBar->setRange(0, szIntersection);
        show_results(intersection, szIntersection, targetWords);
        break;
    case OR:
        if(szUnion<=0)
        {
            ui->progressBar->setRange(0,100);
            ui->progressBar->setValue(100);
            ui->labelStatus->setText("搜索完成");
            QMessageBox::information(this, tr("搜索完成"), tr("搜索完成，未找到相关结果"));
            return;
        }
        ui->progressBar->setRange(0, szUnion);
        show_results(intersection, szIntersection, targetWords);
        show_results(symDifference, szSymdifference, targetWords);
        break;
    default: qDebug() << "Error! From bool_search";
    }

    ui->labelStatus->setText("搜索完成");

}