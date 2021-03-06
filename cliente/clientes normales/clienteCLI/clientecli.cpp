#include "clientecli.h"

ClienteCLI::ClienteCLI()
{
    devices = QCamera::availableDevices();
    NCamaras=devices.size();
    ListaCamaras=new QVector<CAM>;
    conexion=new QTcpSocket;

    QRegExp rx("(\\,|\\/|\\:|\\t)");
    QString sometext(getenv("SESSION_MANAGER"));
    QStringList query = sometext.split(rx);

    NombrePC=new QString(getenv("USER"));
    NombrePC->append("@");
    NombrePC->append(query.at(1));

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermSd))
        qFatal("Couldn't create TERM socketpair");
    // Crear el objeto para monitorizar el segundo socket
    // de la pareja.
    sigTermNotifier = new QSocketNotifier(sigTermSd[1],
        QSocketNotifier::Read, this);
    // Conectar la señal activated() del objeto QSocketNotifier
    // con el slot handleSigTerm() para manejar la señal. Esta
    // señal será emitida cuando hayan datos para ser leidos en
    // el socket monitorizado.
    connect(sigTermNotifier, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
    /*
    //qDebug() << getenv("USERNAME");
    //qDebug() << getenv("SESSION_MANAGER");
   // qDebug() << "NombrePC: "<< NnombrePC<< " size: " << sizeof(nombrePC);;
   */
}

void ClienteCLI::termSignalHandler(int)
{
    char a = 1; // Con enviar un byte es suficiente
    write(sigTermSd[0], &a, sizeof(a));
    //qDebug() << "termSignalHanlder";
}

void ClienteCLI::handleSigTerm()
{
    // Desactivar la monitorización del socket para que por
    // el momento no lleguen más señales de Qt
    sigTermNotifier->setEnabled(false);
    // Leer y desechar el byte enviado por
    // MyDaemon::termSignalHandler()
    //qDebug() << "handleSigTerm";
    char tmp;
    read(sigTermSd[1], &tmp, sizeof(tmp));
    //MI CODIGO
    delete_all();
    // Activar la monitorización para que vuelvan a llegar
    // señales de Qt
    qApp->quit();
    sigTermNotifier->setEnabled(true);
}

ClienteCLI::~ClienteCLI()
{

}

void ClienteCLI::actualizar_cam(int indice,bool valor)
{
    QString namesetting;

    namesetting="indice";
    namesetting.append(indice+48);
    //qDebug() << "ACT NAMESETTING: " << namesetting;
    settings.setValue(namesetting,valor);
}

void ClienteCLI::actualizar_puerto(int puerto)
{
    settings.setValue("PORT",puerto);
}

void ClienteCLI::actualizar_IP(QString IP)
{
    settings.setValue("IP",IP);
}

void ClienteCLI::actualizar(){
    if(parametros==false){
        QString *IP, *camaras;

        camaras=new QString("ALL");
        IP=new QString("127.0.0.1");
        int port=33333;

        for(int i=0; i<NCamaras;i++){
            actualizar_cam(i,false);
        }

        QFile file("cliente.conf");
        if (file.open(QIODevice::ReadOnly)){
            qDebug() << "Leyendo configuracion desde fichero...";
            QTextStream configuracion(&file);

            camaras= new QString(configuracion.readLine());

            QRegExp rx("(\\,)");
            QStringList query = (*camaras).split(rx);

            if(*camaras!="ALL"){
                for(int i=0; i<query.size(); i++){
                    for(int j=0; j<NCamaras;j++){
                        if(devices[j]==query.at(i)){
                            qDebug() << "Camara " << devices[j] << " detectada";
                            actualizar_cam(j,true);
                        }
                    }
                }
            }

            IP=new QString(configuracion.readLine());
            port=(configuracion.readLine()).toInt();
        }
        else
            qDebug() << "Error al acceder al fichero cliente.conf! Cargando configuración por defecto...";

        file.close();

        if(*camaras=="ALL"){
            for(int i=0; i<NCamaras;i++){
                    actualizar_cam(i,true);
                }
        }
        actualizar_puerto(port);
        actualizar_IP(*IP);
    }
    else
        qDebug() << "Procesados parametros por terminal";
}

