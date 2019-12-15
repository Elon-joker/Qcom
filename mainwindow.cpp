#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QPoint>
#include <QFile>
#include <math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    // 申请空间储存数据:
    y(6000),x(6000),y1(6000),y2(6000),y3(6000),

    ui(new Ui::MainWindow)
{
    ui->setupUi(this);   
    //this->setWindowFlags(Qt::FramelessWindowHint); // 设置标题框不可见，而后可以自定义标题框
    //单选框初始化
    RGroup = new QButtonGroup(this);
    RGroup->addButton(ui->radioButton,0);
    RGroup->addButton(ui->radioButton_2,1);
    RGroup->addButton(ui->radioButton_3,2);
    RGroup->addButton(ui->radioButton_4,3);
    ui->radioButton->setChecked(true);//默认选择

    //搜索串口
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
       {
             QSerialPort serial;
           serial.setPort(info);
           if(serial.open(QIODevice::ReadWrite))
             {
                ui->PortBox->addItem(serial.portName());
                serial.close();
            }
        }

    //定时器，用于更新串口
    timer = new QTimer;
    timer->start(1000); //一次检测
     QStringList newPortStringList;
     const auto infos = QSerialPortInfo::availablePorts();
     for (const QSerialPortInfo &info : infos)
     {
         newPortStringList += info.portName();
     }

     //更新串口号
     if(newPortStringList.size() != oldPortStringList.size())
     {
         oldPortStringList = newPortStringList;
         ui->PortBox->clear();
         ui->PortBox->addItems(oldPortStringList);
     }

         //设置波特率下拉菜单默认显示第3项
         ui->BaudBox->setCurrentIndex(2);
         //设置曲线图例可见
         pGraph=ui->customPlot->addGraph();
            pGraph->setName("电机1曲线");
         pGraph1=ui->customPlot->addGraph();
            pGraph1->setName("电机2曲线");
         pGraph2=ui->customPlot->addGraph();
            pGraph2->setName("电机3曲线");
         pGraph3=ui->customPlot->addGraph();
            pGraph3->setName("电机4曲线");
        //设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放、曲线可选、legend可选、设置伸缩比例，使所有图例可见legend->setVisible(false)
        ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal);  //设置允许在某一方向上拽托
        ui->customPlot->legend->setVisible(false);

       //链接信号与槽（即动作与响应的关联）
       //connect（谁发出信号，什么信号，谁响应事件，什么响应事件）；
       connect(timer,&QTimer::timeout,this,&MainWindow::UpdatePort);//更新端口号
       connect(ui->sendButt,SIGNAL(clicked()),this,SLOT(on_SendButt_clicked));
       connect(ui->clearButt,SIGNAL(clicked()),this,SLOT(on_clearButt_clicked));
       connect(ui->saveButt,SIGNAL(clicked()),this,SLOT(on_saveButton_clicked()));
       connect(ui->showChar,SIGNAL(clicked()),this,SLOT(istoshowChart()));
       connect(ui->adjustButt,SIGNAL(clicked()),this,SLOT(adPID()));
       connect(ui->clearchar,SIGNAL(clicked()),this,SLOT(clearShowchar()));
       connect(ui->reButt,SIGNAL(clicked()),this,SLOT(recevie()));

       connect(ui->radioButton,SIGNAL(clicked()),this,SLOT(onRadio()));
       connect(ui->radioButton_2,SIGNAL(clicked()),this,SLOT(onRadio()));
       connect(ui->radioButton_3,SIGNAL(clicked()),this,SLOT(onRadio()));
       connect(ui->radioButton_4,SIGNAL(clicked()),this,SLOT(onRadio()));

       connect(ui->action,SIGNAL(triggered()),this,SLOT(change_pf_1()));
       connect(ui->action_2,SIGNAL(triggered()),this,SLOT(change_pf_2()));
}


MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @function  打开串口
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation finish
 */
