#include "myserver.h"
MyServer::MyServer(int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt), m_nNextBlockSize(0){

    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
        "Server Error",
        "Unable to start the server:"+ m_ptcpServer->errorString());
        m_ptcpServer->close();
        return;
    }
    //если прослушивание без ошибок, соединяем со слотом slotNewConnection()
    connect(m_ptcpServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));

    m_ptxt = new QTextEdit;//для отображения информации
    m_ptxt->setReadOnly(true);

    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;

    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    pvbxLayout->addWidget(m_ptxt);

    setLayout(pvbxLayout);
}

/*virtual*/ void MyServer::slotNewConnection()//вызывается каждый раз для нового клиента
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();//подтверждение соединения с клиентом
    //nextPendingConnection() возвращает сокет, по которому идет связь с клиентом
    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));//слот для отсоединения кллиента
    connect(pClientSocket, SIGNAL(readyRead()),this, SLOT(slotReadClient()));//слот для чтения запросов клиента

    sendToClient(pClientSocket, "Server Response: Connected!");
}

void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();//преобразование указателя,
    //возвращаемого методом sender(), к типу QTcpSocket
    QDataStream in(pClientSocket);
    /*
Цикл for нужен потому, что не все высланные клиентом данные могут прийти одновременно.
Поэтому сервер должен "уметь" получать как весь блок целиком, так и только часть блока,
а также и все блоки сразу. Каждый переданный сокетом блок начинается полем размера блока.
Размер блока считывается при условии, что размер полученных данных не меньше двух байт и
атрибут m_nNextBlockSize равен нулю (т. е. размер блока неизвестен). Если размер доступных
для чтения данных больше или равен размеру блока, тогда данные считываются из потока в переменные time и str.
Затем значение переменной time преобразуется вызовом метода toString() в строку
и вместе со строкой str записывается в строку сообщения strMessage, которая добавляется в виджет
текстового поля вызовом метода append(). Анализ блока данных завершается присваиванием атрибуту
m_nNextBlockSize значения 0, которое говорит о том, что размер очередного блока данных неизвестен.
Вызов метода sendToClient() сообщает клиенту о том, что были успешно прочитаны высланные им данные.
*/
    for (;;) {
        if (!m_nNextBlockSize) {
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;
        QString strMessage = time.toString() + " " + "Client has sent — " + str;
        m_ptxt->append(strMessage);
        m_nNextBlockSize = 0;
        sendToClient(pClientSocket, "Server Response: Received \"" + str + "\"");
    }
}

void MyServer::sendToClient(QTcpSocket* pSocket, const QString& str)//формируются данные для отправки клиенту
{
    /*
Заранее не известен размер блока, а следовательно, невозможно записывать данные сразу в сокет,
т. к. размер блока должен быть выслан в первую очередь. Чтобы досчить цели, сначала создаем объект
arrBlock класса QByteArray. На его основе создается объект класса QDataStream,
в который записывается все данные блока, причем вместо реального размера записывается 0.
После этого указатель перемещается на начало блока вызовом метода seek(), вычисляется размер блока
как размер arrBlock, уменьшенный на sizeof(quint16), и записывается в поток (out) с текущей позиции,
которая уже перемещена в начало блока. После этого созданный блок записывается в сокет вызовом метода write()
*/
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    quint16 sizeofQuint16 = sizeof(quint16);
    out << quint16(arrBlock.size()-sizeofQuint16);

    pSocket->write(arrBlock);
}
