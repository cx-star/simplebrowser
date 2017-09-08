#include "starplug.h"
#include "ui_starplug.h"
#include "tabwidget.h"
#include <QDebug>
#include <QFile>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QMessageBox>

starPlug::starPlug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::starPlug)
{
    ui->setupUi(this);
    m_comboBoxList<<"打开首页"<<"登录"<<"执行";
    ui->comboBox->addItems(m_comboBoxList);
    autoRunIndex = 0;

//    QFile file;
//    file.setFileName(":/js/q.js");
//    file.open(QIODevice::ReadOnly);
//    jQuery = file.readAll();
//    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
//    file.close();

    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(autoRun()));
}

starPlug::~starPlug()
{
    delete ui;
}

void starPlug::on_pushButton_clicked()
{
    switch (ui->comboBox->currentIndex()) {
    case 0://打开首页
        autoRun(0);
        break;
    case 1://登录        
        autoRun(1);
        break;
    default:
        autoRun(10);
        break;
    }
}

void starPlug::login_loadFinished(bool b)
{
    //m_loginWebWiew->page()->runJavaScript(jQuery);
    QMessageBox::about(this,"输入验证码",QString("主页载入：%1\r\n请在网页正确位置输入验证码\r\n（仅输入验证么即可）").arg(b));
}

void starPlug::newTabViewCreated()
{
    qDebug()<<"newTabViewCreated"<<autoRunIndex;
    if(autoRunIndex==2){
        autoRun(2);//绑定main_loadFinished
    }
    else{
        QMessageBox::warning(this,"错误",QString("newTabViewCreated %1").arg(autoRunIndex));
    }
}

void starPlug::main_loadFinished(bool b)
{
    qDebug()<<b<<" main_loadFinished "<<autoRunIndex;
    if(autoRunIndex==3||autoRunIndex==4||autoRunIndex==5){
        waitTimer();
    }
    else{
        QMessageBox::warning(this,"错误",QString("main_loadFinished %1").arg(autoRunIndex));
    }
}

QString starPlug::getCurrentName(int ci)
{
    int t = ci;
    t++;
    return QString("4213810430098");
}

QString starPlug::getCurrentPassword(int ci)
{
    int t = ci;
    t++;
    return QString("cx153719");
}

void starPlug::waitTimer()
{
    qDebug()<<"waitTimer";
    m_timer->start(3*1000);
}

void starPlug::autoRun(int index)
{
    qDebug()<<"in autoRun: "<<index;
    int tabCount = m_TabWindow->count();
    switch (index) {
    case 0://打开首页
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);
        m_browserWindow->loadPage("http://www.faxuan.net/site/hubei/");
        m_loginWebWiew = m_TabWindow->currentWebView();

        connect(m_loginWebWiew,SIGNAL(loadFinished(bool)),this,SLOT(login_loadFinished(bool)));
        break;
    case 1://登录
        connect(m_TabWindow,SIGNAL(newTabCreated()),this,SLOT(newTabViewCreated()));//1-2
        m_loginWebWiew->page()->runJavaScript(QString("$('#user_name').val('%1')").arg(getCurrentName(currentNameIndex)));
        m_loginWebWiew->page()->runJavaScript(QString("$('#user_pass').val('%1')").arg(getCurrentPassword(currentNameIndex)));
        m_loginWebWiew->page()->runJavaScript("$('.login_button').click()");
        break;
    case 2://分数界面。newTabViewCreated自动调用
        if(m_TabWindow->count()==2&&m_TabWindow->currentWebView()->title().contains("宣法在线")){
            connect(m_TabWindow->currentWebView(),SIGNAL(loadFinished(bool)),this,SLOT(main_loadFinished(bool)));//2-3
            qDebug()<<"connect main_loadFinished";
        }else{
            autoRunIndex--;//原地踏步
            qDebug()<<"autoRunIndex--";
            //QMessageBox::warning(this,"分数界面错误",QString("m_TabWindow->count %1 %2").arg(m_TabWindow->count()).arg(m_TabWindow->currentWebView()->title()));
        }
        waitTimer();
        break;
    case 3://打开计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("bps.remCook('bindQQ')");
        qDebug()<<"bindQQ";
        waitTimer();
        //上个case已经connect,3-4
        break;
    case 4://打开计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("location.href = $(\"[href$='考试复习指南（2017）&d=AA==']:first\").attr('href')");
        qDebug()<<"href";
        waitTimer();
        //上个case已经connect,3-4
        break;
    case 5://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("sps.startTiming('timer',10000)");
        qDebug()<<"startTiming";
        waitTimer();
        break;
    case 6://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("sps.exitStudy('timer')");
        qDebug()<<"exitStudy";
        waitTimer();
        break;
    case 7://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("$('#popwinConfirm').click()");
        qDebug()<<"popwinConfirm";
        waitTimer();
        break;
    default:
        qDebug()<<ui->lineEdit->text();
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text());
        break;
    }
    autoRunIndex++;
    qDebug()<<"out autoRun:"<<autoRunIndex;
}

void starPlug::autoRun()
{
    qDebug()<<"autoRun()";
    autoRun(autoRunIndex);
    m_timer->stop();
}

BrowserWindow *starPlug::browserWindow() const
{
    return m_browserWindow;
}

void starPlug::setBrowserWindow(BrowserWindow *browserWindow)
{
    m_browserWindow = browserWindow;
}

TabWidget *starPlug::TabWindow() const
{
    return m_TabWindow;
}

void starPlug::setTabWindow(TabWidget *TabWindow)
{
    m_TabWindow = TabWindow;
}

/*
        qDebug()<<"cachePath "<<m_loginWebWiew->page()->profile()->cachePath();
        qDebug()<<"httpCacheMaximumSize "<<m_loginWebWiew->page()->profile()->httpCacheMaximumSize();
        qDebug()<<"httpCacheType "<<m_loginWebWiew->page()->profile()->httpCacheType();
        qDebug()<<"persistentStoragePath "<<m_loginWebWiew->page()->profile()->persistentStoragePath();
        qDebug()<<"storageName "<<m_loginWebWiew->page()->profile()->storageName();

cachePath  "C:/Users/cx/AppData/Local/simplebrowser/cache/QtWebEngine/Default"
httpCacheMaximumSize  0
httpCacheType  QWebEngineProfile::HttpCacheType(DiskHttpCache)
persistentStoragePath  "C:/Users/cx/AppData/Local/simplebrowser/QtWebEngine/Default"
storageName  "Default"
 */

void starPlug::on_pushButtonLogin_clicked()
{
    autoRunIndex=1;
    autoRun(autoRunIndex);
}
