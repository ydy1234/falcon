// =====================================================================================
// 
//       Filename:  MainWindow.cpp
//
//    Description:  主窗口的类实现文件
//
//        Version:  1.0
//        Created:  2013年08月18日 21时11分08秒
//       Revision:  none
//       Compiler:  cl
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#include <QtGui>
#include <QHttp>

#include "MainWindow.h"
#include "LinkTreeWidget.h"
#include "MainStackWidget.h"
#include "ControlSocket.h"
#include "MainListWidget.h"
#include "ProcessTreeView.h"
#include "LinkProcessWidget.h"
#include "LinkCmdWidget.h"
#include "ScreenViewWidget.h"
#include "FluxViewWidget.h"
#include "InfoWidget.h"
#include "FalconType.h"
#include "SettingDialog.h"
#include "FalconXml.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)	
{
	readSettings();

	// 设置窗口大小
	resize(1000, 650);
	//setFixedSize(1000,650);
	createMainWeiget();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createSystemTrayIcon();
	
	setWindowIcon(QIcon(":/res/ico/main.ico"));
	setWindowTitle(tr("Falcan 企业网络监控系统 -- Alpha 1"));
	setUIEnable(false);
	tryConnServer();
}

MainWindow::~MainWindow()	
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (systemTrayIcon == NULL || isTrayExit) {
		event->accept();
		writeSettings();
	} else {
		int result = QMessageBox::question(this, tr("Falcan 企业网络监控系统"),
						tr("<H3>您点击了关闭按钮。</H3>要最小化到托盘吗？"),
						QMessageBox::Yes | QMessageBox::No);
		if (result == QMessageBox::Yes) {
			event->ignore();
			this->hide();
			showTrayInfo(tr("Falcon 最小化到这里了，点击图标显示主界面。"));
		} else {
			systemTrayIcon->hide();
			writeSettings();
			event->accept();
		}
	}
}

ControlSocket *MainWindow::getConnSocket()
{
	return clientSocket;
}

MainStackWidget *MainWindow::getStackWidget()
{
	return mainStackWidget;
}

void MainWindow::setUIEnable(bool isEnabled)
{
	mainToolBar->setEnabled(isEnabled);
	linkTreeWidget->setEnabled(isEnabled);
	mainStackWidget->setEnabled(isEnabled);
	linkButton->setEnabled(!isEnabled);
	auto_connAction->setEnabled(!isEnabled);
}

