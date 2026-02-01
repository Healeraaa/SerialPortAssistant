#include "SerialPortAssistant.h"

SerialPortAssistant::SerialPortAssistant(QWidget* parent) : QMainWindow(parent)
{
    // QString::fromLocal8Bit：将“本地字符编码”的字符串安全地转换成 Qt 内部使用的 Unicode 编码字符串。
    this->setWindowTitle(QString::fromLocal8Bit("串口示波器 - 最终全功能版"));
    // 设置窗口的最小尺寸
    this->setMinimumSize(1250, 950);
    //在堆上创建一个串口通讯对象。传递 this（即当前窗口的地址）作为其父对象，这样当主窗口销毁时，serialPort 也会被自动释放内存。
    serialPort = new QSerialPort(this);
	// 调用自定义函数来布局界面元素
    initUI();
    //调用自定义函数来绑定“信号与槽”（Signal & Slot）。它规定了点击按钮、串口收到数据等事件发生时，应该执行哪些操作
    setupConnections();
    /*开启一个定时器，每隔 1000 毫秒（1 秒）触发一次 timerEvent 事件
    步骤：
        1.注册定时器：Qt 操作系统层级注册一个定时器，设定间隔为 1000 毫秒。
        2.倒计时：程序继续执行其他代码，不阻塞。
        3.发送事件：时间到达后，Qt 系统会向该对象的事件队列中发送一个 QTimerEvent（定时器事件）。
        4.事件分发：主线程的事件循环（app.exec()）抓取到这个事件，并调用对象的 event() 函数。
        5.进入回调：最终，系统会自动调用你重写的 timerEvent(QTimerEvent* event) 函数。
    */
    this->startTimer(1000); // 串口自动巡检
    // 在程序一启动时，立即运行一次扫描函数，刷新串口选择框（ComboBox），让用户立刻看到当前可用的串口列表。
    updatePortList();
}

