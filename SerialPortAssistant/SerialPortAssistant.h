#pragma once

#ifndef _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#endif

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit> 
#include <QPushButton>
#include <QComboBox> 
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QScrollBar>
#include <QSerialPort> 
#include <QSerialPortInfo>
#include <QMessageBox> 
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

// Qt Charts
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

class SerialPortAssistant : public QMainWindow
{
    Q_OBJECT

public:
    SerialPortAssistant(QWidget* parent = nullptr);
    ~SerialPortAssistant();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void initUI();
    void setupConnections();
    void updatePortList();
    void togglePort(bool open);

    // UI ¿Ø¼þ
    QPlainTextEdit* SerialPort_ReceiveAear, * SerialPort_SendAear;
    QPushButton* SerialPort_Connect, * SerialPort_Disonnect, * SerialPort_Send;
    QComboBox* SerialPort_Number, * SerialPort_BaudRate, * SerialPort_DataBits, * SerialPort_StopBits, * SerialPort_CheckBits;
    QComboBox* SerialPort_ReceiveMode, * SerialPort_SendMode;
    QCheckBox* CheckBox_Timestamp, * CheckBox_SaveCSV, * CheckBox_AutoScale, * CheckBox_AutoScroll;
    QLineEdit* Edit_YMax, * Edit_YMin, * Edit_XRange;
    QScrollBar* ScrollBar_X;

    // Í¼±íÓëÂß¼­
    QChartView* chartView;
    QLineSeries* series;
    QValueAxis* axisX, * axisY;
    double plotCount = 0;
    QSerialPort* serialPort;
    QFile csvFile;
    QVector<QString> lastPortList;
};