void MainWindow::on_OpenSerialButton_clicked()
{
    if(ui->PortBox->currentText()==tr("NO"))
    {
    //设置弹窗，防止无串口输出异常
    QMessageBox msg0(this);  //对话框设置父组件
    msg0.setWindowTitle("错误提示！");//对话框标题
    msg0.setText("未发现串口!");//对话框提示文本
    msg0.setIcon(QMessageBox::Information);//设置图标类型
    msg0.setStandardButtons(QMessageBox::Ok | QMessageBox:: Cancel);//对话框上包含的按钮

    if(msg0.exec() == QMessageBox::Ok)  // 判断是否退出弹窗
    {
       qDebug() << " Ok is clicked!";   //数据处理
    }
    }
    else {
        if(ui->OpenSerialButton->text() == tr("打开串口"))
        {
            serial = new QSerialPort;
            //设置串口名
            serial->setPortName(ui->PortBox->currentText());
            //打开串口
            serial->open(QIODevice::ReadWrite);
            //设置波特率
            switch (ui->BaudBox->currentIndex())
            {
            case 0:
                serial->setBaudRate(QSerialPort::Baud9600);
                break;
            case 1:
                serial->setBaudRate(QSerialPort::Baud38400);
              break;
            case 2:
                serial->setBaudRate(QSerialPort::Baud115200);
              break;
            default:
                break;
            }

            //设置数据位数
            switch (ui->BitBox->currentIndex())
            {
            case 8:
                serial->setDataBits(QSerialPort::Data8);//设置数据位8
                break;
            default:
                break;
            }
            //设置校验位
            switch (ui->ParityBox->currentIndex())
            {
            case 0:
                serial->setParity(QSerialPort::NoParity);
                break;
            default:
                break;
            }
            //设置停止位
            switch (ui->BitBox->currentIndex())
            {
            case 1:
                serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
                break;
            case 2:
                serial->setStopBits(QSerialPort::TwoStop);
                break;
            default:
                break;
            }
            //设置流控制
            serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

            //关闭设置菜单使能
            ui->PortBox->setEnabled(false);
            ui->BaudBox->setEnabled(false);
            ui->BitBox->setEnabled(false);
            ui->ParityBox->setEnabled(false);
            ui->StopBox->setEnabled(false);
            ui->OpenSerialButton->setText(tr("关闭串口"));

            //连接信号槽
            QObject::connect(serial,&QSerialPort::readyRead,this,&MainWindow::ReadData);
        }
        else
        {
            //关闭串口
            serial->clear();
            serial->close();
            serial->deleteLater();

            //恢复设置使能
            ui->PortBox->setEnabled(true);
            ui->BaudBox->setEnabled(true);
            ui->BitBox->setEnabled(true);
            ui->ParityBox->setEnabled(true);
            ui->StopBox->setEnabled(true);
            ui->OpenSerialButton->setText(tr("打开串口"));

        }

    }

}

/**
 * @function  读取串口信息
 * @param     void
 * @return    void
 * @version   2019.12.13
 * @author    占建
 * @situation finish
 */