/*================================================================
*  函 数 名： MainWindow::connectInfo
*
*  功能描述： 切换到连接信息选项卡
*
*  参    数： 无	  
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::connectInfo()
{
	(mainStackWidget->getStackLayout())->setCurrentIndex(0);
}

/*================================================================
*  函 数 名：  MainWindow::screenShot
*
*  功能描述： 切换到屏幕捕获选项卡
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::screenShot()
{
	(mainStackWidget->getStackLayout())->setCurrentIndex(1);
}

/*================================================================
*  函 数 名： MainWindow::flux
*
*  功能描述： 切换到流量显示选项卡
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::flux()
{
	FluxViewWidget *fluxWidget;

	(mainStackWidget->getStackLayout())->setCurrentIndex(2);

	if (strNowSelectIP.size() > 0) {
		fluxWidget = (FluxViewWidget *)mainStackWidget->getStackLayout()->widget(2);
		fluxWidget->showFluxInfoByIP(strNowSelectIP);

		qDebug() << "切换标签自动触发显示流量信息命令" << "\r\n";
	}
}

/*================================================================
*  函 数 名： MainWindow::process
*
*  功能描述： 切换到进程信息选项卡
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::process()
{
	(mainStackWidget->getStackLayout())->setCurrentIndex(3);
	if (strNowSelectIP.size() > 0) {
		(mainStackWidget->getStackLayout())->setCurrentIndex(3);
		clientSocket->sendControlCommand(GetProcessInfo);
		qDebug() << "切换标签自动触发发送获取进程信息命令" << "\r\n";
	}
}

/*================================================================
*  函 数 名： MainWindow::cmd
*
*  功能描述： 切换到远程命令选项卡
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::cmd()
{
	(mainStackWidget->getStackLayout())->setCurrentIndex(4);
}

/*================================================================
*  函 数 名： MainWindow::info
*
*  功能描述： 切换到信息查看选项卡
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::info()
{
	(mainStackWidget->getStackLayout())->setCurrentIndex(5);
	if (strNowSelectIP.size() > 0) {
		clientSocket->sendControlCommand(GethardInfo);
		qDebug() << "切换标签自动触发发送获取硬件信息命令" << "\r\n";
	}
}

void MainWindow::readSettings()
{
	QSettings settings("falcon.ini", QSettings::IniFormat);

	settingInfo.strServerIP = settings.value("ServerIP", true).toString();
	settingInfo.serverPort = settings.value("ServerPort", true).toInt();
	settingInfo.isReConn = settings.value("AutoConn", true).toBool();
	settingInfo.isShowTrayInfo = settings.value("ShowTrayInfo", true).toBool();
	settingInfo.isPlaySound = settings.value("PlaySound", true).toBool();
	settingInfo.isSendPhoneMessage = settings.value("SendPhoneMessage", true).toBool();
	settingInfo.strFetionNumber = settings.value("FetionNumber", true).toString();
	settingInfo.strFetionPass = EncryptOrDecipher(settings.value("FetionPass", true).toString());
	settingInfo.strLanguage = settings.value("Language", true).toString();
	settingInfo.upSpeedLimit = settings.value("upSpeedLimit", true).toInt();
	settingInfo.downSpeedLimit = settings.value("downloadSpeed", true).toInt();
	settingInfo.allowTimes = settings.value("allowTimes", true).toInt();
}

void MainWindow::writeSettings()
{
	QSettings settings("falcon.ini", QSettings::IniFormat);

	settings.setValue("ServerIP", settingInfo.strServerIP);
	settings.setValue("ServerPort", settingInfo.serverPort);
	settings.setValue("AutoConn", settingInfo.isReConn);
	settings.setValue("ShowTrayInfo", settingInfo.isShowTrayInfo);
	settings.setValue("PlaySound", settingInfo.isPlaySound);
	settings.setValue("SendPhoneMessage", settingInfo.isSendPhoneMessage);
	settings.setValue("FetionNumber", settingInfo.strFetionNumber);
	settings.setValue("FetionPass", EncryptOrDecipher(settingInfo.strFetionPass));
	settings.setValue("Language", settingInfo.strLanguage);
	settings.setValue("upSpeedLimit", settingInfo.upSpeedLimit);
	settings.setValue("downloadSpeed", settingInfo.downSpeedLimit);
	settings.setValue("allowTimes", settingInfo.allowTimes);
}

/*================================================================
*  函 数 名： MainWindow::about
*
*  功能描述： 显示关于对话框
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::about()
{
	QMessageBox::about(this, tr("关于 Falcan"),
			tr("<h2>Falcon 企业网络监控系统</h2>"
				"<p>Safe  Stable  Sharp (安全，稳定，敏捷)"
				"<p><h3>一个基于 Windows NT 平台的局域网络管理控制软件，GUI By Qt 4.X 。</h3>"
				"<p>严正声明：本程序仅用于技术研究，任何集体和个人使用该"
				"程序造成的任何违法后果由使用者个人承担！"
				"<p>Copyright (C) 2013 Hunters "));
}

/*================================================================
*  函 数 名： MainWindow::createMainWeiget
*
*  功能描述： 创建主界面控件
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createMainWeiget()
{
	httpSocket = NULL;

	currLinkLabel = new QLabel(tr("正向连接服务端"));
	currPortLabel = new QLabel(tr("  连接端口"));
	
	currLinkIPEdit = new QLineEdit();

	QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
	currLinkIPEdit->setValidator(new QRegExpValidator(regExp, this));
	currLinkIPEdit->setText(settingInfo.strServerIP);
	currLinkIPEdit->setMaxLength(15);
	currLinkIPEdit->setToolTip(tr("您要手动连接的服务器IP地址"));

	currPortSpin = new QSpinBox();
	currPortSpin->setRange(1, 65535);
	currPortSpin->setValue(settingInfo.serverPort);
	currPortSpin->setToolTip(tr("您要手动连接的服务器端口"));

	linkButton = new QPushButton(tr("连接服务端"));
	connect(linkButton, SIGNAL(clicked()), this, SLOT(connectServerBtn()));

	linkButton->setEnabled(false);

	topLayout = new QHBoxLayout();
	topLayout->addWidget(currLinkLabel);
	topLayout->addWidget(currLinkIPEdit);
	topLayout->addWidget(currPortLabel);
	topLayout->addWidget(currPortSpin);
	topLayout->addWidget(linkButton);
	topLayout->addStretch();
	
	mainStackWidget = new MainStackWidget(this);
	linkTreeWidget = new LinkTreeWidget(this);

	bottomSplitter = new QSplitter(Qt::Horizontal);
	bottomSplitter->addWidget(linkTreeWidget);
	bottomSplitter->addWidget(mainStackWidget);
	bottomSplitter->setStretchFactor(1, 1); 

	mainLayout = new QVBoxLayout();
	mainLayout->addLayout(topLayout);
	mainLayout->addWidget(bottomSplitter);

	mainWigget = new QWidget();
	mainWigget->setLayout(mainLayout);

	this->setCentralWidget(mainWigget);

	settingDialog = new SettingDialog();
	settingDialog->setWindowIcon(QIcon(":/res/ico/main.ico"));
	settingDialog->setDialogSetting(settingInfo);
}

/*================================================================
*  函 数 名： MainWindow::createActions
*
*  功能描述： 创建主界面响应
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createActions()
{
	auto_connAction = new QAction(tr("搜索服务端"), this);
	auto_connAction->setIcon(QIcon(":/res/images/auto_conn.png"));
	auto_connAction->setStatusTip(tr("自动搜索服务器"));
	connect(auto_connAction, SIGNAL(triggered()), this, SLOT(startUdpFoundServer()));

	settingAction = new QAction(tr("程序设置"), this);
	settingAction->setIcon(QIcon(":/res/images/setting.png"));
	settingAction->setStatusTip(tr("程序设置"));
	connect(settingAction, SIGNAL(triggered()), this, SLOT(softSetting()));

	exitAction = new QAction(tr("退出程序"), this);
	exitAction->setIcon(QIcon(":/res/images/exit.png"));
	exitAction->setStatusTip(tr("退出程序"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(closeWindow()));

	aboutAction = new QAction(tr("关于本程序"), this);
	aboutAction->setIcon(QIcon(":/res/images/about.png"));
	aboutAction->setStatusTip(tr("关于本程序"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	connectAction = new QAction(tr("连接总览"), this);
	connectAction->setIcon(QIcon(":/res/images/connect.png"));
	connectAction->setStatusTip(tr("客户端连接总览"));
	connect(connectAction, SIGNAL(triggered()), this, SLOT(connectInfo()));

	screenShotAction = new QAction(tr("屏幕捕获"), this);
	screenShotAction->setIcon(QIcon(":/res/images/screenshot.png"));
	screenShotAction->setStatusTip(tr("捕获被控端的屏幕"));
	connect(screenShotAction, SIGNAL(triggered()), this, SLOT(screenShot()));
	
	fluxAction = new QAction(tr("流量监控"), this);
	fluxAction->setIcon(QIcon(":/res/images/flux.png"));
	fluxAction->setStatusTip(tr("客户端流量监控"));
	connect(fluxAction, SIGNAL(triggered()), this, SLOT(flux()));
	
	processAction = new QAction(tr("进程管理"), this);
	processAction->setIcon(QIcon(":/res/images/process.png"));
	processAction->setStatusTip(tr("客户端进程管理"));
	connect(processAction, SIGNAL(triggered()), this, SLOT(process()));
	
	cmdAction = new QAction(tr("远程CMD"), this);
	cmdAction->setIcon(QIcon(":/res/images/cmd.png"));
	cmdAction->setStatusTip(tr("远程执行CMD命令"));
	connect(cmdAction, SIGNAL(triggered()), this, SLOT(cmd()));

	infoAction = new QAction(tr("信息查看"), this);
	infoAction->setIcon(QIcon(":/res/images/information.png"));
	infoAction->setStatusTip(tr("查看客户端信息和处理日志"));
	connect(infoAction, SIGNAL(triggered()), this, SLOT(info()));
}

/*================================================================
*  函 数 名： MainWindow::createMenus
*
*  功能描述： 创建主界面菜单
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createMenus()
{
	fileMenu = this->menuBar()->addMenu(tr("文件"));
	fileMenu->addAction(auto_connAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	setsMenu = this->menuBar()->addMenu(tr("设置"));
	setsMenu->addAction(settingAction);

	helpMenu = this->menuBar()->addMenu(tr("帮助"));
	helpMenu->addAction(aboutAction);
}

/*================================================================
*  函 数 名： MainWindow::createToolBars
*
*  功能描述： 创建主界面工具栏
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createToolBars()
{
	mainToolBar = addToolBar(tr("工具栏"));
	mainToolBar->setMovable(false);
	mainToolBar->setIconSize(QSize(60, 60));
	mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	mainToolBar->setStyleSheet("QToolBar{ background:url(:/res/images/ToolBack.png); }");

	mainToolBar->addSeparator();
	mainToolBar->addAction(connectAction);
	mainToolBar->addSeparator();
	//mainToolBar->addAction(screenShotAction);
	//mainToolBar->addSeparator();
	mainToolBar->addAction(fluxAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(processAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(cmdAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(infoAction);
	mainToolBar->addSeparator();
	mainToolBar->addAction(aboutAction);
	mainToolBar->addSeparator();
}

/*================================================================
*  函 数 名： MainWindow::createStatusBar
*
*  功能描述： 创建主界面状态栏
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createStatusBar()
{
	statusLabel = new QLabel();
	statusLabel->setAlignment(Qt::AlignHCenter);
	statusLabel->setMinimumSize(statusLabel->sizeHint());

	this->statusBar()->addWidget(statusLabel);
}

/*================================================================
*  函 数 名： MainWindow::createSystemTrayIcon
*
*  功能描述： 创建托盘图标
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::createSystemTrayIcon()
{
	// 检测系统是否支持托盘图标
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		systemTrayIcon = NULL;
		return;
	}

	trayMenu = new QMenu();
	trayMenu->addAction(exitAction);

	systemTrayIcon = new QSystemTrayIcon(this);
	systemTrayIcon->setIcon(QIcon(":/res/ico/main.ico"));
	systemTrayIcon->setToolTip(tr("Falcon 企业网络监控系统"));
	systemTrayIcon->setContextMenu(trayMenu);
	systemTrayIcon->show();
	connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
				this, SLOT(showWindow(QSystemTrayIcon::ActivationReason)));
	isTrayExit = false;
}

/*================================================================
*  函 数 名： MainWindow::showWindow
*
*  功能描述： 托盘图标消息响应
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::showWindow(QSystemTrayIcon::ActivationReason type)
{
	// 仅在托盘图标单击时候显示主界面，右击显示菜单
	if (type == QSystemTrayIcon::Trigger || type == QSystemTrayIcon::DoubleClick) {
		this->showNormal();
	}
}

/*================================================================
*  函 数 名： void MainWindow::closeWindow
*
*  功能描述： 托盘菜单退出按钮消息响应
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::closeWindow()
{
	isTrayExit = true;
	this->close();
}

/*================================================================
*  函 数 名： MainWindow::sleep
*
*  功能描述： sleep函数实现
*
*  参    数： unsigned int msec 毫秒
*			  
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::sleep(unsigned int msec)
{
	QTime dieTime = QTime::currentTime().addMSecs(msec);

	while (QTime::currentTime() < dieTime) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}

/*================================================================
*  函 数 名： MainWindow::softSetting
*
*  功能描述： 软件设置对话框
*
*  参    数： 无
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::softSetting()
{
	QByteArray strSend = FalconXml::CreaterControlCommandXml(GetBlackList, "");

	clientSocket->write(strSend);
	clientSocket->flush();

	if (settingDialog->exec() == QDialog::Accepted) {
		settingDialog->getDialogSetting(settingInfo);
		settingDialog->getBlackList(vectBlackList);
		QByteArray strUpdateBlackList = FalconXml::CreateBlackListInfoXml(vectBlackList);
		clientSocket->write(strUpdateBlackList);
		clientSocket->flush();
	}
}

/*================================================================
*  函 数 名： MainWindow::tryConnServer
*
*  功能描述： 尝试连接服务器
*
*  参    数： 无
*			  
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::tryConnServer()
{
	waitServerSocket.bind(5162, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	connect(&waitServerSocket, SIGNAL(readyRead()), this, SLOT(recvUdpMessage()));

	connect(&foundServerTimer, SIGNAL(timeout()), this, SLOT(sendUdpFoundInfo()));

	clientSocket = new ControlSocket(this);

	qDebug() << "尝试连接服务器" << settingInfo.strServerIP << ":" << settingInfo.serverPort << "\r\n";

	clientSocket->connectToHost(settingInfo.strServerIP, settingInfo.serverPort);
	if (clientSocket->waitForConnected(1000)) {
	} else if (settingInfo.isReConn) {
		startUdpFoundServer();
	}
}

/*================================================================
*  函 数 名： MainWindow::sendPhoneMessage
*
*  功能描述： 管理员手机发送短信
*
*  参    数： QString strMsg
*			  
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::sendPhoneMessage(QString strMsg)
{
	if (!settingInfo.isSendPhoneMessage) {
		return;
	}
	if (!httpSocket) {
		httpSocket = new QHttp();
	}
	QString strSend = QString("/fetion/falcon.php?phone=%1&pass=%2&sendto=%3&msg=%4")\
			.arg(settingInfo.strFetionNumber).arg(settingInfo.strFetionPass)
			.arg("18710707494").arg(QString(QUrl::toPercentEncoding(strMsg)));

 	httpSocket->setHost("fedora.sinaapp.com");
 	httpSocket->get(strSend);
 	httpSocket->close();

 	qDebug() << "发送短信" << strMsg << "\r\n";
}

/*================================================================
*  函 数 名： MainWindow::connectServerBtn
*
*  功能描述： 手动连接服务器
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::connectServerBtn()
{
	stopUdpFoundServer();
	linkTreeWidget->clear();
	((MainListWidget *)(mainStackWidget->getStackLayout())->widget(0))->clear();

	QString strServerIP = currLinkIPEdit->text();
	int serverPort = currPortSpin->value();

	qDebug() << "尝试手动连接服务器" << strServerIP << ":" << serverPort << "\r\n";

	showTrayInfo(QString(tr("尝试手动连接服务器 ")) + strServerIP + "...");

	clientSocket->connectToHost(strServerIP, serverPort);
	if (clientSocket->waitForConnected(1000)) {
		settingInfo.strServerIP = strServerIP;
		settingInfo.serverPort = serverPort;
	} else if (settingInfo.isReConn) {
		startUdpFoundServer();
	}
}

/*================================================================
*  函 数 名： MainWindow::showTrayInfo
*
*  功能描述： 托盘弹出气泡图标
*
*  参    数： QString —— 显示文字内容
* 			  unsigned int —— 显示时间
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::showTrayInfo(QString strInfo, unsigned int msec)
{
	if (systemTrayIcon != NULL && settingInfo.isShowTrayInfo) {
		systemTrayIcon->showMessage(tr("Falcon 企业网络监控系统"),
			 strInfo, QSystemTrayIcon::Information, msec);
	}
}

/*================================================================
*  函 数 名： MainWindow::setStatusLabel
*
*  功能描述： 设置状态栏图标
*
*  参    数： QString —— 设置的文字
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setStatusLabel(QString strLabel)
{
	statusLabel->setText(strLabel);
}

/*================================================================
*  函 数 名： MainWindow::EncryptOrDecipher
*
*  功能描述： 字符串加密函数
*
*  参    数： QString —— 待加密字符串
*			  
*  返 回 值： QString —— 已加密字符串
*
*  作    者：刘欢 2013/8
================================================================*/
QString MainWindow::EncryptOrDecipher(QString strPass)
{
	QByteArray bs = strPass.toAscii();
  	
	for (int i = 0; i < bs.size(); ++i) {
		bs[i] = bs[i] ^ 0x8;
	}

	return QString(bs);
}

