#include "SerialPortAssistant.h"
#include <algorithm>


void SerialPortAssistant::SerialPort_SendAear_Init()
{
	SerialPort_SendAear = new QPlainTextEdit(this);                              // 创建发送数据的文本框对象
	SerialPort_SendAear->setFixedSize(800, 100);                                  // 设置文本框固定大小为 800x100 像素
	SerialPort_SendAear->move(30, 500);                                           // 将文本框移动到窗口坐标 (30, 500) 处

	SerialPort_Send = new QPushButton(QString::fromLocal8Bit("发送"), this); // 创建“发送数据”按钮，并处理中文编码
	SerialPort_Send->setFixedSize(150, 50);                                  // 设置文本框固定大小为 800x100 像素
	SerialPort_Send->move(500, 630);
	SerialPort_Send->setDisabled(true);                                     // 初始状态下禁用发送按钮，等待串口连接后启用

	connect(SerialPort_Send, &QPushButton::clicked, [&]() {               // 绑定按钮点击信号到 Lambda 表达式
		// 发送数据的代码
		QString dataToSend = SerialPort_SendAear->toPlainText(); // 获取发送区的文本内容
		if (SerialPort_SendMode->currentText() == "HEX")
		{
			QByteArray arrayToSend;
			for (int i = 0; i < dataToSend.size(); ++i)
			{
				if (dataToSend[i] == ' ')continue;
				int num = dataToSend.mid(i, 2).toInt(nullptr, 16);
				++i;
				arrayToSend.append(num);
			}
			serialPort->write(arrayToSend);
		}
		else
		{
			serialPort->write(dataToSend.toLocal8Bit().data());
		}
		});

	QPushButton* Clear_SendAear = new QPushButton(QString::fromLocal8Bit("清空发送区"), this); // 创建“清空发送区”按钮，并处理中文编码
	Clear_SendAear->setFixedSize(150, 50);                                       // 设置按钮固定大小
	Clear_SendAear->move(680, 630);                                              // 设置按钮在界面上的位置 (680, 630)
	connect(Clear_SendAear, &QPushButton::clicked, [&]() {                       // 绑定按钮点击信号到 Lambda 表达式
		SerialPort_SendAear->clear();                                            // 调用 clear() 函数清空发送区的文本内容
		});
}

void SerialPortAssistant::SerialPort_ReceiveAear_Init()
{
	SerialPort_ReceiveAear = new QPlainTextEdit(this);                              // 创建接收数据的文本框对象
	SerialPort_ReceiveAear->setFixedSize(800, 400);                                  // 设置文本框固定大小为 800x400 像素
	SerialPort_ReceiveAear->move(30, 20);                                           // 将文本框移动到窗口坐标 (30, 20) 处
	SerialPort_ReceiveAear->setReadOnly(true);                                      // 设置为只读模式，防止用户手动修改接收到的通过串口来的数据

	QPushButton* Clear_ReceiveAear = new QPushButton(QString::fromLocal8Bit("清空接收区"), this); // 创建“清空接收区”按钮，并处理中文编码
	Clear_ReceiveAear->setFixedSize(150, 50);                                       // 设置按钮固定大小
	Clear_ReceiveAear->move(680, 430);                                              // 设置按钮在界面上的位置
	connect(Clear_ReceiveAear, &QPushButton::clicked, [&]() {                       // 绑定按钮点击信号到 Lambda 表达式
		SerialPort_ReceiveAear->clear();                                            // 调用 clear() 函数清空接收区的文本内容
		});
}

