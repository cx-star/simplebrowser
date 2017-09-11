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
    QFile file;
    file.setFileName("userPassword.txt");
    file.open(QIODevice::ReadOnly);
    userInfoAll = QString(file.readAll()).split("\r\n");
    file.close();
    userInfoListIndex=0;
    qDebug()<<userInfoAll;

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

void starPlug::newTabViewCreated()
{
    qDebug()<<"newTabViewCreated"<<autoRunIndex;
    switch (autoRunIndex) {
    case autoRunLogin:
        autoRun(autoRunNum);
        break;
    case autoRunReLogin://
        autoRun(autoRunReNum);
        break;
    case autoRunOpenClass:
        autoRun(autoRunClassOpened);
        break;
    default:
        qDebug()<<"newTabViewCreated default"<<autoRunIndex;
        break;
    }
}

void starPlug::view_loadFinished(bool b)
{
    qDebug()<<b<<" num_loadFinished "<<autoRunIndex;
    if(QObject::sender()==m_WebViewLogin){//首次登陆；验证总分数登陆；下一个人登陆
        switch (autoRunIndex) {
        case autoRunMain:
            QMessageBox::about(this,"输入验证码",QString("主页载入：%1\r\n请在网页正确位置输入验证码\r\n（仅输入验证么即可）").arg(b));
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewLogin default"<<autoRunIndex;
            break;
        }
    }else if(QObject::sender()==m_WebViewNum){
        switch (autoRunIndex) {
        case autoRunNum:
            autoRunIndex = autoRunBindQQ;
            waitTimer();
            break;
        case autoRunReNum:
            autoRunIndex = autoRunReNumReload;
            waitTimer();
            break;
        case autoRunReNumReload:
            autoRunIndex = autoRunGetResult;
            waitTimer();
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewNum default"<<autoRunIndex;
            break;
        }
    }else if(QObject::sender()==m_WebViewClass){
        switch (autoRunIndex) {
        case autoRunClassOpened:
            autoRunIndex = autoRunSetTimer;
            waitTimer();
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewClass default"<<autoRunIndex;
            break;
        }
    }else{
        qDebug()<<"QObject::sender()?????";
    }
}

QString starPlug::getCurrentName()
{
    if(userInfoListIndex<userInfoAll.size()){
        QString s = userInfoAll.at(userInfoListIndex);
        userInfoCur = s.split("\t");
        if(userInfoCur.size()==3)
            return userInfoCur.at(1);
    }
    userInfoListIndex = -1;
    return QString();
}

QString starPlug::getCurrentPassword()
{
    if(userInfoListIndex!=-1)
        return userInfoCur.at(2);
    return QString();
}

void starPlug::waitTimer(int t)
{
    qDebug()<<"waitTimer";
    m_timer->start(t);
}

void starPlug::waitViewNum(bool &toNext, bool &needTimer)
{
    qDebug()<<"waitViewNum "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
    //首页："湖北省国家工作人员学法用法考试平台_法宣在线"，分数界面：“法宣在线”
    if( m_TabWindow->count()!=2 || m_TabWindow->currentWebView()->title()!=(QString("法宣在线"))){
        qDebug()<<"autoRunNum autoRunIndex--  "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
        toNext = false;needTimer = true;
    }else{
        //num_loadFinished中waitTimer,超时后下一步
        m_WebViewNum = m_TabWindow->currentWebView();
        connect(m_WebViewNum,SIGNAL(loadFinished(bool)),this,SLOT(view_loadFinished(bool)));
        qDebug()<<"connect view_loadFinished";
        toNext = false;needTimer = false;
    }
}

int starPlug::getTodayResult()
{
    QString runJavaScriptResult;
    QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
    m_WebViewNum->page()->runJavaScript("$('h1').attr('title')+'|'+$('#todypoint').text()+'|'+$('#todaytpoint').text()",[loop,&runJavaScriptResult] (const QVariant& r){
        if(loop->isRunning()){
            runJavaScriptResult = r.toString();
            loop->quit();
        }
    });
    loop->exec();
    int i = runJavaScriptResult.indexOf("：");//8
    int j = runJavaScriptResult.indexOf("|",i);//12
    int r = runJavaScriptResult.mid(i+1,j-i-1).toInt();
    if(r>=110){
        autoRunIndex = autoRunNext-1;
        runJavaScriptResult.replace("|","\t");
        userStudyResult.append(runJavaScriptResult);
        userInfoListIndex++;
        m_WebViewNum->disconnect();
        m_TabWindow->closeTab(m_TabWindow->currentIndex());
    }
    qDebug()<<r<<" "<<runJavaScriptResult;//"胡伦轩|今日积分：110|总?积?分：110"
    return r;
}

void starPlug::autoRun()
{
    qDebug()<<"autoRun()";
    m_timer->stop();
    autoRun(autoRunIndex);
}

void starPlug::haveAResultSlot(const QString &s)
{
    mm_runJavaScriptResult.clear();
    mm_runJavaScriptResult.append(s);
    qDebug()<<"haveAResultSlot "<<mm_runJavaScriptResult;
}