void SerialPortAssistant::initUI()
{
    /*
    QWidget 与其他组件的关系：
        Level 1: QWidget —— 最基础的矩形块，什么都没有。
        Level 2: QPushButton / QLabel —— 在 QWidget 基础上画上了文字和边框。
        Level 3: QMainWindow —— 在 QWidget 基础上进化出了菜单栏、状态栏和复杂的布局结构。
    */
    QWidget* centralWidget = new QWidget(this);
    // 将 centralWidget 这个控件，设置为当前主窗口的正中心核心部件
    this->setCentralWidget(centralWidget);
    // QHBoxLayout: Q 代表Qt框架，H 代表Horizontal（水平），Layout 代表布局。合起来就是“水平布局管理器”。
    /*
        核心功能拆解：
        1. 自动水平排列
            这是它最基本的功能。当你把控件（Widget）添加进 QHBoxLayout 时，它会按照代码添加的先后顺序，从左到右依次摆放。
                1.1无需手动计算坐标：不需要写 x=10, y=20 这种繁琐的代码。
                1.2自动对齐：它会自动处理控件之间的高度对齐，让界面看起来很专业。

        2. 动态自适应（伸缩性）
            这是布局管理器最强大的地方。当你用鼠标拖动窗口边缘、改变窗口大小时：
                2.1空间分配：布局会自动拉宽内部的控件，填满多出来的空间。
                2.2比例控制：代码里有 mainLayout->addLayout(leftLayout, 1); 和 mainLayout->addLayout(rightLayout, 0);。
                    2.2.1权重 1：代表左侧区域会像橡皮筋一样随着窗口变大而变宽（示波器图表就需要这个效果）。
                    2.2.2权重 0：代表右侧区域（配置面板）尽量保持自己的原始宽度，不随窗口变大而过度拉伸。

        3. 间距管理 (Spacing & Margins)
            它负责处理控件之间的“呼吸感”：
                3.1Spacing：控件与控件之间的缝隙。
                3.2Margins：整个布局与窗口边缘的距离。 这些都可以通过一行代码统一设置，而不需要一个一个调整控件。
    */
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // --- 左侧：数据显示与波形 --- 垂直布局管理器
    QVBoxLayout* leftLayout = new QVBoxLayout();
    // QPlainTextEdit - 多行文本编辑器
    SerialPort_ReceiveAear = new QPlainTextEdit();
    // 设置只读
    SerialPort_ReceiveAear->setReadOnly(true);
    /*
        添加标签 (Label)
            new QLabel(...)：现场创建了一个标签控件。它的作用是在界面上显示“接收日志:”这几个字，告诉用户下面这个大框框是干什么用的。
            leftLayout->addWidget(..., 0)：将这个标签放进左侧的垂直布局中。
            数字 0 的含义：这是伸缩因子（Stretch Factor）。0 表示这个标签不拉伸。无论窗口变多高，这个标签只占用它自身文字大小所需的固定高度。
    */
    leftLayout->addWidget(new QLabel(QString::fromLocal8Bit("接收日志:")), 0);
    /*
        添加文本区域 (Text Area)
            SerialPort_ReceiveAear：之前创建好的 QPlainTextEdit 对象（就是那个显示日志的大框）。
            数字 2 的含义：这是伸缩因子。它告诉布局管理器：“在分配剩余垂直空间时，请分给我 2份。”
    */
    leftLayout->addWidget(SerialPort_ReceiveAear, 2);

    /*
        QLineSeries 是“折线系列”。
        它的作用：
            它专门负责存储和管理所有的坐标点(x, y)。只要不停地给它新的坐标，它就会自动连成线。
        在示波器中：
            每当串口接收到一个数字，程序就会把这个数字追加到这个 series 里。
    */
    series = new QLineSeries();
    /*
        QChart 是整个图表的“大脑”或“容器”。
        它的作用：
            它不直接画线，但它管理着线条、标题、坐标轴、背景色等。它决定了图表长什么样，比如有没有网格线、背景是不是白色的。
    */
    QChart* chart = new QChart();
    /*
        它的作用：
            将刚才创建的折线 series 放入图表 chart 中。
        注意：
            一个图表可以添加多条线。在这个程序里，我们目前只添加了一条线，用来显示串口数据。
    */
    chart->addSeries(series);
	// 设置图表标题
    chart->setTitle(QString::fromLocal8Bit("实时波形分析"));
    /*
        什么是 Legend（图例）？ 
            通常在 Excel 图表中，右上角会有一个小框说明“蓝线代表温度，红线代表湿度”，那个就是图例。
        为什么要 hide（隐藏）？ 
            因为我们的示波器目前只有一条线，用户一眼就能看出是什么。隐藏图例可以节省空间，让绘图区域（波形显示区）变得更大、更直观。
    */
    chart->legend()->hide();
    /*
        QValueAxis: 这是一个数值坐标轴类。
        setRange(0, 100): 设定坐标轴的初始范围。
            X 轴：通常代表时间或采样点，这里设为 0 到 100，意味着屏幕初始化时显示 100 个点。
            Y 轴：代表串口传回的数值（比如电压、传感器读数）。
        setLabelFormat("%d"): 设定标签格式。%d 表示 X 轴刻度只显示整数（如 10, 20...），不显示小数（如 10.5）。
    */
    axisX = new QValueAxis(); axisX->setRange(0, 100); axisX->setLabelFormat("%d");
    axisY = new QValueAxis(); axisY->setRange(0, 100);
    /*
        这两行代码决定了轴的位置：
            Qt::AlignBottom: 把 X 轴钉在图表的底部。
            Qt::AlignLeft: 把 Y 轴钉在图表的左侧。
        注意：
            此时图表有了轴，但轴和线（series）还是互不相干的两个物件。
    */
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    /*
        attachAxis: 意思是“依附”。
        作用：告诉 series（数据线），“请按照 axisX 和 axisY 设定的比例尺来画你自己”。
        如果没有这一步：即便你给 series 传了坐标点 $(50, 50)$，它也不知道该画在图表的哪个位置，导致画面一片空白。
    */
    series->attachAxis(axisX); series->attachAxis(axisY);
    /*
        什么是 QChartView？ QChart（图表本身）只是内存中的一个逻辑对象，它负责计算数据，无法直接显示在窗口里。 
            QChartView 就是它的载体。它继承自 QWidget，是一个专门用来展示 QChart 的视图窗口。
        参数 (chart)： 这行代码就像是把刚才画好的“图纸” (chart) 塞进了一个“相框” (chartView)。没有这个相框，你的图表就无法摆放在界面布局里。
    */
    chartView = new QChartView(chart);
    /*
        Render Hint (渲染提示)：告诉绘图引擎在画图时要注意什么。
        Antialiasing (抗锯齿)：
            关闭时：线条是由一个个像素点阶梯状拼成的，斜线看起来会有明显的“毛刺”或“锯齿感”。
            开启后：Qt 会在像素边缘进行平滑处理（通过计算颜色过渡），让曲线看起来非常圆润、丝滑。
    */
    chartView->setRenderHint(QPainter::Antialiasing);
    // 将chartView放入左侧垂直布局 leftLayout 中
    leftLayout->addWidget(chartView, 5);
    /*
        创建横向滚动条
            QScrollBar：这是标准的滚动条组件（。
            Qt::Horizontal：指定它是水平方向的。在示波器中，它位于波形图的正下方，用来控制波形在时间轴（X轴）上的左右移动。
    */
    ScrollBar_X = new QScrollBar(Qt::Horizontal);
    // 初始状态禁用
    ScrollBar_X->setEnabled(false);
    // 将ScrollBar_X放入左侧垂直布局 leftLayout 中
    leftLayout->addWidget(ScrollBar_X);
    
	// 创建了一个纯文本编辑器，用于用户输入要发送的串口指令
    SerialPort_SendAear = new QPlainTextEdit();
	// 设置最大高度为80像素，防止占用过多垂直空间
    SerialPort_SendAear->setMaximumHeight(80);
	// 在左侧布局中添加一个标签，说明下面的文本框是用于发送指令的
    leftLayout->addWidget(new QLabel(QString::fromLocal8Bit("指令发送:")), 0);
	// 将发送指令的文本编辑器添加到左侧布局中，伸缩因子为1
    leftLayout->addWidget(SerialPort_SendAear, 1);

	// 创建垂直布局管理器 rightLayout 用于右侧配置面板
    QVBoxLayout* rightLayout = new QVBoxLayout();
	// 创建一个 QWidget 作为配置选项的容器
    QWidget* configGroup = new QWidget();
	// 设置固定宽度为 300 像素
    configGroup->setFixedWidth(300);
    /*
        创建一个网格布局管理器 grid 用于在 configGroup 中排列配置选项
            作用：在 configGroup 内部安装一个网格布局。
            逻辑：QGridLayout 是最像 Excel 表格的布局。它允许指定控件放在第几行、第几列。
            在项目里：
            第一列（Column 0）放标签：如“串口号:”、“波特率:”。
            第二列（Column 1）放输入框：如 QComboBox。
            这样能保证所有的冒号和下拉框都垂直对齐，看起来非常舒服。
    */
    QGridLayout* grid = new QGridLayout(configGroup);


    SerialPort_Number = new QComboBox();//下拉列表控件
    SerialPort_BaudRate = new QComboBox();
	SerialPort_BaudRate->addItems({ "9600","19200","38400","115200","256000","921600" });//添加波特率选项
	SerialPort_BaudRate->setCurrentText("115200");//默认选择115200波特率
    SerialPort_DataBits = new QComboBox(); SerialPort_DataBits->addItems({ "5","6","7","8" }); SerialPort_DataBits->setCurrentText("8");
    SerialPort_StopBits = new QComboBox(); SerialPort_StopBits->addItems({ "1","1.5","2" });
    SerialPort_CheckBits = new QComboBox(); SerialPort_CheckBits->addItems({ QString::fromLocal8Bit("无"),QString::fromLocal8Bit("奇"),QString::fromLocal8Bit("偶") });

    SerialPort_ReceiveMode = new QComboBox(); SerialPort_ReceiveMode->addItems({ "TEXT", "HEX" });
    SerialPort_SendMode = new QComboBox(); SerialPort_SendMode->addItems({ "TEXT", "HEX" });

    Edit_XRange = new QLineEdit("100");//数值输入框
    Edit_YMax = new QLineEdit("100");
    Edit_YMin = new QLineEdit("0");
    CheckBox_AutoScale = new QCheckBox(QString::fromLocal8Bit("Y轴自动量程"));//功能复选框
    CheckBox_AutoScroll = new QCheckBox(QString::fromLocal8Bit("X轴自动跟随"));
    CheckBox_AutoScroll->setChecked(true);
    CheckBox_Timestamp = new QCheckBox(QString::fromLocal8Bit("显示时间戳"));
    CheckBox_SaveCSV = new QCheckBox(QString::fromLocal8Bit("保存数据到CSV"));

	int r = 0; //行号计数器
    /*
        grid->addWidget(控件, 行, 列) 的逻辑
            QGridLayout 就像 Excel 表格：
                列 0 (Column 0)：放置 QLabel（标签）。这些是说明文字，如“串口号:”。
                列 1 (Column 1)：放置实际的控制组件，如 SerialPort_Number（下拉框）。
    */
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


	rightLayout->addWidget(configGroup);// 将配置面板添加到右侧布局中
    /* 
        添加一个伸缩空间（Stretch）。

            原理：
                可以把 addStretch() 想象成一个无形的弹簧。它没有固定的高度，但它会极力地“撑开”，占领所有剩下的空白空间。
            为什么要加它？
                如果没有它：当你垂直拉长整个软件窗口时，顶部的那些下拉框和输入框可能会被强行拉高、拉散。比如“串口号”和“波特率”之间会产生巨大的空隙，看起来非常滑稽且不专业。
                有了它之后：这个“弹簧”会把上面的所有配置项紧紧地推向顶部。无论你把窗口拉到多高，所有的设置按钮都会整齐地靠在上方，而多出来的空间则会变成下方的留白。
    */
	rightLayout->addStretch();// 添加弹性空间，推送下面的按钮到最底部

	SerialPort_Connect = new QPushButton(QString::fromLocal8Bit("开启串口"));// 创建按钮
    SerialPort_Disonnect = new QPushButton(QString::fromLocal8Bit("关闭串口"));
	SerialPort_Disonnect->setEnabled(false);// 初始状态禁用
    SerialPort_Send = new QPushButton(QString::fromLocal8Bit("发送数据"));
	SerialPort_Send->setEnabled(false);// 初始状态禁用

	rightLayout->addWidget(SerialPort_Connect);// 将按钮添加到右侧布局中
    rightLayout->addWidget(SerialPort_Disonnect);
    rightLayout->addWidget(SerialPort_Send);

	mainLayout->addLayout(leftLayout, 1);// 左侧布局拉伸因子1
	mainLayout->addLayout(rightLayout, 0);// 右侧布局拉伸因子0
    /*
        addWidget 的默认规则就是：先来后到，顺次排列。
            但有一种情况会打破这个视觉规律：addStretch()（弹簧）。
                在你之前的代码里：
                1.先添加了 configGroup（配置面板）。
                2.接着添加了 addStretch()（弹簧）。
                3.最后添加了三个按钮。
                视觉效果是： 
                    配置面板被顶在最上方，三个按钮被挤在最下方，中间是由于弹簧产生的巨大空隙。
                    虽然按钮是最后 addWidget 进去的，但因为弹簧的存在，它们看起来像是“掉到底部”了。
    */
}

