#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot.h"

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QPointer>
#include <QButtonGroup>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadStyleSheet(const QString &styleSheetFile);

private slots:
    void on_OpenSerialButton_clicked();

    void ReadData();

    void UpdatePort();

    void on_sendButt_clicked();

    void on_clearButt_clicked();

//    void showchart(); 封装弃用
    void on_saveButton_clicked();

    void istoshowChart();

    void adPID();

    void clearShowchar();

    QString copy(QString str,int index);

    void data_show_char(int buff_index,QVector<double> *d,QString id);

    void onRadio();

private:
    QVector<double> y,x,y1,y2,y3; // initialize with entries 0..6000
    Ui::MainWindow *ui;

    QSerialPort *serial;
    QTimer *timer;
    QStringList oldPortStringList;

    QByteArray buf;
    QByteArray buf2;

    int rangy=800;
    int rangx=400;
    int i=0;
    int flage=0;
    int char_id=1;
    QString test;

    QString PID;

    QCPGraph *pGraph;
    QCPGraph *pGraph1;
    QCPGraph *pGraph2;
    QCPGraph *pGraph3;

    QButtonGroup *RGroup;

};

#endif // MAINWINDOW_H