void starPlug::autoRun(int index)
{
    qDebug()<<"in autoRun: "<<index;
    autoRunIndex = index;
    bool toNext(false),needTimer(false);
    int tabCount = m_TabWindow->count();

    switch (index) {
    case autoRunMain://打开首页
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);//关闭所有页面，最后一个页面被关闭后会新建一个空页面
        m_browserWindow->loadPage("http://www.faxuan.net/site/hubei/");//
        m_WebViewLogin = m_TabWindow->currentWebView();
        connect(m_WebViewLogin,SIGNAL(loadFinished(bool)),this,SLOT(view_loadFinished(bool)));
        connect(m_TabWindow,SIGNAL(newTabCreated()),this,SLOT(newTabViewCreated()));//1-2
        toNext = false;needTimer = false;break;
    case autoRunLogin://登录
        m_WebViewLogin->page()->runJavaScript(QString("$('#user_name').val('%1')").arg(getCurrentName()));
        m_WebViewLogin->page()->runJavaScript(QString("$('#user_pass').val('%1')").arg(getCurrentPassword()));
        if(userInfoListIndex!=-1){
            m_WebViewLogin->page()->runJavaScript("$('.login_button').click()");
        }
        else{
            QMessageBox::about(this,"学习结果",userStudyResult.join("\r\n"));//由此位置结束学习
        }
        toNext = false;needTimer = false;break;//到newTabViewCreated中处理autoRun(autoRunNum1);
    case autoRunNum://分数界面。newTabViewCreated自动调用
        waitViewNum(toNext,needTimer);
        //未成功toNext = false;needTimer = true;
        //  成功toNext = false;needTimer = false;connect view_loadFinished
        break;
    case autoRunBindQQ://打开计时界面
        if(getTodayResult()<110){
            m_TabWindow->currentWebView()->page()->runJavaScript("bps.remCook('bindQQ')");
            qDebug()<<"bindQQ";
        }
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
            m_WebViewClass->reload();//网速太快时，来不及connect……
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
    case autoRunPopwinConfirm://计时界面,关闭了当前学习计时界面
        m_WebViewClass->disconnect();
        m_TabWindow->currentWebView()->page()->runJavaScript("$('#popwinConfirm').click()");
        qDebug()<<"popwinConfirm";
        toNext = true;needTimer = true;break;
    case autoRunCloseNumForReload:
        if(m_TabWindow->currentWebView()==m_WebViewNum){
            m_WebViewNum->disconnect();
            m_TabWindow->closeTab(m_TabWindow->currentIndex());
            toNext = true;needTimer = true;
        }else{
            toNext = false;needTimer = true;
            qDebug()<<"autoRunCloseNumForReload 关闭失败";
        }
        break;
    case autoRunReLogin:
        if(m_TabWindow->currentWebView()==m_WebViewLogin){
            m_WebViewLogin->page()->runJavaScript("$('.login_button').click()");
        }
        toNext = false;needTimer = false;//newTabViewCreated中处理,获取学习总结果
        break;
    case autoRunReNum:        //第二次登陆
        waitViewNum(toNext,needTimer);
        //未成功toNext = false;needTimer = true;
        //  成功toNext = false;needTimer = false;connect view_loadFinished
        break;
    case autoRunReNumReload:
        m_WebViewNum->reload();
        toNext = false;needTimer = false;break;
    case autoRunGetResult:
        getTodayResult();
        toNext = true;needTimer = true;break;
    case autoRunNext:
    {
        bool b= ui->checkBox->isChecked();
        if(b){
            toNext = b;
            autoRunIndex=autoRunLogin-1;//经过后面++后，为autoRunLogin
        }
    }
        needTimer = true;break;
    case execLineEdit:
        qDebug()<<"execLineEdit "<<ui->lineEdit->text();
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text());
        toNext = false;needTimer = false;break;
    case execLineEditWithResult:
        qDebug()<<"execLineEditWithResult "<<ui->lineEdit->text();
        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text(),[this](const QVariant& r){emit haveAResultSignal(r.toString());});
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

void starPlug::on_pushButtonLogin_clicked()
{
    autoRunIndex=1;
    autoRun(autoRunIndex);
}

/*
//page的profile相关设置
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

//runJavaScript异步问题
//    {
//        QString runJavaScriptResult;
//        QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
//        m_TabWindow->currentWebView()->page()->runJavaScript(ui->lineEdit->text(),[loop,&runJavaScriptResult] (const QVariant& r){
//            if(loop->isRunning()){
//                runJavaScriptResult = r.toString();
//                loop->quit();
//            }
//        });
//        loop->exec();
//        runJavaScriptResult2 = runJavaScriptResult;
//    }


//QString runJavaScriptResult;
//QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
//m_WebViewLogin->page()->runJavaScript("$('.testword_input').val()",[loop,&runJavaScriptResult] (const QVariant& r){
//    if(loop->isRunning()){
//        runJavaScriptResult = r.toString();
//        loop->quit();
//    }
//});
//loop->exec();
//if(runJavaScriptResult.isEmpty()){//首次登陆，未输入验证码
//    QMessageBox::about(this,"输入验证码",QString("主页载入：%1\r\n请在网页正确位置输入验证码\r\n（仅输入验证么即可）").arg(b));
//}
