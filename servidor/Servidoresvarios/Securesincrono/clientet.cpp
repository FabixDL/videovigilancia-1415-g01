#include "clientet.h"


clienteT::clienteT(qintptr socketDescriptor, QSqlDatabase &bdd,QObject *parent):
    QThread(parent),
    socketDescriptor_(socketDescriptor),
    bddc(bdd),
    contador(0)
{
    Tpaquete =0;
    paquete.Clear();
    if (!bddc.open()) {
       qDebug() <<"No se pudo acceder a los datos";
       exit(1);
    }





}

void clienteT::run()
{
        QSslSocket tcpSocket;
        // Inicializarlo con el socket nativo de la conexión con el cliente
        if (tcpSocket.setSocketDescriptor(socketDescriptor_)) {
             QFile fileCert("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.crt");
          connect(&tcpSocket,SIGNAL(sslErrors(QList<QSslError>)),&tcpSocket,SLOT(ignoreSslErrors()));

          qDebug() <<"existe pem:"<<QFile::exists("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.crt");
          qDebug() <<" ceritifado abierto: " <<fileCert.open(QIODevice::ReadOnly);
          fileCert.close();
          QFile algo("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.key");
          qDebug() <<" Key abierta: " <<algo.open(QIODevice::ReadOnly);
          algo.close();
          tcpSocket.setProtocol(QSsl::AnyProtocol);
          tcpSocket.ignoreSslErrors();
          //tcpSocket.addDefaultCaCertificates("")
          tcpSocket.setPrivateKey("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.key");
          tcpSocket.setPeerVerifyMode(QSslSocket::QueryPeer);
          tcpSocket.addCaCertificates("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.crt");
          tcpSocket.setLocalCertificate("/home/fabix/Documentos/SOA/Proyecto_Videovigilancia/servidor/videovigilancia.crt");
          tcpSocket.startServerEncryption();
          qDebug() <<"certificado es nulo ?: "<<tcpSocket.localCertificate().isNull();
          qDebug() <<"soy valido ?: "<<tcpSocket.isValid();
          qDebug() <<"estoy encriptado ?: "<<tcpSocket.isEncrypted();
          qDebug() << "nuevaConexion";
          tcpSocket.waitForEncrypted();
          qDebug() <<"estoy encriptado ?: "<<tcpSocket.isEncrypted();

        } else {
            emit error(tcpSocket.error());
            return;

        }
       do {
        //qDebug() <<"ESPERANDO PARA LEER";

        tcpSocket.waitForReadyRead();
        //qDebug() <<"LEEMOS";
            //while(tcpSocket.bytesAvailable() > 0 ){
                //qDebug() <<"bytes disp"<< tcpSocket.bytesAvailable() ;

            if (tcpSocket.isEncrypted()){
                //qDebug() << "ESTOY ENCRIPTADO!!!";
                   deserializacion(&tcpSocket);

            }else{
                qDebug() << "O NO T_T";
                return;
            }


        }while(tcpSocket.isValid());
         while(tcpSocket.bytesAvailable() > 0){

             QString aux;
             QByteArray algo;
             QDataStream in(&tcpSocket);
             in.setVersion(QDataStream::Qt_4_0);
                 //Recojiendo en tamaño del paquete
              if(tcpSocket.bytesAvailable() >= (int)(sizeof(qint32))&& (Tpaquete==0))
              {
                  in >> Tpaquete;
                  aux=QString::number(Tpaquete);
                  qDebug() <<"tamaño paquete:"<< aux;
                 //Teniendo el tamaño de paquete lo leemos del buffer
              } if ((Tpaquete !=0) && (tcpSocket.bytesAvailable() >=Tpaquete )){
                 algo=tcpSocket.read(Tpaquete);
                 paquete.ParseFromString(algo.toStdString());
                 Tpaquete =0;
                 almacenamiento(paquete);

             }else{

                tcpSocket.readAll();
            }
         }

        tcpSocket.disconnectFromHost();
        tcpSocket.waitForDisconnected(100);
 }

void clienteT::deserializacion(QSslSocket *tcpSocket_)
{

    qDebug() << "DESERIALIZANDO";

    QString aux, aux3;
    std::string aux2;
    QByteArray algo;
    QDataStream in(tcpSocket_);
    in.setVersion(QDataStream::Qt_4_0);
        //Recojiendo en tamaño del paquete
    //qDebug() <<"bytes disponibles"<< tcpSocket_->bytesAvailable() ;
     if(tcpSocket_->bytesAvailable() >= (int)(sizeof(qint32))&& (Tpaquete==0))
     {
         in >> Tpaquete;
         aux=QString::number(Tpaquete);
         qDebug() <<"tamaño paquete:"<< aux;
        //Teniendo el tamaño de paquete lo leemos del buffer
     }
     if ((Tpaquete !=0) && (tcpSocket_->bytesAvailable() >=Tpaquete )){
        algo=tcpSocket_->read(Tpaquete);
        paquete.ParseFromString(algo.toStdString());
        Tpaquete =0;
        almacenamiento(paquete);

    }else

    return;
}