void MainWindow::ReadData()
{
   buf=serial->readAll();//将串口里的可读信息全部读取
//   buf=serial->read(1);     //只读取一个字节的内容
//    if(!buf.isEmpty()&&buf.data()[0]=='a'&&buf.data()[6]=='b')
//    {
// //        //协议解析：a(实际数据)b
// //        //当拼接时，如果有\u0000被赋予，会无法使用toInt()函数正常转换&&buf.data()[count+1]!='d'
//        for(int count=0;count<4;count++)
//        {
//            if(buf.data()[count+1]!='\u0000')
//             data[count]=buf.data()[1+count];
//            if(buf.data()[count+1+6]!='\u0000')
//             data2[count]=buf.data()[1+count+6];
//        }
// //        /* 输出到控制台进行测试 */
//        qDebug()<<buf;
//       int temp1=0,temp2=0,po=1;
//       for(int j=0;j<4;j++)
//       {
//           temp1+=data.toInt()/po%10;
//           temp2+=data2.toInt()/po%10;
//           po*=10;
//       }
//       QString bb="b",aa="a";
//       bb=buf.data()[5];
//       aa=buf.data()[11];
//    if(temp1%10==bb.toInt()&&temp2%10==aa.toInt())
//    {
//        //判断是否画图
//            if(flage)
//                showchart();
// //        ui->textBrowser->insertPlainText(myStrTemp);
// //        将数据展示在textBrowser上，并且该函数自动换行
//          ui->textBrowser->append("data1:"+data+"\t"+"data2:"+data2);
//    }
//    }
   //判断接收模式：绘图接收或原始接收
     if(!re_statu)
     {switch (char_id)
   {
      case 1:
          pGraph->setVisible(true);
          if(!buf.isEmpty()&&buf.data()[0]=='a')
           {
              data_show_char(1,&y,"1");
          }
       break;

      case 2:
          pGraph1->setVisible(true);
       if(!buf.isEmpty()&&buf.data()[0]=='a'&&buf.data()[6]=='b')
          {
           data_show_char(1,&y,"1");
           data_show_char(7,&y1,"2");
          }
       break;

      case 3:
       pGraph2->setVisible(true);
       if(!buf.isEmpty()&&buf.data()[0]=='a'&&buf.data()[6]=='b'&&buf.data()[11]=='c')
          {
           data_show_char(1,&y,"1");
           data_show_char(7,&y1,"2");
           data_show_char(12,&y2,"3");
          }
      break;

      case 4:
        pGraph3->setVisible(true);
       if(!buf.isEmpty()&&buf.data()[0]=='a'&&buf.data()[6]=='b'&&buf.data()[11]=='c'&&buf.data()[16]=='d')
          {
           data_show_char(1,&y,"1");
           data_show_char(7,&y1,"2");
           data_show_char(12,&y2,"3");
           data_show_char(17,&y3,"4");
          }
      break;

   default:
       break;
   }
    buf.clear();
  }
     else {
         ui->textBrowser->append(buf);
     }
}

/**
 * @function  实时更新串口，检测是否有串口插入，拔出
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation finish
 */
void MainWindow::UpdatePort()
{
    QStringList newPortStringList;//存放新串口号，用于比较检测和更新
    const auto infos = QSerialPortInfo::availablePorts();

    //自动搜索所有串口
    for (const QSerialPortInfo &info : infos)
    {
        newPortStringList += info.portName();
    }
    //更新串口号
    if(newPortStringList.size() != oldPortStringList.size())
    {
        oldPortStringList = newPortStringList;
         ui->PortBox->clear();
         ui->PortBox->addItems(oldPortStringList);
    }

}

/**
 * @function  发送槽函数
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation finish
 */
void MainWindow::on_sendButt_clicked()
{
    if(ui->PortBox->currentText()!=tr("NO"))
    serial->write(ui->textEdit_2->toPlainText().toLatin1());
    else {
        //设置弹窗，防止无串口输出异常
        QMessageBox msg(this);  //对话框设置父组件
        msg.setWindowTitle("错误提示！");//对话框标题
        msg.setText("未发现串口!");//对话框提示文本
        msg.setIcon(QMessageBox::Information);//设置图标类型
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox:: Cancel);//对话框上包含的按钮
        if(msg.exec() == QMessageBox::Ok)
            // 判断是否退出弹窗
        {
           qDebug() << " Ok is clicked!";   //数据处理
        }
    }
}

/**
 * @function  清除槽函数
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation finish
 */
void MainWindow::on_clearButt_clicked()
{
    ui->textBrowser->clear();
}

