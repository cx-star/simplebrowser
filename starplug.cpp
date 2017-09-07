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
}

starPlug::~starPlug()
{
    delete ui;
}

void starPlug::on_pushButton_clicked()
{
    int tabCount = m_TabWindow->count();
    switch (ui->comboBox->currentIndex()) {
    case 0://打开首页
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);
        m_browserWindow->loadPage("http://www.faxuan.net/site/hubei/");
        m_loginWebWiew = m_TabWindow->currentWebView();
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
        connect(m_loginWebWiew,SIGNAL(loadFinished(bool))
                ,this,SLOT(login_loadFinished(bool)));
        break;
    case 1://登录
        m_loginWebWiew->page()->runJavaScript(QString("$('#user_name').val('%1')").arg(getCurrentName(currentNameIndex)));
        m_loginWebWiew->page()->runJavaScript(QString("$('#user_pass').val('%1')").arg(getCurrentPassword(currentNameIndex)));
        m_loginWebWiew->page()->runJavaScript("$('.login_button').click()");
        break;
    default:
        //bps.remCook('bindQQ')
        //location.href = $("[href$='考试复习指南（2017）&d=AA==']:first").attr('href')
        //sps.startTiming("timer",10000)
        //javascript:sps.exitStudy('timer')
        //$('#popwinConfirm').click()
        qDebug()<<ui->lineEdit->text();
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text());
        break;
    }
}

void starPlug::login_loadFinished(bool b)
{
    //m_loginWebWiew->page()->runJavaScript(jQuery);
    QMessageBox::about(this,"输入验证码","请在网页正确位置输入验证码（仅输入验证么即可）");
}

const QString &starPlug::getCurrentName(int ci)
{
    return QString("4213810430098");
}

const QString &starPlug::getCurrentPassword(int ci)
{
    return QString("cx153719");
}

void starPlug::autoRun(int index)
{
    switch(index){
    case 0:
        break;
    case 1:
        break;
    default:
    }
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