void SerialPortAssistant::SerialPort_Control_Init()
{
	this->SerialPort_Number = new QComboBox(this);
	this->SerialPort_BaudRate = new QComboBox(this);
	this->SerialPort_DataBits = new QComboBox(this);
	this->SerialPort_StopBits = new QComboBox(this);
	this->SerialPort_CheckBits = new QComboBox(this);
	this->SerialPort_SendMode = new QComboBox(this);
	this->SerialPort_ReceiveMode = new QComboBox(this);

	QLabel* Label_SerialPort_Number = new QLabel(QString::fromLocal8Bit("串口号"), this);

	QLabel* Label_SerialPort_BaudRate = new QLabel(QString::fromLocal8Bit("波特率"), this);
	this->SerialPort_BaudRate->addItem("4800");
	this->SerialPort_BaudRate->addItem("9600");
	this->SerialPort_BaudRate->addItem("115200");

	QLabel* Label_SerialPort_DataBits = new QLabel(QString::fromLocal8Bit("数据位"), this);
	this->SerialPort_DataBits->addItem("8");

	QLabel* Label_SerialPort_StopBits = new QLabel(QString::fromLocal8Bit("停止位"), this);
	this->SerialPort_StopBits->addItem("1");
	this->SerialPort_StopBits->addItem("1.5");
	this->SerialPort_StopBits->addItem("2");

	QLabel* Label_SerialPort_CheckBits = new QLabel(QString::fromLocal8Bit("校验位"), this);
	this->SerialPort_CheckBits->addItem(QString::fromLocal8Bit("无"));
	this->SerialPort_CheckBits->addItem(QString::fromLocal8Bit("奇校验"));
	this->SerialPort_CheckBits->addItem(QString::fromLocal8Bit("偶校验"));

	QLabel* Label_SerialPort_SendMode = new QLabel(QString::fromLocal8Bit("发送格式"), this);
	this->SerialPort_SendMode->addItem("HEX");
	this->SerialPort_SendMode->addItem(QString::fromLocal8Bit("文本"));

	QLabel* SerialPort_ReceiveMode = new QLabel(QString::fromLocal8Bit("接收格式"), this);
	this->SerialPort_ReceiveMode->addItem("HEX");
	this->SerialPort_ReceiveMode->addItem(QString::fromLocal8Bit("文本"));

	QVector<QComboBox*>Control;
	Control.push_back(this->SerialPort_Number);
	Control.push_back(this->SerialPort_BaudRate);
	Control.push_back(this->SerialPort_DataBits);
	Control.push_back(this->SerialPort_StopBits);
	Control.push_back(this->SerialPort_CheckBits);
	Control.push_back(this->SerialPort_SendMode);
	Control.push_back(this->SerialPort_ReceiveMode);

	QVector<QLabel*>Control_Labels;
	Control_Labels.push_back(Label_SerialPort_Number);
	Control_Labels.push_back(Label_SerialPort_BaudRate);
	Control_Labels.push_back(Label_SerialPort_DataBits);
	Control_Labels.push_back(Label_SerialPort_StopBits);
	Control_Labels.push_back(Label_SerialPort_CheckBits);
	Control_Labels.push_back(Label_SerialPort_SendMode);
	Control_Labels.push_back(SerialPort_ReceiveMode);

	for (int i = 0; i < Control.size(); ++i)
	{
		Control[i]->setFixedSize(200, 50);
		Control[i]->move(850, 20 + i * 80);
		Control_Labels[i]->move(1080, 25 + i * 80);
	}





}

void SerialPortAssistant::SerialPort_Connection_Init()
{
	SerialPort_Connect = new QPushButton(QString::fromLocal8Bit("连接串口"), this);
	SerialPort_Disonnect = new QPushButton(QString::fromLocal8Bit("断开串口"), this);
	SerialPort_Connect->setFixedSize(150,50);
	SerialPort_Disonnect->setFixedSize(150, 50);
	SerialPort_Connect->move(850, 600);
	SerialPort_Disonnect->move(1000, 600);

	SerialPort_Disonnect->setDisabled(true);

	connect(SerialPort_Connect, &QPushButton::clicked, [&]() {
		// 连接串口的代码
		if (SerialPort_Number->currentText() != "")
		{
			SerialPort_Send->setDisabled(false);
			SerialPort_Connect->setDisabled(true);
			SerialPort_Disonnect->setDisabled(false);
			SerialPort_Connection();
		}
		else
		{
			QMessageBox::critical(this, QString::fromLocal8Bit("串口打开失败"), QString::fromLocal8Bit("请确认串口是否连接正确"));
		}
		
		
		});
	connect(SerialPort_Disonnect, &QPushButton::clicked, [&](){
		// 断开串口的代码
		SerialPort_Send->setDisabled(true);
		SerialPort_Connect->setDisabled(false);
		SerialPort_Disonnect->setDisabled(true);
		serialPort->close();
		});

}