// /**
// * @function  画图槽函数
// * @param     void
// * @return    void
// * @version   2019.11.10
// * @author    占建
// * @situation 封装到data_show_char
// *
// */
//void MainWindow::showchart()
//{
//    // give the axes some labels:
//    ui->customPlot->xAxis->setLabel("t");
//    ui->customPlot->yAxis->setLabel("V");
// //    //QVector<double> y,x 存放数据的容器
// //    //i  计数器  一直增加 来一条数据增加一下 控制x轴前进 实现动态效果
// //    //这时容器里面还没10个点 所有一直向里面存
// //    if(i < 10)
// //    {
// //        y.append(data.toInt());
// //        x.append(i);
// //        y1.append(data2.toInt());
// //        //设置范围正好 能显示当前点
// //    //   ui->customPlot->xAxis->setRange(0,100);
// //   //    ui->customPlot->yAxis->setRange(0,10000);
// //    }
// //    else
// //    {
// //        //容器数据现在是正好10个  把第一个出栈  把第11个入栈  正好还是10个数据
// //       y.removeFirst();
// //       y1.removeFirst();
// //       x.removeFirst();

// //        //入栈
// //           x.append(i);
// //       y.append(data.toInt());
// //       y1.append(data2.toInt());
// //        //设置范围正好 能显示当前点并使曲线随x轴运动
//          if(i==300)
//          ui->customPlot->xAxis->setRange(0,400);
//          else
//          ui->customPlot->xAxis->setRange(i-300,100+i);
// //    }
//          switch (char_id)
//          {
//          case 1:
//          pGraph->setData(x,y);
//          pGraph->setPen(QPen(Qt::blue));
//                 break;
//          case 2:
//          pGraph->setData(x,y);
//          pGraph->setPen(QPen(Qt::blue));
//          pGraph1->setData(x,y1);
//          pGraph1->setPen(QPen(Qt::yellow));
//                 break;
//          case 3:
//          pGraph->setData(x,y);
//          pGraph->setPen(QPen(Qt::blue));
//          pGraph1->setData(x,y1);
//          pGraph1->setPen(QPen(Qt::yellow));
//          pGraph2->setData(x,y2);
//          pGraph2->setPen(QPen(Qt::red));
//                 break;

//          case 4:
//          pGraph->setData(x,y);
//          pGraph->setPen(QPen(Qt::blue));
//          pGraph1->setData(x,y1);
//          pGraph1->setPen(QPen(Qt::yellow));
//          pGraph2->setData(x,y2);
//          pGraph2->setPen(QPen(Qt::red));
//          pGraph3->setData(x,y3);
//          pGraph3->setPen(QPen(Qt::black));
//                 break;
//          }
//   //设置坐标系 自动缩放以正常显示所有的数据
//    ui->customPlot->yAxis->rescale(true);
//    ui->customPlot->replot();
//    //计数器，实现动态画线
//   i++;
//}

/**
 * @function  保存数据
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation finish
 */
void MainWindow::on_saveButton_clicked()
{
   QString sFilePath="D:\\test.txt";
   QFile file(sFilePath);
   if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
   { QMessageBox::critical(nullptr,"提示","无法创建文件");
       return;
   }
  QString strTxtEdt = ui->textBrowser->toPlainText();

  std::string str = strTxtEdt.toStdString();
  const char* ch = str.c_str();

  file.write(ch);//将ch存进test.txt文件夹里面
   file.flush();
   file.close();
}

/**
 * @function  判断是否需要画图
 * @param     void
 * @return    void
 * @version   2019.11.3
 * @author    占建
 * @situation  finish
 */
void MainWindow::istoshowChart()
{
    if(ui->showChar->text()==tr("绘制图像"))
       {
        //退出原始接收
        re_statu=false;
        //进入绘图接收
        flage=1;
        ui->showChar->setText("停止绘图");

       }
    else {
        flage=0;
        ui->showChar->setText("绘制图像");
    }
}

/**
 * @function  调PID参数
 * @param     void
 * @return    void
 * @version   2019.11.20
 * @author    占建
 * @situation finish
 */