/*================================================================
*  函 数 名： MainWindow::setClientList
*
*  功能描述： 根据收取的分组信息设置控件显示
*
*  参    数： QVector<GroupWithClient> &vectClient —— 分组信息
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientList(QVector<GroupWithClient> &vectClient)
{
	QVector<GroupWithClient>::iterator index = vectClient.begin();
	for (; index != vectClient.end(); ++index) {
		linkTreeWidget->addGroupItem(index->strGroup);

		QVector<QString>::iterator index_client = (index->vectClient).begin();
		for (; index_client != (index->vectClient).end(); ++index_client) {
			linkTreeWidget->addComputerItem(index->strGroup, *index_client, true);
			MainListWidget *listWidget = (MainListWidget *)mainStackWidget->getStackLayout()->widget(0);
			listWidget->addComputerItem(*index_client, true);
		}
	}

	//结点全部展开
	linkTreeWidget->expandAll();
}

/*================================================================
*  函 数 名： MainWindow::setClientoffList
*
*  功能描述： 设置客户端离线后控件显示
*
*  参    数： QVector<GroupWithClient> &vectClient —— 分组信息
*			  
*  返 回 值：无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientOffList(QVector<GroupWithClient> &vectClient)
{
	QVector<GroupWithClient>::iterator index = vectClient.begin();
	for (; index != vectClient.end(); ++index) {
		QVector<QString>::iterator index_client = (index->vectClient).begin();
		for (; index_client != (index->vectClient).end(); ++index_client) {
			linkTreeWidget->removeComputerItem(*index_client);
			MainListWidget *listWidget = (MainListWidget *)mainStackWidget->getStackLayout()->widget(0);
			listWidget->removeComputerItem(*index_client);
		}
	}

	//结点全部展开
	linkTreeWidget->expandAll();
}

/*================================================================
*  函 数 名： MainWindow::setClientScreenInfo
*
*  功能描述： 根据收取的屏幕信息设置控件显示
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientScreenInfo()
{

}

/*================================================================
*  函 数 名： MainWindow::setClientFluxInfo
*
*  功能描述： 根据收取的流量信息设置控件显示
*
*  参    数： ClientFluxInfo &clientFluxInfo —— 收取的流量信息
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientFluxInfo(ClientFluxInfo &clientFluxInfo)
{
	FluxViewWidget *fluxWidget;
	fluxWidget = (FluxViewWidget *)mainStackWidget->getStackLayout()->widget(2);

	fluxWidget->addOnceSpeedByIP(clientFluxInfo.strClientIP,
					 		clientFluxInfo.uploadSpeed, clientFluxInfo.downloadSpeed);
}

/*================================================================
*  函 数 名： MainWindow::setClientProcessInfo
*
*  功能描述： 根据收取的进程信息设置控件显示
*
*  参    数： QVector<ClientProcessInfo> &vectProcessInfo —— 收取的进程信息
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientProcessInfo(QVector<ClientProcessInfo> &vectProcessInfo)
{
	LinkProcessWidget *linkProcessWidget = (LinkProcessWidget *)mainStackWidget->getStackLayout()->widget(3);
	ProcessTreeView * processView = linkProcessWidget->getProcessView();

	processView->rebuildInfo();
	QVector<ClientProcessInfo>::iterator index = vectProcessInfo.begin();
	for (;index != vectProcessInfo.end(); ++index) {
		processView->addOneProcessItem(index->process, index->pid, index->mem, index->path, index->description);
	}
}

/*================================================================
*  函 数 名： MainWindow::setClientCmdInfo
*
*  功能描述： 根据收取的cmd信息设置控件显示
*
*  参    数： QString &strCmdInfo —— 收取的cmd信息
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientCmdInfo(QString &strCmdInfo)
{
	LinkCmdWidget *cmdWidget = (LinkCmdWidget *)mainStackWidget->getStackLayout()->widget(4);
	cmdWidget->cmdViewAddInfo(strCmdInfo);
}

/*================================================================
*  函 数 名： MainWindow::setClientHardInfo
*
*  功能描述： 根据收取的硬件信息设置控件显示
*
*  参    数： QString &strHardInfo —— 收取的硬件信息
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientHardInfo(QString &strHardInfo)
{
	InfoWidget *infoWidget = (InfoWidget *)mainStackWidget->getStackLayout()->widget(5);
	infoWidget->infoView->clear();
	infoWidget->infoView->append(strHardInfo);
}

/*================================================================
*  函 数 名： MainWindow::setClientLogInfo
*
*  功能描述： 根据日志信息设置控件显示
*
*  参    数： QString &strLogInfo —— 日志信息
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setClientLogInfo(QString &strLogInfo)
{
	InfoWidget *infoWidget = (InfoWidget *)mainStackWidget->getStackLayout()->widget(5);
	infoWidget->infoView->clear();
	infoWidget->infoView->append(strLogInfo);
	infoWidget->infoView->append(QString("$> "));
}

/*================================================================
*  函 数 名： MainWindow::startUdpFoundServer
*
*  功能描述： 开启UDP广播寻找服务端
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::startUdpFoundServer()
{
	if (!foundServerTimer.isActive()) {
		foundServerTimer.start(3000);
		showTrayInfo(tr("自动开始在局域网里寻找服务器..."));
	}
}

/*================================================================
*  函 数 名： MainWindow::stopUdpFoundServer
*
*  功能描述： 关闭UDP广播寻找服务端
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::stopUdpFoundServer()
{
	foundServerTimer.stop();
}

/*================================================================
*  函 数 名： MainWindow::recvUdpMessage
*
*  功能描述： 接收UDP信息
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::recvUdpMessage()
{
	char szRecv[1024];
	QHostAddress ipAddr;

	stopUdpFoundServer();

	waitServerSocket.readDatagram(szRecv, sizeof(szRecv), &ipAddr);

	QByteArray recvData(szRecv);

	if (FalconXml::getXmlInfoType(recvData) == ServerResponse) {
		linkTreeWidget->clear();
		((MainListWidget *)(mainStackWidget->getStackLayout())->widget(0))->clear();
		showTrayInfo(QString(tr("自动连接服务器 ")) + ipAddr.toString() + " 中...");
		clientSocket->connectToHost(ipAddr.toString(), 8086);
		settingInfo.strServerIP = ipAddr.toString();
	} else {
		startUdpFoundServer();
	}
}

/*================================================================
*  函 数 名： MainWindow::sendUdpFoundInfo
*
*  功能描述： 发送UDP广播的函数
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::sendUdpFoundInfo()
{
	QByteArray sendMsg = FalconXml::CreateUdpFoundServerXml();
	foundServerSocket.writeDatagram(sendMsg, QHostAddress::Broadcast, 5161);
}

/*================================================================
*  函 数 名： MainWindow::setBlackList
*
*  功能描述： 设置对话框里的黑名单信息
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void MainWindow::setBlackList()
{
	settingDialog->setBlackList(vectBlackList);
}