void SerialPortAssistant::SerialPort_Connection()
{
	QSerialPort::BaudRate Baud;
	QSerialPort::DataBits Data;
	QSerialPort::StopBits Stop;
	QSerialPort::Parity Check;
	 
	QString port = SerialPort_Number->currentText();
	QString baud = SerialPort_BaudRate->currentText();
	QString data = SerialPort_DataBits->currentText();
	QString stop = SerialPort_StopBits->currentText();
	QString check = SerialPort_CheckBits->currentText();

	if (baud == "4800") Baud = QSerialPort::Baud4800;
	else if (baud == "9600") Baud = QSerialPort::Baud9600;
	else if (baud == "115200") Baud = QSerialPort::Baud115200;

	if (data == "8") Data = QSerialPort::Data8;

	if (stop == "1") Stop = QSerialPort::OneStop;
	else if (stop == "1.5") Stop = QSerialPort::OneAndHalfStop;
	else if (stop == "2") Stop = QSerialPort::TwoStop;

	if (check == QString::fromLocal8Bit("无")) Check = QSerialPort::NoParity;
	else if (check == QString::fromLocal8Bit("奇校验")) Check = QSerialPort::OddParity;
	else if (check == QString::fromLocal8Bit("偶校验")) Check = QSerialPort::EvenParity;

	serialPort = new QSerialPort(this);
	serialPort->setBaudRate(Baud);
	serialPort->setDataBits(Data);
	serialPort->setStopBits(Stop);
	serialPort->setParity(Check);
	serialPort->setPortName(port); 
	 
	if (serialPort->open(QSerialPort::ReadWrite))
	{
		connect(serialPort, &QSerialPort::readyRead, [&]() {
			auto data = serialPort->readAll();
			if (SerialPort_ReceiveMode->currentText() == "HEX")
			{
				QString hex = data.toHex(' ');
				SerialPort_ReceiveAear->appendPlainText(hex);
			}
			else
			{
				QString str = QString(data);
				SerialPort_ReceiveAear->appendPlainText(data);
			}
			});
	}
	else
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("串口打开失败"), QString::fromLocal8Bit("请确认串口是否连接正确"));
	}

}


void SerialPortAssistant::timerEvent(QTimerEvent* event)
{
	QVector<QString> currentPorts;
	for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
	{
		currentPorts.push_back(info.portName());
	}

	// 使用自定义 Lambda 表达式进行排序
	std::sort(currentPorts.begin(), currentPorts.end(), [](const QString& s1, const QString& s2) {
		// 提取 "COM" 后面的数字部分
		// mid(3) 假设名字都是 "COM" 开头。如果可能是其它名字，需要更严谨的处理。
		int n1 = s1.mid(3).toInt();
		int n2 = s2.mid(3).toInt();

		// 如果名字不是以 COM 开头或者转换失败，退回到默认字符串比较
		if (s1.startsWith("COM") && s2.startsWith("COM") && n1 > 0 && n2 > 0) {
			return n1 < n2;
		}
		return s1 < s2;
		});

	// ... (后续更新 UI 的逻辑)
	if (currentPorts != portList)
	{
		this->SerialPort_Number->clear();
		this->portList = currentPorts;
		for (auto& a : portList) this->SerialPort_Number->addItem(a);
	}
}

SerialPortAssistant::SerialPortAssistant(QWidget *parent): QMainWindow(parent)
{
	this->setWindowTitle(QString::fromLocal8Bit("陈娇的串口助手"));
	this->setFixedSize(1200,750);

	SerialPort_ReceiveAear_Init();
	SerialPort_SendAear_Init();
	SerialPort_Control_Init();
	SerialPort_Connection_Init();
	this->startTimer(1000);
}

SerialPortAssistant::~SerialPortAssistant()
{}