void MainWindow:: adPID()
{
    if(ui->PortBox->currentText()!=tr("NO")&&ui->OpenSerialButton->text()==tr("关闭串口"))
    {
        PID[0]='P';
        test=ui->PValue->toPlainText();
        PID=copy(PID,1);

        PID[6]='I';
        test=ui->IValue->toPlainText();        
        PID=copy(PID,7);

        PID[12]='D';
        test=ui->DValue->toPlainText();
        PID=copy(PID,13);

        std::string str = PID.toStdString();
        const char* ch =str.c_str();
        serial->write(ch);
        qDebug()<<ch;
    }
    else {
        //设置弹窗，防止无串口输出异常
        QMessageBox msg1(this);  //对话框设置父组件
        msg1.setWindowTitle("错误提示！");//对话框标题
        msg1.setText("未发现串口!");//对话框提示文本
        msg1.setIcon(QMessageBox::Information);//设置图标类型
        msg1.setStandardButtons(QMessageBox::Ok | QMessageBox:: Cancel);//对话框上包含的按钮

        if(msg1.exec() == QMessageBox::Ok)
            // 判断是否退出弹窗
        {
           qDebug() << " Ok is clicked!";   //数据处理
        }
         }
}

/**
 * @function  清除图像
 * @param     void
 * @return    void
 * @version   2019.11.9
 * @author    占建
 * @situation TODO
 */
void MainWindow:: clearShowchar()
{
//    //清空计数器，从0开始画
//        i=0;
//    //删除对应图层
//        ui->customPlot->removeGraph(0);
//        ui->customPlot->replot();
//        ui->customPlot->removeGraph(1);
//        ui->customPlot->replot();
//    //删除后停止绘图
//        flage=0;
//        ui->showChar->setText("绘制图像");
//    //创建新图层
//        pGraph=ui->customPlot->addGraph();
//        pGraph->setName("电机1曲线");
//        pGraph1=ui->customPlot->addGraph();
//        pGraph1->setName("电机2曲线");
//    //重新绘制图像
//        ui->customPlot->replot();
}

/**
 * @function  复制字符串到指定位置
 * @param     void
 * @return    QString
 * @version   2019.11.20
 * @author    占建
 * @situation finish
 */
QString MainWindow:: copy(QString str,int index)
{
    for(int i=index;i<5+index;i++)
            str[i]=test[i-index];
    return str;
}

/**
 * @function  执行样式表文件
 * @param     filePath
 * @return    void
 * @version   2019.11.15
 * @author    占建
 * @situation finish
 */
void MainWindow::loadStyleSheet(const QString &styleSheetFile)
{
    QFile file(styleSheetFile);
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString styleSheet = this->styleSheet();
        styleSheet += QLatin1String(file.readAll());//读取样式表文件
        this->setStyleSheet(styleSheet);//把文件内容传参
        file.close();
    }
    else
    {
        QMessageBox::information(this,"tip","cannot find qss file");
    }
}

/**
 * @function  封装一组数据的处理和画图功能
 * @param     存数据的位置 d,截取数据的起始地方 buff_index,数据暂存的位置 data_temp,第几组数据 id
 * @return    void
 * @version   2019.12.8
 * @author    占建
 * @situation finish
 */
