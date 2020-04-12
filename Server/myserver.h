#ifndef MYSERVER_H
#define MYSERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QDebug>
#include <QByteArray>
#include <QTcpSocket>
#include <QTextEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
class MyServer : public QWidget
{
    Q_OBJECT
public:
    MyServer(int nPort, QWidget* pwgt = 0);

private:
    QTcpServer* m_ptcpServer;  //основа управления сервером
    QTextEdit* m_ptxt;         //многострочное тектовое поле, для отобрадения событий
    quint16 m_nNextBlockSize;  //длина следующего блока полученного от сокета

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

public slots:
    virtual void slotNewConnection();
    void slotReadClient ();

signals:

};

#endif // MYSERVER_H