void ClienteCLI::capturar()
{
    CAM aux;
    QString namesetting;

    qDebug() << "IP: " << settings.value("IP").toString() << " PORT: " << settings.value("PORT").toInt();
    conexion->connectToHost(settings.value("IP").toString(),settings.value("PORT").toInt());

    for(int i=0;i<NCamaras;i++)
    {
        namesetting="indice";
        namesetting.append(i+48);
        //qDebug() << "CAPT NAMESETTING: " << namesetting;
        if((settings.value(namesetting))==true){
            aux.Camera=new QCamera(devices[i]);
            aux.captureBuffer=new CaptureBuffer;
            aux.captureBuffer->id=i;
            ListaCamaras->push_back(aux);
            ListaCamaras->value(i).Camera->setViewfinder(ListaCamaras->value(i).captureBuffer);
            ListaCamaras->value(i).Camera->start();
            qDebug() << "Transmitiendo de " << QCamera::deviceDescription(devices[i]) << "(" << devices[i] << ")";
            connect(ListaCamaras->value(i).captureBuffer,SIGNAL(s_image(QImage,int)),this,SLOT(emitir(QImage,int)));
        }
    }
}

void ClienteCLI::emitir(const QImage &image, int id){

    //required string protocolo = 1;
    //required bytes  version = 2;
    //optional uint32  Tnombrecamara = 3 ;
    //required string  nombrecamara = 4;
    //optional uint32  TnombrePC = 5 ;
    //required string  nombrePC = 6;
    //required string  timestamp = 7;
    //optional uint32  TImagen=8;
    //required string  imagen=9;

    QBuffer buffer;
    QImageWriter writer;
    std::string spaquete;
    VAF paquete;

    paquete.set_protocolo(NPROTOCOLO);
    paquete.set_version(VPROTOCOLO);

    //QString dprotocolo(paquete.protocolo().c_str());
    //QString diversion(paquete.version().c_str());
    //qDebug() << dprotocolo << diversion;

    std::string nombrecamara((QCamera::deviceDescription(devices[id])).toStdString());
    paquete.set_tnombrecamara(sizeof(nombrecamara));
    paquete.set_nombrecamara(nombrecamara);

    //qint32 dtnombrecamara(paquete.tnombrecamara());
    //QString dnombrecamara(paquete.nombrecamara().c_str());
    //qDebug() << dnombrecamara << dtnombrecamara;

    paquete.set_tnombrepc(sizeof(NombrePC->toStdString()));
    paquete.set_nombrepc(NombrePC->toStdString());

    //qint32 dtnombrepc(paquete.tnombrepc());
    //QString dnombrepc(paquete.nombrepc().c_str());
    //qDebug() << dnombrepc << dtnombrepc;

    paquete.set_timestamp((QTime::currentTime().toString("hh:mm:ss:zzz")).toStdString());
    paquete.set_datestamp((QDate::currentDate().toString("dd.MM.yyyy")).toStdString());

    //QString dtime(paquete.timestamp().c_str());
    //qDebug() << dtime;
    writer.setDevice(&buffer);
    writer.setFormat("jpeg");
    writer.setCompression(70);
    writer.write(image);
    QByteArray bimagen = buffer.buffer();
    //qDebug()<< "imagen:"<< bimagen.size();
    //QString imagen(bimagen);
    paquete.set_timagen(bimagen.size());
    paquete.set_imagen(bimagen.data(),bimagen.size());

    //qint32 dtimagen((paquete.timagen()));
    //QString dimagen(paquete.imagen().c_str());
    //qDebug() << dimagen << dtimagen;

    paquete.SerializeToString(&spaquete);

    QByteArray bpaquete(spaquete.c_str(),spaquete.size());
    qint32 tbpaquete = bpaquete.size();
    //qDebug() << "TBSIZE: " << tbpaquete;
    //QByteArray btbpaquete;
    //btbpaquete.append((const char*)&tbpaquete,sizeof(qint32));
    //btbpaquete.append('\n');

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint32)tbpaquete;
    out << tbpaquete;

    conexion ->write(block);
    //qDebug() << "sizeof mandado OK";
    conexion->write(bpaquete);
    //qDebug() << "bpaquete mandado OK";

}

void ClienteCLI::delete_all(){
    QString namesetting;
    for(int i=0;i<ListaCamaras->size();i++)
    {
        namesetting="indice";
        namesetting.append(i+48);
        if((settings.value(namesetting))==true){
            ListaCamaras->value(i).Camera->stop();   
            ListaCamaras->remove(i);
            qDebug() << "Camara " << i << " eliminada!";
        }
    }
    ListaCamaras->clear();
}