void clienteT::almacenamiento(VAF &paquete)
{
    QDir directorio;
    contador++;
    QByteArray buffer;
    buffer.append(paquete.imagen().c_str(),paquete.imagen().length());
    QImage im;
    im.loadFromData(buffer, "JPEG");

   //control de paquetes
    //qDebug() <<"nombre camara:"<< QString::fromStdString(paquete.nombrecamara());
    //qDebug()  << "nombre pc:"<< QString::fromStdString(paquete.nombrepc());
    //qDebug()  <<"Protocolo:" << QString::fromStdString(paquete.protocolo());
    //qDebug()  << "timestamp:" << QString::fromStdString(paquete.timestamp());
    //qDebug()  <<"timagen: " <<paquete.timagen();

    //introducciendo en la Base de Datos
    QSqlQuery query(bddc);
    query.prepare("INSERT INTO REGVAF (PRO,V,NCAMARA,NPC,DATESTAMP,TIMESTAMP,DIRECTORIO) "
                  "VALUES (:PRO,:V,:NCAMARA,:NPC,:DATESTAMP,:TIMESTAMP,:DIRECTORIO)");

    query.bindValue(":PRO",QString::fromStdString(paquete.protocolo()));
    query.bindValue(":V", QString::fromStdString(paquete.version()));
    query.bindValue(":NCAMARA", QString::fromStdString(paquete.nombrecamara()));
    query.bindValue(":NPC", QString::fromStdString(paquete.nombrepc()));
    query.bindValue(":TIMESTAMP", QString::fromStdString(paquete.timestamp()));
    query.bindValue(":DATESTAMP", QString::fromStdString(paquete.datestamp()));

    //Hayando la ruta de la foto
    QString pc=QString::fromStdString(
                QString::fromStdString(paquete.nombrepc()).toUtf8().toHex().toStdString());

    QString camara=QString::fromStdString(
                QString::fromStdString(paquete.nombrecamara()).toUtf8().toHex().toStdString());

    QString date=QString::fromStdString(
                QString::fromStdString(paquete.datestamp()).toUtf8().toHex().toStdString());

    std::string time= QString::fromStdString(paquete.timestamp()).toUtf8().toHex().toStdString();

    //qDebug() <<QString::fromStdString(paquete.timestamp());
    //qDebug() << QString::fromStdString(time);

    QString hora= QString::fromStdString(
                time.substr(0,6));

    QString minutos= QString::fromStdString(
                time.substr(6,6));

    QString segundos=QString::fromStdString(
                time.substr(12,6));
    QString segundosMs=QString::fromStdString(
                time.substr(18,time.size()-18));


    QString direct= QString(APP_VARDIR) +
            "/"+"clientes/" +pc+"/"+camara+"/"+date+"/"+hora+
            "/"+ minutos+"/"+segundos+"/" +segundosMs +".jpeg";
    query.bindValue(":DIRECTORIO",direct);

    directorio.mkpath(QString(APP_VARDIR) +"/"+"clientes/" +pc+"/"+camara+"/"+date+"/"+hora+
                      "/"+ minutos+"/"+segundos);


    //query.bindValue(":DIRECTORIO","algo");
    qDebug() << query.exec();
    //query.exec("SELECT nombre FROM contactos");


    qDebug() << im.save(direct);
    //Limpieza del paquete
    paquete.Clear();
    query.prepare("INSERT INTO ROI (DIRECTORIO,ANCHO,ALTO,CRX,CRY) "
                  "VALUES (:DIRECTORIO,:ANCHO,:ALTO,:CRX,:CRY)");
    query.bindValue(":DIRECTORIO",direct);
    query.bindValue(":ANCHO",1);
    query.bindValue(":ALTO",2);
    query.bindValue(":CRX",3);
    query.bindValue(":CRY",4);
     query.exec() ;
     query.prepare("INSERT INTO ROI (DIRECTORIO,ANCHO,ALTO,CRX,CRY) "
                   "VALUES (:DIRECTORIO,:ANCHO,:ALTO,:CRX,:CRY)");
     query.bindValue(":DIRECTORIO",direct);
     query.bindValue(":ANCHO",2);
     query.bindValue(":ALTO",3);
     query.bindValue(":CRX",4);
     query.bindValue(":CRY",5);
      query.exec() ;


}

