#ifndef INITRIDTHREAD
#define INITRIDTHREAD

#include <qthread>
#include <qdebug>
using namespace std;

class InitRidThread : public QThread
{
    Q_OBJECT
private:
    int init_stat;
    QString entry;
    int webNum;
    void run();

public:
    explicit InitRidThread(QObject *parent = 0);
    void RID_initiation(int webNum);
    bool writeRID(fstream &fn);
    void RID_initiator_internal(int webNum);
    int mark(string token, int *request);

signals:
    void showProgress(int p);
    void initRidComplete(const QString &filename, int state);
public slots:
    void slotInitRidParameter(const QString &str, int pageNumber);
};

#endif // INITRIDTHREAD

