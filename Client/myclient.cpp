#include "myclient.h"

MyClient::MyClient(const QString& strHost, int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt), m_nNextBlockSize(0){
    m_pTcpSocket = new QTcpSocket(this);//создается сокет
    m_pTcpSocket->connectToHost(strHost, nPort);//соединение клиента с хостом
    //в случае успешного соединения испускается сигнал connected(), в противном случае - error(код_ошибки)
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));

    //создание интерфейса
    m_ptxtInfo = new QTextEdit;
    m_ptxtInput = new QLineEdit;
    m_ptxtInfo->setReadOnly(true);
    //кнопка для отправки серверу введенных данных
    QPushButton* pcmd = new QPushButton("&Send");
    //соединение нажатия кнопки со слотом slotSendToServer(), котроый отправляет данные на сервер
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));
    //для клавиши Enter аналогичные действия
    connect(m_ptxtInput, SIGNAL(returnPressed()), this, SLOT(slotSendToServer()));

    //кнопка для открытия директории
    QPushButton* openDir = new QPushButton("&Open");
    connect(openDir, SIGNAL(clicked()), SLOT(slotOpenDirectory()));


    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;

    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(m_ptxtInput);
    pvbxLayout->addWidget(pcmd);
    pvbxLayout->addWidget(openDir);

    setLayout(pvbxLayout);
}
//слот для работы с директорией
void MyClient::slotOpenDirectory(){
    filter = "All Files (*.*)";
    QString filePath =  QFileDialog::getOpenFileName(this,"Choose file",
                                                      "C://", filter);
    if (filePath!=""){
        byteArrayFromFile = readFile(filePath);
    }

    else {
        QMessageBox msgBox;
        msgBox.setText("You have not selected a file");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
}
//метод, читающий файл и преобразующий его в массив байт
//todo максимальный размер QByteArray=10.000, что мало. необходимо разбивать по пакетам.
QByteArray MyClient::readFile(QString filePath){
    QFile file(filePath);

    QByteArray dataArray;
    if (!file.open(QIODevice::ReadOnly)){
        QMessageBox msgBox;
        msgBox.setText("Error while open file");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return dataArray;
    }
    //int i=0;
    while(!file.atEnd()){
        dataArray += file.readLine();
    }
    return dataArray;
}


void MyClient::slotReadyRead(){
    QDataStream in(m_pTcpSocket);
    /*
    * Цикл нужен потому, что с сервера не все данные могут прийти одновременно.
    * Поэтому клиент, как и сервер, должен быть в состоянии получать как весь блок
    * целиком, так и только часть блока или даже все блоки сразу.
    * Каждый принятый блок начинается полем размера блока.
    * Когда блок получен целиком,можно без опасения использовать оператор >>
    * объекта потока QDataStream (переменная in).
    * Чтение данных из сокета осуществляется при помощи объекта потока данных.
    * Полученная информация добавляется в виджет многострочного текстового поля (указатель m_ptxtInfo) с помощью метода append().
    * В завершение анализа блока данных атрибуту m_nNextBlockSize присваивается значение 0, которое говорит о том,
    * что размер очередного блока данных неизвестен.
   */
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
        in >> m_nNextBlockSize;
        }
        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
     QTime time;
     QString str;
     in >> time >> str;
     m_ptxtInfo->append(time.toString() + " " + str);
     m_nNextBlockSize = 0;
     }
}
//слот, обрабатывающий ошибки
void MyClient::slotError(QAbstractSocket::SocketError err){
    QString strError =
    "Error: " + (err == QAbstractSocket::HostNotFoundError ?
        "The host was not found." :
            err == QAbstractSocket::RemoteHostClosedError ?
                "The remote host is closed." :
                    err == QAbstractSocket::ConnectionRefusedError ?
                        "The connection was refused." :
                            QString(m_pTcpSocket->errorString()));
    m_ptxtInfo->append(strError);
}
//слот отправки данных серверу
//по аналогии с MyServer::sendToClient()
void MyClient::slotSendToServer()
{
//    QByteArray arrBlock;
//    QDataStream out(&arrBlock, QIODevice::WriteOnly);
//    //out.setVersion(QDataStream::Qt_4_5);
//    out << quint16(0) << QTime::currentTime() << m_ptxtInput->text();

//    out.device()->seek(0);
//    quint16 sizeofQuint16 = sizeof(quint16);
//    out << quint16(arrBlock.size()-sizeofQuint16);

//    m_pTcpSocket->write(arrBlock);
//    m_ptxtInput->setText("");
//-----------------------------------
    QByteArray arrBlock;
    m_ptxtInput->setText(byteArrayFromFile.data());//--------------------подмена
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << quint16(0) << QTime::currentTime() << m_ptxtInput->text();

    out.device()->seek(0);
    quint16 sizeofQuint16 = sizeof(quint16);
    out << quint16(arrBlock.size()-sizeofQuint16);

    m_pTcpSocket->write(arrBlock);
    m_ptxtInput->setText("");
}

//слот для отображения подтверждения подключения к серверу
void MyClient::slotConnected(){
    m_ptxtInfo->append("Received the connected() signal");
}

