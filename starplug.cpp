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
    m_comboBoxList<<"打开首页"<<"登录"<<"执行"<<"执行带结果";
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

    connect(this,SIGNAL(haveAResultSignal(QString)),this,SLOT(haveAResultSlot(QString)));
}

starPlug::~starPlug()
{
    delete ui;
}

void starPlug::on_pushButton_clicked()
{
    switch (ui->comboBox->currentIndex()) {
    case 0://打开首页
        autoRun(autoRunMain);
        break;
    case 1://登录        
        autoRun(autoRunLogin);
        break;
    case 2:
        autoRun(execLineEdit);
        break;
    case 3:
        autoRun(execLineEditWithResult);
        break;
    default:
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

    switch (autoRunIndex) {
    case autoRunLogin:
        autoRun(autoRunNum);
        break;
    case autoRunOpenClass:
        autoRun(autoRunClassOpened);
        break;
    default:
        break;
    }
}

void starPlug::view_loadFinished(bool b)
{
    qDebug()<<b<<" num_loadFinished "<<autoRunIndex;
    if(QObject::sender()==m_WebViewNum){
        switch (autoRunIndex) {
        case autoRunNum:
            autoRunIndex = autoRunBindQQ;
            waitTimer();
            break;
        case autoRunReLoad:
            autoRunIndex = autoRunClose;
            waitTimer(5*1000);
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewNum default";
            break;
        }
    }else if(QObject::sender()==m_WebViewClass){
        switch (autoRunIndex) {
        case autoRunClassOpened:
            autoRunIndex = autoRunSetTimer;
            waitTimer();
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewClass default";
            break;
        }
    }else{
        qDebug()<<"QObject::sender()?????";
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

void starPlug::waitTimer(int t)
{
    qDebug()<<"waitTimer";
    m_timer->start(t);
}

void starPlug::autoRun()
{
    qDebug()<<"autoRun()";
    m_timer->stop();
    autoRun(autoRunIndex);
}

void starPlug::haveAResultSlot(const QString &s)
{
    runJavaScriptResult.clear();
    runJavaScriptResult.append(s);
    qDebug()<<"haveAResultSlot "<<runJavaScriptResult;
}

void starPlug::autoRun(int index)
{
    autoRunIndex = index;
    qDebug()<<"in autoRun: "<<index;
    bool toNext(false),needTimer(false);
    int tabCount = m_TabWindow->count();
    switch (index) {
    case autoRunMain://打开首页
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);//关闭所有页面，最后一个页面被关闭后会新建一个空页面
        m_browserWindow->loadPage("http://www.faxuan.net/site/hubei/");//
        m_loginWebView = m_TabWindow->currentWebView();
        connect(m_loginWebView,SIGNAL(loadFinished(bool)),this,SLOT(login_loadFinished(bool)));
        toNext = false;needTimer = false;break;
    case autoRunLogin://登录
        connect(m_TabWindow,SIGNAL(newTabCreated()),this,SLOT(newTabViewCreated()));//1-2
        m_loginWebView->page()->runJavaScript(QString("$('#user_name').val('%1')").arg(getCurrentName(currentNameIndex)));
        m_loginWebView->page()->runJavaScript(QString("$('#user_pass').val('%1')").arg(getCurrentPassword(currentNameIndex)));
        m_loginWebView->page()->runJavaScript("$('.login_button').click()");
        toNext = false;needTimer = false;break;//到newTabViewCreated中处理autoRun(autoRunNum1);
    case autoRunNum://分数界面。newTabViewCreated自动调用
        //首页："湖北省国家工作人员学法用法考试平台_法宣在线"，分数界面：“法宣在线”
        qDebug()<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
        if( m_TabWindow->count()!=2 || m_TabWindow->currentWebView()->title()!=(QString("法宣在线"))){
            qDebug()<<"autoRunNum autoRunIndex--  "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
            toNext = false;needTimer = true;
        }else{
            //num_loadFinished中waitTimer,超时后下一步
            m_WebViewNum = m_TabWindow->currentWebView();
            connect(m_WebViewNum,SIGNAL(loadFinished(bool)),this,SLOT(view_loadFinished(bool)));
            qDebug()<<"connect num_loadFinished";
            toNext = false;needTimer = false;
        }
        break;
    case autoRunBindQQ://打开计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("bps.remCook('bindQQ')");
        qDebug()<<"bindQQ";
        toNext = true;needTimer = true;break;
    case autoRunOpenClass://打开计时界面
        //m_TabWindow->currentWebView()->page()->runJavaScript("location.href = $(\"[href$='考试复习指南（2017）&d=AA==']:first\").attr('href')");
        m_WebViewNum->page()->runJavaScript("window.open($(\"[href$='考试复习指南（2017）&d=AA==']:first\").attr('href'))");
        qDebug()<<"href";
        toNext = false;needTimer = false;break;//到newTabViewCreated中处理autoRun(autoRunClassOpened);
        break;
    case autoRunClassOpened:
        qDebug()<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
        if( m_TabWindow->count()!=3 || m_TabWindow->currentWebView()->title()!=(QString("课程"))){
            qDebug()<<"autoRunClassOpened autoRunIndex--  "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
            toNext = false;needTimer = true;
        }else{
            //num_loadFinished中waitTimer,超时后下一步
            m_WebViewClass = m_TabWindow->currentWebView();
            connect(m_WebViewClass,SIGNAL(loadFinished(bool)),this,SLOT(view_loadFinished(bool)));
            qDebug()<<"connect m_WebViewClass view_loadFinished";
            toNext = false;needTimer = false;
        }
        break;
    case autoRunSetTimer://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("sps.startTiming('timer',10000)");
        qDebug()<<"startTiming";
        toNext = true;needTimer = true;break;
    case autoRunExitTimer://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("sps.exitStudy('timer')");
        qDebug()<<"exitStudy";
        toNext = true;needTimer = true;break;
    case autoRunPopwinConfirm://计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("$('#popwinConfirm').click()");
        qDebug()<<"popwinConfirm";
        toNext = true;needTimer = true;break;
    case autoRunReLoad:
        if(m_TabWindow->currentWebView()==m_WebViewNum){
            m_WebViewNum->reload();
        }
        toNext = false;needTimer = false;//num_loadFinished中处理
        break;
    case autoRunClose:
        m_TabWindow->closeTab(m_TabWindow->currentIndex());
        toNext = false;needTimer = false;break;
    case execLineEdit:
        qDebug()<<ui->lineEdit->text();
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text());
        toNext = false;needTimer = false;break;
    case execLineEditWithResult:
        qDebug()<<ui->lineEdit->text();
        //网上例子ui.myWebView->page()->toHtml([textEdit](const QString &result){ textEdit->setHtml(result); });
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text(),[this](const QVariant& r){emit haveAResultSignal(r.toString());});
        //qDebug()<<"execLineEditWithResult "<<runJavaScriptResult;
        //成功m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text(),[](const QVariant& r){qDebug()<<r;});
        toNext = false;needTimer = false;break;
    default:
        toNext = false;needTimer = false;break;
    }

    if(needTimer){
        waitTimer();
    }/*else{//超时槽函数有stop
        m_timer->stop();
    }*/
    if(toNext){
        autoRunIndex++;
    }

    qDebug()<<"out autoRun:"<<autoRunIndex;
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
