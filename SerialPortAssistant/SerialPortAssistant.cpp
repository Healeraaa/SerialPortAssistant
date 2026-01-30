#include "SerialPortAssistant.h"

SerialPortAssistant::SerialPortAssistant(QWidget* parent) : QMainWindow(parent)
{
    this->setWindowTitle(QString::fromLocal8Bit("串口示波器 - 最终全功能版"));
    this->setMinimumSize(1250, 950);

    serialPort = new QSerialPort(this);
    initUI();
    setupConnections();
    this->startTimer(1000); // 串口自动巡检
    updatePortList();
}

void SerialPortAssistant::initUI()
{
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // --- 左侧：数据显示与波形 ---
    QVBoxLayout* leftLayout = new QVBoxLayout();

    SerialPort_ReceiveAear = new QPlainTextEdit();
    SerialPort_ReceiveAear->setReadOnly(true);
    leftLayout->addWidget(new QLabel(QString::fromLocal8Bit("接收日志:")), 0);
    leftLayout->addWidget(SerialPort_ReceiveAear, 2);

    series = new QLineSeries();
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString::fromLocal8Bit("实时波形分析"));
    chart->legend()->hide();

    axisX = new QValueAxis(); axisX->setRange(0, 100); axisX->setLabelFormat("%d");
    axisY = new QValueAxis(); axisY->setRange(0, 100);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX); series->attachAxis(axisY);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    leftLayout->addWidget(chartView, 5);

    ScrollBar_X = new QScrollBar(Qt::Horizontal);
    ScrollBar_X->setEnabled(false);
    leftLayout->addWidget(ScrollBar_X);

    SerialPort_SendAear = new QPlainTextEdit();
    SerialPort_SendAear->setMaximumHeight(80);
    leftLayout->addWidget(new QLabel(QString::fromLocal8Bit("指令发送:")), 0);
    leftLayout->addWidget(SerialPort_SendAear, 1);

    // --- 右侧：配置面板 ---
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QWidget* configGroup = new QWidget();
    configGroup->setFixedWidth(300);
    QGridLayout* grid = new QGridLayout(configGroup);

    SerialPort_Number = new QComboBox();
    SerialPort_BaudRate = new QComboBox();
    SerialPort_BaudRate->addItems({ "9600","19200","38400","115200","256000","921600" });
    SerialPort_BaudRate->setCurrentText("115200");
    SerialPort_DataBits = new QComboBox(); SerialPort_DataBits->addItems({ "5","6","7","8" }); SerialPort_DataBits->setCurrentText("8");
    SerialPort_StopBits = new QComboBox(); SerialPort_StopBits->addItems({ "1","1.5","2" });
    SerialPort_CheckBits = new QComboBox(); SerialPort_CheckBits->addItems({ QString::fromLocal8Bit("无"),QString::fromLocal8Bit("奇"),QString::fromLocal8Bit("偶") });

    SerialPort_ReceiveMode = new QComboBox(); SerialPort_ReceiveMode->addItems({ "TEXT", "HEX" });
    SerialPort_SendMode = new QComboBox(); SerialPort_SendMode->addItems({ "TEXT", "HEX" });

    Edit_XRange = new QLineEdit("100");
    Edit_YMax = new QLineEdit("100");
    Edit_YMin = new QLineEdit("0");
    CheckBox_AutoScale = new QCheckBox(QString::fromLocal8Bit("Y轴自动量程"));
    CheckBox_AutoScroll = new QCheckBox(QString::fromLocal8Bit("X轴自动跟随"));
    CheckBox_AutoScroll->setChecked(true);
    CheckBox_Timestamp = new QCheckBox(QString::fromLocal8Bit("显示时间戳"));
    CheckBox_SaveCSV = new QCheckBox(QString::fromLocal8Bit("保存数据到CSV"));

    int r = 0;
    grid->addWidget(new QLabel(QString::fromLocal8Bit("串口号:")), r, 0); grid->addWidget(SerialPort_Number, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("波特率:")), r, 0); grid->addWidget(SerialPort_BaudRate, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("数据位:")), r, 0); grid->addWidget(SerialPort_DataBits, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("停止位:")), r, 0); grid->addWidget(SerialPort_StopBits, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("校验位:")), r, 0); grid->addWidget(SerialPort_CheckBits, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("接收模式:")), r, 0); grid->addWidget(SerialPort_ReceiveMode, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("发送模式:")), r, 0); grid->addWidget(SerialPort_SendMode, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("X显示跨度:")), r, 0); grid->addWidget(Edit_XRange, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("Y上限:")), r, 0); grid->addWidget(Edit_YMax, r++, 1);
    grid->addWidget(new QLabel(QString::fromLocal8Bit("Y下限:")), r, 0); grid->addWidget(Edit_YMin, r++, 1);
    grid->addWidget(CheckBox_AutoScale, r++, 1);
    grid->addWidget(CheckBox_AutoScroll, r++, 1);
    grid->addWidget(CheckBox_Timestamp, r++, 1);
    grid->addWidget(CheckBox_SaveCSV, r++, 1);

    rightLayout->addWidget(configGroup);
    rightLayout->addStretch();

    SerialPort_Connect = new QPushButton(QString::fromLocal8Bit("开启串口"));
    SerialPort_Disonnect = new QPushButton(QString::fromLocal8Bit("关闭串口"));
    SerialPort_Disonnect->setEnabled(false);
    SerialPort_Send = new QPushButton(QString::fromLocal8Bit("发送数据"));
    SerialPort_Send->setEnabled(false);

    rightLayout->addWidget(SerialPort_Connect);
    rightLayout->addWidget(SerialPort_Disonnect);
    rightLayout->addWidget(SerialPort_Send);

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 0);
}

