#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

class MyClient : public QWidget
{
    Q_OBJECT
public:
    explicit MyClient(const QString& strHost, int nPort, QWidget* pwgt = 0) ;

private:
    QTcpSocket* m_pTcpSocket;//атрибут для управления клинетом
    QTextEdit* m_ptxtInfo;//отображение инофрмации
    QLineEdit* m_ptxtInput;//ввод информации
    quint16 m_nNextBlockSize;//длина следующего, полченного от сокета, блока
    QString filter;//фильтр для поиска по директории
    QByteArray byteArrayFromFile;//считанный из файла массив байт

private:
    QByteArray readFile(QString filePath);

private slots:
    void slotReadyRead ();
    void slotError (QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected ();
    void slotOpenDirectory();


signals:

};

#endif // MYCLIENT_H
