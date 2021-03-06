// =====================================================================================
// 
//       Filename:  ScreenViewWidget.cpp
//
//    Description:  屏幕监控显示
//
//        Version:  1.0
//        Created:  2013年08月20日 11时08分08秒
//       Revision:  none
//       Compiler:  cl
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#include <QtGui>

#include "ScreenViewWidget.h"

ScreenViewWidget::ScreenViewWidget(MainWindow *mainWindow)
{
	qDebug() << "ScreenViewWidget 控件初始化\r\n";
	this->mainWindow = mainWindow;
	setStyleSheet("QGroupBox {border-width:1px; border-style:solid; border-color:gray;}");

	label = new QLabel();
	originalPixmap = new QPixmap(":/res/images/desktop.png");
	label->setPixmap(*originalPixmap);
	screenButton = new QPushButton(tr("捕获屏幕"), this);
	connect(screenButton, SIGNAL(clicked()), this, SLOT(screenShotNative()));
	saveButton = new QPushButton(tr("保存图片"), this);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveScreenShot()));

	hBoxLayout = new QHBoxLayout();
	vBoxLayout = new QVBoxLayout();
	
	hBoxLayout->addStretch();
	hBoxLayout->addWidget(screenButton);
	hBoxLayout->addStretch();
	hBoxLayout->addWidget(saveButton);
	hBoxLayout->addStretch();

	vBoxLayout->addWidget(label);
	vBoxLayout->addLayout(hBoxLayout);

	setLayout(vBoxLayout);
}

ScreenViewWidget::~ScreenViewWidget()
{

}

/*================================================================
*  函 数 名： ScreenViewWidget::screenShotNative
*
*  功能描述： 捕获本地机器的桌面截图
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void ScreenViewWidget::screenShotNative()
{
	*originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
	label->setPixmap(originalPixmap->scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/*================================================================
*  函 数 名： creenViewWidget::saveScreenShot
*
*  功能描述： 保存捕获的截图
*
*  参    数： 无
*			  
*  返 回 值： 无
*
*  作    者：刘欢 2013/8
================================================================*/
void ScreenViewWidget::saveScreenShot()
{
	QString format = "png";
	QString initialPath = QDir::currentPath() + tr("/untitled.") + format;

	QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"),
													initialPath,
													tr("%1 文件 (*.%2);;所有文件 (*)")
													.arg(format.toUpper())
													.arg(format));
	if (!fileName.isEmpty()) {
		originalPixmap->save(fileName, format.toAscii());
	}
}