void SerialPortAssistant::setupConnections()
{
    connect(SerialPort_Connect, &QPushButton::clicked, [=]() { togglePort(true); });
    connect(SerialPort_Disonnect, &QPushButton::clicked, [=]() { togglePort(false); });

    // --- CSV 动态开启/关闭修复 ---
    connect(CheckBox_SaveCSV, &QCheckBox::toggled, [=](bool checked) {
        if (serialPort->isOpen()) {
            if (checked && !csvFile.isOpen()) {
                QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
                csvFile.setFileName(fileName);
                if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&csvFile);
                    out.setGenerateByteOrderMark(true); // 解决Excel中文乱码
                    out << "Time,Direction,Data\n";
                    SerialPort_ReceiveAear->appendPlainText(QString::fromLocal8Bit("--- CSV录制已开启: %1 ---").arg(fileName));
                }
            }
            else if (!checked && csvFile.isOpen()) {
                csvFile.close();
                SerialPort_ReceiveAear->appendPlainText(QString::fromLocal8Bit("--- CSV录制已停止并保存 ---"));
            }
        }
        });

    // --- X轴动态刻度设置 ---
    connect(Edit_XRange, &QLineEdit::textChanged, [=](const QString& t) {
        int xRange = t.toInt(); if (xRange < 10) return;
        if (CheckBox_AutoScroll->isChecked()) {
            double startX = qMax(0.0, plotCount - xRange);
            axisX->setRange(startX, startX + xRange);
        }
        else {
            axisX->setMax(axisX->min() + xRange);
        }
        });

    // --- 自动跟随切换 ---
    connect(CheckBox_AutoScroll, &QCheckBox::toggled, [=](bool checked) {
        ScrollBar_X->setEnabled(!checked);
        if (checked) ScrollBar_X->setValue(ScrollBar_X->maximum());
        });

    // --- 滑动条控制历史拖动 ---
    connect(ScrollBar_X, &QScrollBar::valueChanged, [=](int value) {
        if (!CheckBox_AutoScroll->isChecked()) {
            int xRange = Edit_XRange->text().toInt();
            axisX->setRange(value, value + xRange);
        }
        });

    // --- 核心接收与绘图逻辑 ---
    connect(serialPort, &QSerialPort::readyRead, [=]() {
        QByteArray data = serialPort->readAll();
        if (data.isEmpty()) return;

        QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        QString raw = (SerialPort_ReceiveMode->currentText() == "HEX") ?
            data.toHex(' ').toUpper() : QString::fromLocal8Bit(data).trimmed();

        // 文本日志显示
        SerialPort_ReceiveAear->appendPlainText(CheckBox_Timestamp->isChecked() ? "[" + timeStr + "] " + raw : raw);
        SerialPort_ReceiveAear->moveCursor(QTextCursor::End);

        // CSV 写入逻辑
        if (csvFile.isOpen()) {
            QTextStream out(&csvFile);
            out << timeStr << ",RX," << "\"" << raw << "\"\n";
        }

        // 绘图逻辑
        bool ok;
        double val = raw.toDouble(&ok);
        if (ok) {
            series->append(plotCount, val);
            int xRange = Edit_XRange->text().toInt();

            if (CheckBox_AutoScroll->isChecked()) {
                double startX = qMax(0.0, plotCount - xRange);
                axisX->setRange(startX, startX + xRange);
                ScrollBar_X->setRange(0, qMax(0, (int)plotCount - xRange));
                ScrollBar_X->setValue(ScrollBar_X->maximum());
            }
            else {
                ScrollBar_X->setRange(0, qMax(0, (int)plotCount - xRange));
            }

            // Y轴自动量程（基于当前视窗）
            if (CheckBox_AutoScale->isChecked()) {
                auto points = series->pointsVector();
                int start = qMax(0, (int)points.size() - xRange);
                double minY = val, maxY = val;
                for (int i = start; i < points.size(); ++i) {
                    minY = qMin(minY, points[i].y()); maxY = qMax(maxY, points[i].y());
                }
                double padding = (maxY - minY) * 0.15 + 1.0;
                axisY->setRange(minY - padding, maxY + padding);
                Edit_YMax->setText(QString::number(maxY + padding, 'f', 2));
                Edit_YMin->setText(QString::number(minY - padding, 'f', 2));
            }
            else {
                axisY->setRange(Edit_YMin->text().toDouble(), Edit_YMax->text().toDouble());
            }
            plotCount++;
        }
        });

    // 发送逻辑
    connect(SerialPort_Send, &QPushButton::clicked, [=]() {
        if (serialPort->isOpen()) {
            QString text = SerialPort_SendAear->toPlainText();
            if (SerialPort_SendMode->currentText() == "HEX")
                serialPort->write(QByteArray::fromHex(text.toUtf8()));
            else
                serialPort->write(text.toLocal8Bit());
        }
        });
}