void MainWindow:: data_show_char(int buff_index,QVector<double> *d,QString id)
{
      QString data_temp="0";
    //        //协议解析：a(实际数据)b
    //        //当拼接时，如果有\u0000被赋予，会无法使用toInt()函数正常转换&&buf.data()[count+1]!='d'
            for(int count=buff_index;count<4+buff_index;count++)
            {
                if(buf.data()[count]!='\u0000')
                 data_temp[count-buff_index]=buf.data()[count];
            }
    //        /* 输出到控制台进行测试 */
//            qDebug()<<data_temp.toInt();

   //第二重校验，位数的求和取个位数
           int temp1=0,po=1;
           for(int j=0;j<4;j++)
           {
               temp1+=data_temp.toInt()/po%10;
               po*=10;
           }
           QString bb="b";
           bb=buf.data()[buff_index+4];
        if(temp1%10==bb.toInt())
        {
            //QVector<double> y,x 存放数据的容器
            //i  计数器  一直增加 来一条数据增加一下 控制x轴前进 实现动态效果
            //这时容器里面还没10个点 所有一直向里面存
            if(flage)
            {
                ui->customPlot->xAxis->setLabel("t");
                ui->customPlot->yAxis->setLabel("V");
                if(i < 10)
                {
                    d->append(data_temp.toInt());
                    x.append(i);
                }
                else
                {
                    //容器数据现在是正好10个  把第一个出栈  把第11个入栈  正好还是10个数据
                   d->removeFirst();
                   x.removeFirst();

                    //入栈
                   x.append(i);
                   d->append(data_temp.toInt());
                 }

                //设置x轴的范围，使得数据不压缩且动态跟随显示
                if(i==300)
                ui->customPlot->xAxis->setRange(0,400);
                else
                ui->customPlot->xAxis->setRange(i-300,100+i);

                //设置y轴自动根据数据进行缩放
                ui->customPlot->yAxis->rescale(true);

                //判断画多少曲线
                switch (char_id)
                {
                case 1:
                pGraph->setData(x,y);
                pGraph->setPen(QPen(Qt::blue));
                       break;
                case 2:
                pGraph->setData(x,y);
                pGraph->setPen(QPen(Qt::blue));
                pGraph1->setData(x,y1);
                pGraph1->setPen(QPen(Qt::yellow));
                       break;
                case 3:
                pGraph->setData(x,y);
                pGraph->setPen(QPen(Qt::blue));
                pGraph1->setData(x,y1);
                pGraph1->setPen(QPen(Qt::yellow));
                pGraph2->setData(x,y2);
                pGraph2->setPen(QPen(Qt::red));
                       break;

                case 4:
                pGraph->setData(x,y);
                pGraph->setPen(QPen(Qt::blue));
                pGraph1->setData(x,y1);
                pGraph1->setPen(QPen(Qt::yellow));
                pGraph2->setData(x,y2);
                pGraph2->setPen(QPen(Qt::red));
                pGraph3->setData(x,y3);
                pGraph3->setPen(QPen(Qt::black));
                       break;
                }
                ui->customPlot->replot();
                //计数器，实现动态画线
               i++;
            }
        }
        ui->textBrowser->append("data"+id+":"+data_temp);
        data_temp.clear();//清除残留
}

/**
 * @function  返回选择的曲线条数
 * @param     void
 * @return    int
 * @version   2019.12.13
 * @author    占建
 * @situation finish
 */
void MainWindow:: onRadio()
{
    switch(RGroup->checkedId())
      {
      case 0:
          char_id=1;
          break;
      case 1:
        char_id=2;
        break;
      case 2:
        char_id=3;
        break;
      case 3:
        char_id=4;
         break;
      default:
                break;
      }
//    qDebug()<<RGroup->checkedId();
}

/**
 * @function  进入正常接收模式，不做任何校验区别于绘图模式的校验
 * @param     void
 * @return    void
 * @version   2019.12.15
 * @author    占建
 * @situation finish
 */
void MainWindow:: recevie()
{
    //退出绘图模式
    flage=0;
    ui->showChar->setText("绘制图像");
    //进入正常接收模式
    re_statu=true;

}

/**
 * @function  换肤尝试
 * @param     void
 * @return    void
 * @version   2019.12.15
 * @author    占建
 * @situation finish
 */
void MainWindow:: change_pf_1()
{
       setStyle(":/qss/uistyle2.qss");//加载样式表
}

void MainWindow:: change_pf_2()
{
          setStyle(":/qss/uiStyle.qss");
}
