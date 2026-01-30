#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit> 
#include <QPushButton>
#include <QComboBox> 
#include <QLabel>
#include <QDebug>
#include <QSerialPort> 
#include <QSerialPortInfo>
#include <QMessageBox> 

class SerialPortAssistant : public QMainWindow
{
    Q_OBJECT

public:
    SerialPortAssistant(QWidget *parent = nullptr);
    ~SerialPortAssistant();

	void SerialPort_SendAear_Init();
    void SerialPort_ReceiveAear_Init();
	void SerialPort_Control_Init();
    void SerialPort_Connection_Init();
    void SerialPort_Connection();
    void timerEvent(QTimerEvent* event);


private:
    QPlainTextEdit* SerialPort_SendAear;
    QPlainTextEdit* SerialPort_ReceiveAear;

	QPushButton* SerialPort_Send;
	QPushButton* SerialPort_Connect;
    QPushButton* SerialPort_Disonnect;

	QComboBox* SerialPort_Number;
    QComboBox* SerialPort_BaudRate;
    QComboBox* SerialPort_DataBits;
    QComboBox* SerialPort_StopBits;
    QComboBox* SerialPort_CheckBits;
    QComboBox* SerialPort_SendMode;
	QComboBox* SerialPort_ReceiveMode;

	QSerialPort* serialPort;
	QVector<QString> portList;
};