void SerialPortAssistant::togglePort(bool open)
{
    if (open) {
        if (SerialPort_Number->currentText().isEmpty()) return;
        serialPort->setPortName(SerialPort_Number->currentText());
        serialPort->setBaudRate(SerialPort_BaudRate->currentText().toInt());
        serialPort->setDataBits((QSerialPort::DataBits)SerialPort_DataBits->currentText().toInt());

        QString s = SerialPort_StopBits->currentText();
        serialPort->setStopBits(s == "1" ? QSerialPort::OneStop : (s == "1.5" ? QSerialPort::OneAndHalfStop : QSerialPort::TwoStop));

        QString c = SerialPort_CheckBits->currentText();
        serialPort->setParity(c.contains(QString::fromLocal8Bit("奇")) ? QSerialPort::OddParity : (c.contains(QString::fromLocal8Bit("偶")) ? QSerialPort::EvenParity : QSerialPort::NoParity));

        if (serialPort->open(QSerialPort::ReadWrite)) {
            // 如果连接瞬间勾选了CSV，开启文件
            if (CheckBox_SaveCSV->isChecked()) {
                csvFile.setFileName(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv");
                if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&csvFile); out.setGenerateByteOrderMark(true);
                    out << "Time,Direction,Data\n";
                }
            }
            SerialPort_Connect->setEnabled(false);
            SerialPort_Disonnect->setEnabled(true);
            SerialPort_Send->setEnabled(true);
            SerialPort_Number->setEnabled(false);
        }
        else {
            QMessageBox::critical(this, "Error", serialPort->errorString());
        }
    }
    else {
        if (csvFile.isOpen()) csvFile.close();
        serialPort->close();
        SerialPort_Connect->setEnabled(true);
        SerialPort_Disonnect->setEnabled(false);
        SerialPort_Send->setEnabled(false);
        SerialPort_Number->setEnabled(true);
    }
}

void SerialPortAssistant::updatePortList()
{
    QVector<QString> current;
    for (const QSerialPortInfo& i : QSerialPortInfo::availablePorts()) current << i.portName();
    if (current != lastPortList) {
        QString old = SerialPort_Number->currentText();
        SerialPort_Number->clear();
        for (const QString& s : current) SerialPort_Number->addItem(s);
        SerialPort_Number->setCurrentText(old);
        lastPortList = current;
    }
}

void SerialPortAssistant::timerEvent(QTimerEvent*) { updatePortList(); }
SerialPortAssistant::~SerialPortAssistant() { if (csvFile.isOpen()) csvFile.close(); }