void SerialPortAssistant::setupConnections()
{
    connect(SerialPort_Connect, &QPushButton::clicked, [=]() { togglePort(true); });
    connect(SerialPort_Disonnect, &QPushButton::clicked, [=]() { togglePort(false); });
    
    // --- CSV 动态开启/关闭修复 ---
	connect(CheckBox_SaveCSV, &QCheckBox::toggled, [=](bool checked) {// 勾选状态翻转时触发，携带一个 bool 参数，表明现在是开(true)还是关(false)。
		if (serialPort->isOpen()) {// 仅当串口处于打开状态时，才允许操作 CSV 文件
			if (checked && !csvFile.isOpen()) {// 如果勾选了“保存到 CSV”且当前没有打开 CSV 文件
                QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
				csvFile.setFileName(fileName);// 设置文件名
				if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {// 以写入文本模式打开文件
                    QTextStream out(&csvFile);
                    out.setGenerateByteOrderMark(true); // 解决Excel中文乱码
                    out << "Time,Direction,Data\n";
                    /*
                        SerialPort_ReceiveAear->appendPlainText(...)
                            在文本编辑器（通常是 QPlainTextEdit 类）的末尾追加一行文字。
                        .arg(fileName)：动态占位符
                            %1（占位符）：在字符串中，%1 是一个“预留位”。它告诉 Qt：“这里先空着，一会儿会有一个变量填进来。”
                            .arg() 函数：它的作用就是把括号里的变量（这里是 fileName，即之前生成的 20260201_131925.csv）塞进 %1 的位置。
                    */
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
	connect(Edit_XRange, &QLineEdit::textChanged, [=](const QString& t) {// 输入框（Edit_XRange）文本发生变化时触发，携带一个 QString 参数，代表新的文本内容。
		int xRange = t.toInt(); if (xRange < 10) return;// 转换为整数，并设置最小值为10，防止过小导致显示异常
		if (CheckBox_AutoScroll->isChecked()) {// 如果“自动跟随”复选框被勾选
            /*
                plotCount：当前串口收到的总点数（随着时间一直增加，比如从 0 增加到 10,000）。
                xRange：用户设定的屏幕宽度（比如屏幕一次只看 100 个点）。
                startX：计算出的屏幕左边缘对应的点数序号。
            */
			double startX = qMax(0.0, plotCount - xRange);// 越界保护逻辑，计算新的起始 X 坐标
			axisX->setRange(startX, startX + xRange);// 设置 X 轴范围
        }
        else {
			axisX->setMax(axisX->min() + xRange);// 非自动跟随模式下，仅调整最大值，还是X轴范围
        }
        });

    // --- 自动跟随切换 ---
	connect(CheckBox_AutoScroll, &QCheckBox::toggled, [=](bool checked) {// 勾选状态翻转时触发，携带一个 bool 参数，表明现在是开(true)还是关(false)。
		ScrollBar_X->setEnabled(!checked);// 根据是否自动跟随，启用或禁用滚动条
		if (checked) ScrollBar_X->setValue(ScrollBar_X->maximum());// 如果切换到自动跟随，滚动条直接跳到最右侧
        });

    // --- 滑动条控制历史拖动 ---
	connect(ScrollBar_X, &QScrollBar::valueChanged, [=](int value) {// 滑动条数值变化时触发，携带一个 int 参数，代表新的滑动条位置。
		if (!CheckBox_AutoScroll->isChecked()) {// 仅在非自动跟随模式下响应滑动条变化
			int xRange = Edit_XRange->text().toInt();// 获取当前 X 轴显示跨度
			axisX->setRange(value, value + xRange);// 根据滑动条位置调整 X 轴范围
        }
        });

    // --- 核心接收与绘图逻辑 ---
	connect(serialPort, &QSerialPort::readyRead, [=]() {// 当接收缓存器有新数据时触发
		QByteArray data = serialPort->readAll();// 读取所有数据
		if (data.isEmpty()) return;// 数据为空则返回

		QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");// 获取当前时间字符串，格式为时:分:秒.毫秒
        /*
            toHex(' ')：
                将 QByteArray 中的每个字节（Byte）转换成十六进制表示，并在每个字节之间插入一个空格（' '）。
                data.toHex(':') 输出类似 55 AA 01。
                data.toHex(':') 输出类似 55:AA:01。
                data.toHex('-') 输出类似 55-AA-01。
            .toUpper()：将所有小写字母转换成大写。
                如"1a 2b" --->"1A 2B"
            .trimmed()：
                去除首尾空白。它会删掉字符串开头和结尾的所有换行符（\n）、回车符（\r）、空格或制表符。
                原因：很多单片机发送数据时习惯以 \r\n 结尾，直接显示会导致 UI 界面出现不必要的空行。trimmed() 能让日志排版更紧凑。
        */
        QString raw = (SerialPort_ReceiveMode->currentText() == "HEX") ?
            data.toHex(' ').toUpper() : QString::fromLocal8Bit(data).trimmed();

        // 文本日志显示
		SerialPort_ReceiveAear->appendPlainText(CheckBox_Timestamp->isChecked() ? "[" + timeStr + "] " + raw : raw);// 是否显示时间戳
        /*
            moveCursor(...)
                操作文本框的“光标”
            QTextCursor::End
                这是一个预定义的常量，代表文档的绝对末尾（即最后一个字符之后）
        */
		SerialPort_ReceiveAear->moveCursor(QTextCursor::End);// 光标移动到最底部，自动滚动到最新日志

        // CSV 写入逻辑
        if (csvFile.isOpen()) {
			QTextStream out(&csvFile);// 创建文本流对象，关联到 csvFile 文件
			out << timeStr << ",RX," << "\"" << raw << "\"\n";// 按行写入时间戳、方向和数据，数据用引号括起来以防逗号干扰
        }

        // 绘图逻辑
        bool ok;
		double val = raw.toDouble(&ok);// 防御性编程；尝试将接收到的字符串转换为浮点数，如果失败 ok 会被设置为 false，代表不是有效数字
        if (ok) {
			series->append(plotCount, val);// 将新点添加到折线系列中；plotCount:x坐标,当前是第几个数据点；val：y坐标,从串口解析出来的实际数值
			int xRange = Edit_XRange->text().toInt();// 获取当前 X 轴显示跨度`

			if (CheckBox_AutoScroll->isChecked()) {// X轴自动跟随
                double startX = qMax(0.0, plotCount - xRange);// 计算起始坐标
				axisX->setRange(startX, startX + xRange);// 设置 X 轴范围
				ScrollBar_X->setRange(0, qMax(0, (int)plotCount - xRange));// 更新滚动条范围
				ScrollBar_X->setValue(ScrollBar_X->maximum());// 设置滚动条值到最大，保持在最右侧
            }
            else {
				ScrollBar_X->setRange(0, qMax(0, (int)plotCount - xRange));// 更新滚动条范围
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
				Edit_YMax->setText(QString::number(maxY + padding, 'f', 2));// 设置YMax文本框显示，保留两位小数
                Edit_YMin->setText(QString::number(minY - padding, 'f', 2));// 设置YMin文本框显示，保留两位小数
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

// 串口开关逻辑
void SerialPortAssistant::togglePort(bool open)
{
    if (open) {
		if (SerialPort_Number->currentText().isEmpty()) return;// 没有选择串口则返回
        serialPort->setPortName(SerialPort_Number->currentText());//获得底层的串口硬件对象
        // .toInt()：将字符串转换为整数
		serialPort->setBaudRate(SerialPort_BaudRate->currentText().toInt());//设置波特率
        serialPort->setDataBits((QSerialPort::DataBits)SerialPort_DataBits->currentText().toInt());//设置数据位

        QString s = SerialPort_StopBits->currentText();
		serialPort->setStopBits(s == "1" ? QSerialPort::OneStop : (s == "1.5" ? QSerialPort::OneAndHalfStop : QSerialPort::TwoStop));//设置停止位

        /*
        ①为什么要用 contains 而不是直接等于？
            使用 c.contains(...) 比 c == "奇" 更健壮。如果下拉框文字是“奇校验(Odd)”或者包含空格，contains 依然能正确识别出关键字“奇”。
        ② QString::fromLocal8Bit 的作用
            为了防止中文乱码。源代码文件可能是 GBK 或 UTF-8 编码，直接写 "奇" 可能会导致程序在运行时识别不出这个字符。fromLocal8Bit 会根据系统的本地设置来正确转码。
        */
        QString c = SerialPort_CheckBits->currentText();
		serialPort->setParity(c.contains(QString::fromLocal8Bit("奇")) ? QSerialPort::OddParity : (c.contains(QString::fromLocal8Bit("偶")) ? QSerialPort::EvenParity : QSerialPort::NoParity));//设置校验位

		if (serialPort->open(QSerialPort::ReadWrite)) {//打开串口
            // 如果连接瞬间勾选了CSV，开启文件
            if (CheckBox_SaveCSV->isChecked()) {// “保存数据到CSV”的勾选框被选中时
                /*
                    csvFile.setFileName(...) —— 设置文件名
                        QDateTime::currentDateTime()：获取当前的日期和时间。
                        toString("yyyyMMdd_HHmmss")：将日期时间格式化为字符串，格式为“年年年年月月日日_时时分分秒秒”，例如“20240615_142530”。
                        + ".csv"：在格式化后的字符串后面添加“.csv”扩展名，表示这是一个 CSV 文件。
                        格式化占位符：
                            占位符,含义,示例值
                            yyyy,四位年份,2026
                            MM,两位月份（补零）,01
                            dd,两位日期（补零）,31
                            _,下划线分隔符,_
                            HH,24小时制的小时,21
                            mm,分钟,09
                            ss,秒钟,45
                */
                csvFile.setFileName(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv");
                /*
                    csvFile.open(...) —— 打开文件
                        QIODevice::WriteOnly：以写入模式打开文件。如果文件不存在，会创建一个新文件；如果文件已存在，会覆盖原有内容。
						QIODevice::Text：以文本模式打开文件，确保写入的内容是以文本格式存储，而不是二进制格式。
					QTextStream out(&csvFile)：创建一个文本流对象 out，并将其与 csvFile 关联起来。这样就可以通过 out 来写入文本数据到 csvFile。
                */
                if (csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&csvFile); //：实例化一个 QTextStream 类，并立即调用它的构造函数，把 csvFile 的地址传进去。
					out.setGenerateByteOrderMark(true);// 解决Excel中文乱码
					out << "Time,Direction,Data\n";// 写入 CSV 文件的表头
                }
            }
            SerialPort_Connect->setEnabled(false);
            SerialPort_Disonnect->setEnabled(true);
            SerialPort_Send->setEnabled(true);
            SerialPort_Number->setEnabled(false);
        }
        else {
			QMessageBox::critical(this, "Error", serialPort->errorString());//弹出错误对话框
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
    /*
        for (... : ...)
            固定的语法结构，意思是“遍历冒号右边的集合，依次放入冒号左边的变量”。
        QSerialPortInfo::availablePorts()
            这是 Qt 提供的静态函数，它会扫描系统底层（设备管理器）并返回当前所有可用的串口列表（如 COM1, COM3）。
        current << i.portName()
            遍历扫描到的信息，只提取端口名字（String），存入 current 容器中。
    */
    for (const QSerialPortInfo& i : QSerialPortInfo::availablePorts()) current << i.portName();
	if (current != lastPortList) {// 仅当串口列表发生变化时才更新UI，避免频繁刷新导致界面卡顿
		QString old = SerialPort_Number->currentText();// 保存当前选择的串口号
		SerialPort_Number->clear();// 清空下拉列表
		for (const QString& s : current) SerialPort_Number->addItem(s);// 重新添加当前可用的串口号
        SerialPort_Number->setCurrentText(old);
        lastPortList = current;
    }
}

void SerialPortAssistant::timerEvent(QTimerEvent*) { updatePortList(); }
SerialPortAssistant::~SerialPortAssistant() { if (csvFile.isOpen()) csvFile.close(); }