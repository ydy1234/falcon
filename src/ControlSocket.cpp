// =====================================================================================
// 
//       Filename:  ControlSocket.cpp
//
//    Description:  控制端套接字
//
//        Version:  1.0
//        Created:  2013年08月23日 22时08分08秒
//       Revision:  none
//       Compiler:  cl
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#include <QtGui>

#include "ControlSocket.h"
#include "MainWindow.h"
#include "FalconXml.h"

ControlSocket::ControlSocket(MainWindow *mainWindow)
{
	this->mainWindow = mainWindow;

	connect(this, SIGNAL(connected()), this, SLOT(sendRequest()));
	connect(this, SIGNAL(disconnected()), this, SLOT(connFailed()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connError()));
	connect(this, SIGNAL(readyRead()), this, SLOT(getServerInfo()));
}

ControlSocket::~ControlSocket()
{

}

void ControlSocket::sendRequest()
{
	mainWindow->setStatusLabel(tr("  成功连接到服务器"));

	qDebug() << "成功连接到服务器，向服务器请求用户连接数据\r\n";
}

void ControlSocket::getServerInfo()
{
	QByteArray recvData;

	recvData = this->readAll();
	
	qDebug() << "收到数据：" << recvData << "\r\n";

	QVector<GroupWithClient> vectClient;
	switch (FalconXml::getXmlInfoType(recvData)) {
		// 客户端列表信息包
		case ClientListInfo:
			FalconXml::ResolvingClientListInfoXml(recvData, vectClient);
			mainWindow->setClientList(vectClient);
			break;
		// 客户端上报处理结果信息包
		case ClientDisposeInfo:
			FalconXml::ResolvingClientDisposeInfoXml(recvData);
			break;
		// 桌面截图信息包
		case DesktopScreen:
			FalconXml::ResolvingDesktopScreenXml(recvData);
			break;
		// 客户端网络流量信息包
		case FluxInfo:
			FalconXml::ResolvingFluxInfoXml(recvData);
			break;
		// 进程信息包
		case ProcessInfo:
			FalconXml::ResolvingProcessInfoXml(recvData);
			break;
		// 硬件信息包
		case hardInfo:
			FalconXml::ResolvinghardInfoXml(recvData);
			break;
		// 日志信息包
		case LogInfo:
			FalconXml::ResolvingLogInfoXml(recvData);
			break;
	}
}

void ControlSocket::connFailed()
{
	qDebug() << "服务器连接套接字被关闭\r\n";
	this->close();
}

void ControlSocket::connError()
{
	qDebug() << "服务器套接字错误：" << this->errorString() << "\r\n";
	mainWindow->showTrayInfo("连接服务器错误：" + this->errorString());
	mainWindow->setStatusLabel(QString(tr("  连接服务器错误：")) + this->errorString());
}