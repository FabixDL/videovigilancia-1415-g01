#include "client.h"

client::client(QTcpSocket *tcpSocket,QSqlDatabase &bdd, QObject *parent) : QObject(parent),
    tcpSocket_(tcpSocket),bddc(bdd)
{
     Tpaquete =0;
    connect(tcpSocket_, SIGNAL(readyRead()), this, SLOT(deserializacion()));
    connect(tcpSocket_,SIGNAL(disconnected()), this, SLOT(borrarlista()));


    if (!bddc.open()) {
        qDebug() <<"No se pudo acceder a los datos";
        exit(1);
    }

}
client::~client()
{

}

qintptr client::getsocketDescriptor()
{
    return tcpSocket_->socketDescriptor();
}

void client::borrarlista()
{
    emit eliminar(tcpSocket_->socketDescriptor());
}
void client::deserializacion()
{


    QString aux, aux3;

    std::string aux2;
    QByteArray algo;
    QDataStream in(tcpSocket_);
    in.setVersion(QDataStream::Qt_4_0);
        //Recojiendo en tamaño del paquete
     if(tcpSocket_->bytesAvailable() >= (int)(sizeof(qint32))&& (Tpaquete==0))
     {
         in >> Tpaquete;
         aux=QString::number(Tpaquete);
         qDebug() <<"tamaño paquete:"<< aux;
        //Teniendo el tamaño de paquete lo leemos del buffer
     }else if ((Tpaquete !=0) && (tcpSocket_->bytesAvailable() >=Tpaquete )){
        algo=tcpSocket_->read(Tpaquete);
        paquete.ParseFromString(algo.toStdString());
        Tpaquete =0;
        almacenamiento(paquete);

    }else

    return;
}

void client::almacenamiento(VAF paquete)
{
    QDir directorio;
    QByteArray buffer;
    buffer.append(paquete.imagen().c_str(),paquete.imagen().length());
    QImage im;

    im.loadFromData(buffer, "JPEG");


    //control de paquetes
    qDebug() <<"nombre camara:"<< QString::fromStdString(paquete.nombrecamara());
    qDebug()  << "nombre pc:"<< QString::fromStdString(paquete.nombrepc());
    qDebug()  <<"Protocolo:" << QString::fromStdString(paquete.protocolo());
    qDebug()  << "timestamp:" << QString::fromStdString(paquete.timestamp());
    qDebug()  <<"timagen: " <<paquete.timagen();

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
    QString pc=QString::fromStdString(QString::fromStdString(paquete.nombrepc()).toUtf8().toHex().toStdString());
    QString camara=QString::fromStdString(QString::fromStdString(paquete.nombrecamara()).toUtf8().toHex().toStdString());
    QString date=QString::fromStdString(QString::fromStdString(paquete.datestamp()).toUtf8().toHex().toStdString());
    QString time= QString::fromStdString(QString::fromStdString(paquete.timestamp()).toUtf8().toHex().toStdString());
    QString direct= QString(APP_VARDIR) +"/"+"clientes/" +pc+"/"+camara+"/"+date+"/"+time+".jpeg";
    query.bindValue(":DIRECTORIO",direct);
    qDebug() << query.exec();
    directorio.mkpath(QString(APP_VARDIR) +"/"+"clientes/" +pc+"/"+camara+"/"+date);
    qDebug() << im.save(direct);
    //Limpieza del paquete
    paquete.Clear();


}


