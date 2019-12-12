#include "starplug.h"
#include "stardialogresult.h"
#include "ui_starplug.h"
#include "tabwidget.h"
#include <QDebug>
#include <QFile>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QMessageBox>
#include <QTextCodec>
#include <QWebEngineCookieStore>
#include <QtNetwork>
#include <QLabel>
#include <QPicture>

starPlug::starPlug(TabWidget*tw, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::starPlug),
    m_TabWindow(tw)
{
    ui->setupUi(this);

    errorNeedReboot = false;

    m_comboBoxList<<"自动"<<"登录"<<"执行"<<"执行带结果"<<"dealExam"<<"填入star"<<"save page";
    ui->comboBox->addItems(m_comboBoxList);
    autoRunIndex = 0;

    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(autoRun()));

    connect(this,SIGNAL(haveAResultSignal(QString)),this,SLOT(haveAResultSlot(QString)));

    m_setting = new QSettings(QCoreApplication::applicationDirPath()+"/setting.ini", QSettings::IniFormat);
    m_setting->setIniCodec(QTextCodec::codecForName("UTF-8"));
    qDebug()<<QCoreApplication::applicationDirPath();

    foreach (QString s, m_setting->childGroups()) {
        if(s.startsWith("421381043")){
            m_setting->beginGroup(s);
            if(m_setting->value("isVaild").toBool()){
                if( (m_setting->value("Taday").toString()==QDate::currentDate().toString("yy-MM-dd")
                     && m_setting->value("TadayNum").toInt()==150)
                        || m_setting->value("TotalNum").toInt()>2400){
                    qDebug()<<QString("已学满:%1 %2").arg(m_setting->value("name").toString()).arg(s);
                }else{
                    QString t = QString("%1\t%2\t%3\t%4\t%5")
                            .arg(m_setting->value("name").toString())
                            .arg(s)
                            .arg(m_setting->value("pw").toString())
                            .arg(m_setting->value("TotalNum").toString())
                            .arg(m_setting->value("OldNum").toString());
                    userInfoAll.append(t);
                }
            }
            m_setting->endGroup();
        }
    }
    //qDebug()<<userInfoAll;

    userInfoListIndex = 0;

    connect(m_TabWindow,SIGNAL(urlChanged(QUrl)),this,SLOT(slot_tabUrlChanged(QUrl)));

    QFile file;
    file.setFileName(QCoreApplication::applicationDirPath()+"/exam.dat");
    file.open(QIODevice::ReadOnly);
    if(file.isOpen()){
        QDataStream in(&file);
        in>>m_exam;
        file.close();
    }
    ui->pushButtonExam->setText(QString("%1").arg(m_exam.size()));

    QFile file2;
    file2.setFileName(QCoreApplication::applicationDirPath()+"/oneChar.dat");
    file2.open(QIODevice::ReadOnly);
    if(file2.isOpen()){
        QDataStream in(&file2);
        in>>oneCharDataMap;
        file2.close();
    }

    m_dialogExam = new DialogExam();
    connect(this,SIGNAL(signal_examDataChanged(QMap<QString,QStringList>)),m_dialogExam,SLOT(updateExamData(QMap<QString,QStringList>)));
    emit signal_examDataChanged(m_exam);
    connect(this,SIGNAL(signal_doExamCurTiMuId(QString)),m_dialogExam,SLOT(setComboBoxString(QString)));
    //m_dialogExam->show();

    net_manager = new QNetworkAccessManager(m_TabWindow);//跟网页不是同一个session
    connect(net_manager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(replyFinished(QNetworkReply*)));

    m_WebViewLogin=m_WebViewNum=m_WebViewClass=m_WebViewExam=0;
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
    case 4:
    {
        dealExamAnswer();
        //QRegExp
    }
        break;
    case 5:
    {
        m_TabWindow->currentWebView()->page()->runJavaScript(QString("$('#userAccount').val('%1')").arg("4213810430098"));
        m_TabWindow->currentWebView()->page()->runJavaScript(QString("$('#userPassword').val('%1')").arg("cx153719"));
        break;
    }
    case 6:
    {
        QWebEnginePage *page = m_TabWindow->currentWebView()->page();
        QAction *a = page->action(QWebEnginePage::SavePage);
        a->activate(QAction::Trigger);
    }
        break;
    default:
        break;
    }
}

void starPlug::slot_newTabViewCreated()
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

void starPlug::slot_viewLoadFinished(bool b)
{
    if(!b){
        m_TabWindow->currentWebView()->reload();
        qDebug()<<"view_loadFinished false!!!";
        return;
    }
    qDebug()<<b<<" view_loadFinished "<<autoRunIndex;
    if(QObject::sender()==m_WebViewLogin){//首次登陆；验证总分数登陆；下一个人登陆
        switch (autoRunIndex) {
        case autoRunCheckCaptchaAndJquery:
        {
            qDebug()<<"载入成功，即将下载并OCR";
            m_captchaWaitTimes = 0;//等待OCR，防止autoRun退回（刷新）
            m_WebViewLogin->page()->profile()->disconnect();
            connect(m_WebViewLogin->page()->profile(), SIGNAL(downloadRequested(QWebEngineDownloadItem*)),
                    this, SLOT(downloadRequested(QWebEngineDownloadItem*)));
            QWebEnginePage *page = m_TabWindow->currentWebView()->page();
            QAction *a = page->action(QWebEnginePage::SavePage);
            a->activate(QAction::Trigger);
        }
            break;
        default:
            qDebug()<<"QObject::sender()==m_WebViewLogin default"<<autoRunIndex;
            break;
        }
    }else if(QObject::sender()==m_WebViewNum){
        switch (autoRunIndex) {
        case autoRunNum:
            autoRunIndex = autoRunNum+1;
            waitTimer();
            break;
        case autoRunReNum:
            autoRunIndex = autoRunReNum+1;
            waitTimer();
            break;
        case autoRunReNumReload:
            autoRunIndex = autoRunReNumReload+1;
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
    }else if(QObject::sender()==m_WebViewExam){
        if(dealExam3over && m_WebViewExam->url().toString().contains("exercies_3_t")){//刷题时的做题
            QTimer::singleShot(3*1000,this,SLOT(commitExam()));
            dealExam3over = false;//不再次进入
            qDebug()<<"m_WebViewExam 3:"<<m_WebViewExam->url().toString();
        }else if(dealExam4over && m_WebViewExam->url().toString().contains("exercies_4_t")){//答案
            QTimer::singleShot(3*1000,this,SLOT(dealExamAnswer()));
            dealExam4over = false;//不再次进入
            qDebug()<<"m_WebViewExam 4:"<<m_WebViewExam->url().toString();
        }else if(doExamMode3 && m_WebViewExam->url().toString().contains("exercies_3_t")){//做题
            QTimer::singleShot(3*1000,this,SLOT(doExam()));
            doExamMode3 = false;//不再次进入
            qDebug()<<"m_WebViewExam doExamMode 3:"<<m_WebViewExam->url().toString();
        }else if(doExamMode4 && m_WebViewExam->url().toString().contains("exercies_4_t")){//做题时答案
            autoRunIndex = autoRunDoExamOver;
            QTimer::singleShot(3*1000,this,SLOT(autoRun()));
            doExamMode4 = false;//不再次进入
            qDebug()<<"m_WebViewExam doExamMode 4:"<<m_WebViewExam->url().toString();
        }
    }else{
        qDebug()<<"QObject::sender()?????";
    }
}

QString starPlug::getCurrentUserId()
{
    if(!errorNeedReboot){
        if(userInfoListIndex ==-1){//重新开始遍历账号
            userInfoListIndex = 0;
        }
        ui->pushButtonLogin->setText(QString("登录%1/%2").arg(userInfoListIndex+1).arg(userInfoAll.size()));
        while(userInfoListIndex<userInfoAll.size()){//返回一个不已*开头的账号（改ini后，不存在这种情况）
            QString s = userInfoAll.at(userInfoListIndex);
            if(!s.startsWith('*')){
                userInfoCur = s.split("\t");
                if(userInfoCur.size()>=3)
                    return userInfoCur.at(1);
            }else{
                qDebug()<<"getCurrentName:pass "<<s;
            }
            userInfoListIndex++;
            continue;
        }
    }
    userInfoListIndex = -1;//账号全部遍历完毕
    return QString();
}

QString starPlug::getCurrentPassword()
{
    if(userInfoListIndex!=-1)
        return userInfoCur.at(2);
    return QString();
}

int starPlug::getIniTotalNum()
{
    if(userInfoCur.size()>=4){
        QString s = userInfoCur.at(3);
        return s.toInt();
    }
    return 0;//0为未获取，-1未密码错误
}

int starPlug::getCurrentOldNum()
{
    if(userInfoCur.size()>=5){
        QString s = userInfoCur.at(4);
        return s.toInt();
    }
    return 0;
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
    if(m_TabWindow->count()==1 || m_TabWindow->count()==3){//登录错误，没有弹出新页面
        autoRunIndex = autoRunLogin-1;
        userStudyResult.append(getCurrentUserId());//记录错误账号
        userInfoListIndex++;
        toNext = true;needTimer = true;
    }else if(m_TabWindow->currentWebView()->title()==QString("国家工作人员学法用法及考试平台_登录")
             || m_TabWindow->currentWebView()->title()==QString("400 Request Header Or Cookie Too Large")){
        //登录超时。关闭重新登录
        m_TabWindow->closeTab(m_TabWindow->currentIndex());
        autoRunIndex =  autoRunLogin-1;
        errorNeedReboot = true;
        toNext = true;needTimer = true;
    }else if( m_TabWindow->currentWebView()->title()!=(QString("法宣在线"))){
        qDebug()<<"autoRunNum autoRunIndex--  "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
        toNext = false;needTimer = true;
    }else{//两个tab，且第二个为“法宣在线”
        m_WebViewNum = m_TabWindow->currentWebView();
        connect(m_WebViewNum,SIGNAL(loadFinished(bool)),this,SLOT(slot_viewLoadFinished(bool)));
        qDebug()<<"connect view_loadFinished";
        toNext = false;needTimer = false;
    }
}

void starPlug::getTodayResult(int* todayNum,int* TotalNum, QString* curName)
{
    qDebug()<<"getTodayResult";
    QString runJavaScriptResult;//"胡伦轩|今日积分：110|总?积?分：110"
    QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
    m_WebViewNum->page()->runJavaScript("$('h1').attr('title')+'|'+$('#todypoint').text()+'|'+$('#todaytpoint').text()",[loop,&runJavaScriptResult] (const QVariant& r){
        if(loop->isRunning()){
            runJavaScriptResult = r.toString();
            loop->quit();
        }
    });
    loop->exec();
    int j = runJavaScriptResult.indexOf("|");
    QString m_curName = runJavaScriptResult.mid(0,j);
    int i = runJavaScriptResult.indexOf("：");//8
    j = runJavaScriptResult.indexOf("|",i);//12
    int m_todayNum = runJavaScriptResult.mid(i+1,j-i-1).toInt();
    i = runJavaScriptResult.indexOf("：",j);
    int m_TotalNum = runJavaScriptResult.mid(i+1).toInt();

    if(todayNum!=0)
        *todayNum = m_todayNum;
    if(TotalNum!=0)
        *TotalNum = m_TotalNum;
    if(curName!=0)
        *curName = m_curName;
}

void starPlug::dealExamAnswer()//获取答案
{
    QString html;
    QWebEnginePage *page = m_TabWindow->currentWebView()->page();
    QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
    page->toHtml([loop,&html] (const QVariant& r){
        if(loop->isRunning()){
            html = r.toString();
            loop->quit();
        }
    });
    loop->exec();

    QRegExp tm("<h3>(.*)</h3>");
    QRegExp daAll("<ul(.*)</ul>");
    QRegExp daOne("<li>.*\">(.*)</li>");
    tm.setMinimal(true);
    daAll.setMinimal(true);
    daOne.setMinimal(true);
    int pos(0),posDAStart(0),posDAEnd(0);
    int newAdd(0);
    while( (pos = tm.indexIn(html,pos)) != -1 ){
        QStringList TiXuanDa;
        TiXuanDa.append(tm.cap(1));//题目
        posDAStart = daAll.indexIn(html,pos);
        posDAEnd = daAll.matchedLength()+posDAStart;
        //qDebug()<<posDAStart<<" "<<posDAEnd;
        while((posDAStart=daOne.indexIn(html,posDAStart))<posDAEnd){
            if(posDAStart==-1)
                break;
            TiXuanDa.append(daOne.cap(1));//答案
            posDAStart+=daOne.matchedLength();
        }//匹配完答案选项

        //获取正确答案
        QRegExp rightDA("正确答案：(\\w+)");//<strong>正确答案：DA
        if(rightDA.indexIn(html,posDAEnd)!=-1){
            TiXuanDa.append(rightDA.cap(1));
        }
        //获取题目编号
        QRegExp tmID("id=\\\"(\\w\\d+)");//用户选择：</strong><a id=\"d170146\"
        if(tmID.indexIn(html,posDAEnd)!=-1){
            QString id = tmID.cap(1);
            if(!m_exam.contains(id))
                newAdd++;
            m_exam[id]=TiXuanDa;
        }
        pos += tm.matchedLength();
    }
    ui->pushButtonExam->setText(QString("%1+%2").arg(m_exam.size()-newAdd).arg(newAdd));
    qDebug()<<"newAdd:"<<newAdd;
    allPageNewAdd += newAdd;

    int curPage,totalPage;
    QRegExp pageExp("showCurpage\">(\\w).*stotal\">(\\w)");//第<span id="showCurpage">1</span>页&nbsp;&nbsp;共<span id="stotal">4</span>页</span>
    if(pageExp.indexIn(html)!=-1){
        curPage = pageExp.cap(1).toInt();
        totalPage = pageExp.cap(2).toInt();
    }
    qDebug()<<QString("第%1页，共%2页").arg(curPage).arg(totalPage);
    if(curPage!=totalPage){
        m_WebViewExam->page()->runJavaScript("sps.changePage('next')");
        QTimer::singleShot(2*1000,this,SLOT(dealExamAnswer()));
        qDebug()<<"next page";
    }else{
        doExamHaveUnKnownId = false;
        m_TabWindow->closeTab(m_TabWindow->currentIndex());
        QTimer::singleShot(2*1000,this,SLOT(runExamJs()));
        qDebug()<<"all page";
    }
}

void starPlug::autoRun()
{
    qDebug()<<"autoRun()";
    m_timer->stop();
    autoRun(autoRunIndex);
}

void starPlug::haveAResultSlot(const QString &s)
{
    m_runJavaScriptResult.clear();
    m_runJavaScriptResult.append(s);
    qDebug()<<"haveAResultSlot "<<m_runJavaScriptResult;
}

QPair<bool, QVariant>  starPlug::syncRunJavaScript(QWebEnginePage *page, const QString &javascript, int msec)
{
    QPair<bool, QVariant> result = qMakePair(false, 0);
    QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
    QTimer::singleShot(msec, loop.data(), &QEventLoop::quit);
    page->runJavaScript(javascript, [loop, &result](const QVariant &val) {
        if (loop->isRunning()) {
            result.first = true;
            result.second = val;
            loop->quit();
        }
    });
    loop->exec();
    return result;
}

void starPlug::Save(bool b_TotalNum, bool b_ErrorId, const QString &s)
{
    qDebug()<<"save "<<b_TotalNum<<b_ErrorId<<s;
    QStringList list = s.split("\n");
    foreach (QString ss, list) {
        if(b_ErrorId && ss.startsWith("42138")){
            qDebug()<<QString("错误人员姓名：%1，账号：%2，密码：%3")
                      .arg(m_setting->value(ss+"/name").toString())
                      .arg(ss)
                      .arg(m_setting->value(ss+"/pw").toString());
            m_setting->setValue(ss+"/isVaild",false);
        }
        if(b_TotalNum){
            QStringList ll = ss.split("\t");
            if(ll.size()==4){
                QString name = ll.at(0);
                QString id = ll.at(1);
                QString todayNum = ll.at(2);
                QString TotalNum = ll.at(3);
                int p = todayNum.mid(todayNum.indexOf("：")+1).toInt();
                int q = TotalNum.mid(TotalNum.indexOf("：")+1).toInt();
                qDebug()<<QString("姓名：%1，今日积分：%2，总积分：%3").arg(name).arg(p).arg(q);
                QString errorString;
                int iniTotalNum = m_setting->value(id+"/TotalNum").toInt();
                if(iniTotalNum<q)
                {
                    m_setting->setValue(id+"/TotalNum",q);
                    m_setting->setValue(id+"/OldNum",iniTotalNum);
                    m_setting->setValue(id+"/TadayNum",p);
                    m_setting->setValue(id+"/Taday",QDate::currentDate().toString("yy-MM-dd"));
                }
                else
                    errorString.append(id+"\r\n");
            }
        }
    }
}

void starPlug::copy(const QString &s)
{
    qDebug()<<"copy "<<s;
}

void starPlug::slot_tabUrlChanged(QUrl url)
{
    if(url.toString().contains("exercies_3_t")){//考试界面
        m_WebViewExam = m_TabWindow->currentWebView();
        connect(m_WebViewExam,SIGNAL(loadFinished(bool)),this,SLOT(slot_viewLoadFinished(bool)));
    }
}

void starPlug::autoRun(int index)
{
    qDebug()<<"in autoRun: "<<index;
    autoRunIndex = index;
    bool toNext(false),needTimer(false);
    int tabCount = m_TabWindow->count();
    bool resultTestJquery;//

    switch (index) {
    case autoRunMain://打开首页
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);//关闭所有页面，最后一个页面被关闭后会新建一个空页面
        qDebug()<<"cachePath "<<m_TabWindow->currentWebView()->page()->profile()->cachePath();
        m_TabWindow->currentWebView()->page()->profile()->clearHttpCache();//这句无效
        m_TabWindow->currentWebView()->page()->profile()->cookieStore()->deleteAllCookies();//执行后不能登录。。。
        if(m_WebViewLogin)
            m_WebViewLogin->disconnect(this,SLOT(slot_viewLoadFinished(bool)));
        if(m_TabWindow)
            m_TabWindow->disconnect(this,SLOT(slot_newTabViewCreated()));//myObject->disconnect(myReceiver);应用对象this,而不是具体的slot
        m_browserWindow->loadPage("http://www.faxuan.net/bps/site/42.html");//
        m_WebViewLogin = m_TabWindow->currentWebView();
        connect(m_WebViewLogin,SIGNAL(loadFinished(bool)),this,SLOT(slot_viewLoadFinished(bool)));//QMessageBox
        connect(m_TabWindow,SIGNAL(newTabCreated()),this,SLOT(slot_newTabViewCreated()));//autoRunLogin-autoRunNum
        m_captcha.clear();
        m_captchaWaitTimes = 0;
        toNext = true;needTimer = true;break;//view_loadFinished--OCR
    case autoRunCheckCaptchaAndJquery:
        if(m_captcha.isEmpty()){//页面还没有载入完成
            qDebug()<<"页面还没有载入完成 ";
            m_captchaWaitTimes++;
            if(m_captchaWaitTimes==20){
                autoRunIndex = autoRunMain;//needTimer 5次*1秒后，刷新
            }
            toNext = false;needTimer=true;break;
        }

        resultTestJquery = syncRunJavaScript(m_WebViewLogin->page(),"typeof jQuery !='undefined'",500).second.toBool();

        if(!resultTestJquery){//Jquery没有加载成功
            qDebug()<<"Jquery没有加载成功";

            autoRunIndex = autoRunMain;
            toNext = false;needTimer=true;break;
        }

        if(m_captcha.size()==4 && !m_captcha.contains('*')){//验证码成功
            qDebug()<<"验证码成功 ";
            toNext= true;needTimer=true;break;
        }
        qDebug()<<"autoRunCheckCaptchaAndJquery break;";
        autoRunIndex = autoRunMain;
        toNext = false;needTimer=true;
        break;
    case autoRunLogin://登录
//        m_WebViewLogin->page()->runJavaScript(QString("$('#userAccount').val('%1')").arg(getCurrentUserId()));//账号错误，getCurrentUserId=-1
//        m_WebViewLogin->page()->runJavaScript(QString("$('#userPassword').val('%1')").arg(getCurrentPassword()));
//        m_WebViewLogin->page()->runJavaScript(QString("$('#usercheckcode').val('%1')").arg(m_captcha));
        m_WebViewLogin->page()->runJavaScript(QString("document.getElementById(\"userAccount\").value=\"%1\"").arg(getCurrentUserId()));//账号错误，getCurrentUserId=-1
        m_WebViewLogin->page()->runJavaScript(QString("document.getElementById(\"userPassword\").value=\"%1\"").arg(getCurrentPassword()));
        m_WebViewLogin->page()->runJavaScript(QString("document.getElementById(\"usercheckcode\").value=\"%1\"").arg(m_captcha));
        if(userInfoListIndex!=-1){
            m_WebViewLogin->page()->runJavaScript("$('.close_button').click()");//关闭账号密码错误的提示
            m_WebViewLogin->page()->runJavaScript("$('.login_button').click()");
        }
        else{//账号遍历完毕
            qDebug()<<userStudyResult;
            //QMessageBox::about(this,"学习结果",userStudyResult.join("\r\n"));//由此位置结束学习
            starDialogResult d(userStudyResult.join("\r\n"));
            connect(&d,SIGNAL(pushButtonCopy(QString)),this,SLOT(copy(QString)));
            connect(&d,SIGNAL(pushButtonSave(bool,bool,const QString &)),this,SLOT(Save(bool,bool,const QString &)));
            d.exec();
        }
        toNext = false;needTimer = false;break;//到newTabViewCreated中->autoRunNum
    case autoRunNum://分数界面。newTabViewCreated自动调用
        waitViewNum(toNext,needTimer);
        //未成功toNext = false;needTimer = true;
        //  成功toNext = false;needTimer = false;connect view_loadFinished中->autoRunCheckNum
        break;
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
            m_WebViewLogin->page()->runJavaScript("$('.login_button').click()");//再次点击主页面登陆
        }
        toNext = false;needTimer = false;//newTabViewCreated中->autoRunReNum
        break;
    case autoRunReNum:        //第二次登陆
        waitViewNum(toNext,needTimer);
        //未成功toNext = false;needTimer = true;
        //  成功toNext = false;needTimer = false;connect view_loadFinished->autoRunReNum+1
        break;
    case autoRunCheckNum:
    {
        int todayNum,totalNum;
        QString curName;
        getTodayResult(&todayNum,&totalNum,&curName);
        qDebug()<<"autoRunCheckNum "<<" "<<todayNum<<" "<<totalNum<<" "<<getIniTotalNum();

        if((getIniTotalNum()+todayNum)>totalNum){//记录的值比较大，则刷新,退回到上一步
            qDebug()<<QString("记录总分：%1，网页显示总分：%2").arg(getIniTotalNum()).arg(totalNum);
            autoRunIndex = autoRunNum-1;
            m_WebViewNum->reload();
            toNext = true;needTimer = true;break;
        }
        if(todayNum>=150){//大于110时，直接设置autoRunIndex跳转到autoRunNext
            autoRunIndex = autoRunNext-1;
            QString curResult;
            curResult = QString("%1\t%2\t今日积分：%3\t总积分：%4").arg(curName).arg(getCurrentUserId()).arg(todayNum).arg(totalNum);
            userStudyResult.append(curResult);
            userInfoListIndex++;
            m_WebViewNum->disconnect();
            m_TabWindow->closeTab(m_TabWindow->currentIndex());
            qDebug()<<curResult;
            toNext = true;needTimer = true;break;
        }
        toNext = true;needTimer = true;break;
    }
    case autoRunBindQQ://打开计时界面
        m_TabWindow->currentWebView()->page()->runJavaScript("bps.remCook('bindQQ')");
        toNext = true;needTimer = true;break;
    case autoRunOpenClass://打开计时界面
        //m_TabWindow->currentWebView()->page()->runJavaScript("location.href = $(\"[href$='考试复习指南（2018）&d=AA==']:first\").attr('href')");
        //m_WebViewNum->page()->runJavaScript("window.open($(\"[href$='考试复习指南（2019）&d=AA==']:first\").attr('href'))");
        m_WebViewNum->page()->runJavaScript("window.open($(\"[href$='【视频】《反分裂国家法》解读 &d=AA==']\").attr('href'))");
        qDebug()<<"href";
        toNext = false;needTimer = false;break;//到newTabViewCreated中->autoRunClassOpened
        break;
    case autoRunClassOpened:
        qDebug()<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
        if(m_TabWindow->currentWebView()->title()==(QString("400 Request Header Or Cookie Too Large"))){
            autoRunIndex =  autoRunLogin-1;
            errorNeedReboot = true;
            toNext = false;needTimer = true;
        }else if( m_TabWindow->count()!=3 || m_TabWindow->currentWebView()->title()!=(QString("课程"))){
            qDebug()<<"autoRunClassOpened autoRunIndex--  "<<m_TabWindow->count()<<m_TabWindow->currentWebView()->title();
            toNext = false;needTimer = true;
        }else{
            m_WebViewClass = m_TabWindow->currentWebView();
            connect(m_WebViewClass,SIGNAL(loadFinished(bool)),this,SLOT(slot_viewLoadFinished(bool)));//->autoRunSetTimer
            m_WebViewClass->reload();//网速太快时，connect来不及生效……即使生效view_loadFinished的m_WebViewClass中只处理autoRunClassOpened->autoRunSetTimer
            qDebug()<<"connect m_WebViewClass view_loadFinished";
            toNext = false;needTimer = false;
        }
        break;
    case autoRunSetTimer://更改计时
        m_WebViewClass->page()->runJavaScript("sps.startTiming('timer',10000)");
        qDebug()<<"startTiming";
        toNext = true;needTimer = true;break;
    case autoRunDoExam://做练习题
        m_WebViewClass->page()->runJavaScript("sps.detail('2087','第一章　全面推进依法治国的重大战略布局','1')");
        //同时激活newTabViewCreated（不做处理），tabUrlChanged，连接m_WebViewClass到view_loadFinished
        dealExam3over = false; dealExam4over = false; doExamMode3 = true; doExamMode4 = true; doExamHaveUnKnownId = false;doExamOneOver=false;
        toNext = false;needTimer = false;break;
        break;
    case autoRunDoExamOver:
        m_WebViewExam->disconnect();
        if(doExamHaveUnKnownId){
            dealExamAnswer();//结束后会关闭m_WebViewExam
        }else{
            m_TabWindow->closeTab(m_TabWindow->currentIndex());
        }
        toNext = true;needTimer = true;break;
    case autoRunDoExamDealOver:
        qDebug()<<"autoRunDoExamDealOver "<<doExamHaveUnKnownId;
        toNext = !doExamHaveUnKnownId;
        needTimer = true;
        break;
    case autoRunExitTimer://退出学习
        m_WebViewClass->page()->runJavaScript("sps.exitStudy('timer')");
        qDebug()<<"exitStudy";
        toNext = true;needTimer = true;break;
    case autoRunPopwinConfirm://确认退出学习，会自动关闭当前页面
        m_WebViewClass->disconnect();
        m_TabWindow->currentWebView()->page()->runJavaScript("$('#popwinConfirm').click()");
        qDebug()<<"popwinConfirm";
        toNext = true;needTimer = true;break;

    case autoRunReNumReload:
        m_WebViewNum->reload();
        toNext = false;needTimer = false;break;//slot_viewLoadFinished中autoRunGetResult
    case autoRunGetResult:
        getTodayResult(0,0,0);
        m_WebViewNum->disconnect();
        m_TabWindow->closeTab(m_TabWindow->currentIndex());
        toNext = true;needTimer = true;break;
    case autoRunNext:
    {
        bool b= ui->checkBox->isChecked();
        if(b){
            toNext = b;
            autoRunIndex=autoRunMain-1;//退回到autoRunLogin，在autoRunCheckNum中
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
    autoRun(autoRunLogin);
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
/*
$("[href^='javascript:sps.detail']")
[<a href=​"javascript:sps.detail('979','第一章　全面推进依法治国的重大战略布局','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('980','第一章　全面推进依法治国的重大战略布局','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('981','第三章　 七五普法规划知识','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('982','第四章　法治思维和法治方式','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('983','第五章　宪法和宪法相关法','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('984','第六章　公务员简明法律知识','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('985','第七章　公务员法律制度','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('986','第八章　公务员依法行政概述','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('987','第九章　公务员依法行政法律制度','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('988','第十章　公务员依法行政常见违法问题','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('989','第十一章　公务员廉政建设和常见职务犯罪预防','1')​">​开始练习​</a>​,
<a href=​"javascript:sps.detail('990','第十二章　党内法规的学习宣传','1')​">​开始练习​</a>​]

javascript:sps.myCommit();
$('.tanchu_btn01').click()
题目：
$("h3")

*/

//    QFile file;
//    file.setFileName(":/js/q.js");
//    file.open(QIODevice::ReadOnly);
//    jQuery = file.readAll();
//    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
//    file.close();
//    QFile file;
//    file.setFileName("userPassword.txt");
//    file.open(QIODevice::ReadOnly);
//    userInfoAll = QString(file.readAll()).split("\r\n");
//    file.close();
//    userInfoListIndex=0;
//    qDebug()<<userInfoAll;


//    {
//        foreach (QString s, userInfoAll) {
//            QStringList l = s.split("\t");
//            if(l.size()==3){
//                QString name = l.at(0);
//                QString id = l.at(1);
//                QString pass = l.at(2);
//                m_setting->beginGroup(id);
//                m_setting->setValue("name",name);
//                m_setting->setValue("pw",pass);
//                m_setting->setValue("isVaild",!name.startsWith("*"));
//                m_setting->setValue("TotalNum",100);
//                m_setting->setValue("OldNum",100);
//                m_setting->endGroup();
//            }
//        }
//    }

//使用syncRunJavaScript获取验证码地址失败，不明原因。。。
//QMessageBox::about(this,"输入验证码",QString("主页载入：%1\r\n请在网页正确位置输入验证码\r\n（仅输入验证么即可）").arg(b));
//            QPair<bool,QVariant> r = syncRunJavaScript(m_WebViewLogin->page(),"$('#captcha').attr('src')",5*1000);
//            if(r.first){
//                QString s = r.second.toString();
//                slot_captchaChanged(s);
//            }
//            qDebug()<<r.first<<r.second;

//            QString runJavaScriptResult;//
//            QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
//            m_TabWindow->currentWebView()->page()->runJavaScript("$('#captcha').attr('src')",[loop,&runJavaScriptResult] (const QVariant& r){
//                if(loop->isRunning()){
//                    runJavaScriptResult = r.toString();
//                    loop->quit();
//                }
//            });
//            //loop->exec();
//            qDebug()<<runJavaScriptResult;
//slot_captchaChanged(runJavaScriptResult);

//m_WebViewLogin->page()->runJavaScript("$('#captcha').attr('src')",[this](const QVariant& r){emit slot_captchaChanged(r.toString());});


void starPlug::on_pushButtonExam_clicked()
{
    ui->pushButtonExam->setEnabled(false);
    QFile file;
    file.setFileName(QCoreApplication::applicationDirPath()+"/exam.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<m_exam;
    file.close();

    emit signal_examDataChanged(m_exam);
    ui->pushButtonExam->setEnabled(true);
}

void starPlug::on_checkBox_2_clicked()
{
    if(ui->checkBox_2->isChecked())
        m_dialogExam->show();
    else
        m_dialogExam->hide();
}

void starPlug::on_pushButtonGetExam_clicked()
{
    QString buttonText = ui->pushButtonGetExam->text();
    if(buttonText.contains("获取题库")){
        if(m_TabWindow->currentWebView()->url().toString().contains("courseware_1_t")){
            ui->pushButtonGetExam->setText("结束获取");
            m_WebViewClass = m_TabWindow->currentWebView();
            if(startExamJSList.isEmpty()){
                startExamJSList.append("sps.detail('2087','第一章　全面推进依法治国的重大战略布局','1')");
                startExamJSList.append("sps.detail('2088','第一章　全面推进依法治国的重大战略布局','1')");
                startExamJSList.append("sps.detail('2089','第三章　 七五普法规划知识','1')");
                startExamJSList.append("sps.detail('2090','第四章　法治思维和法治方式','1')");
                startExamJSList.append("sps.detail('2091','第五章　宪法和宪法相关法','1')");
                startExamJSList.append("sps.detail('2092','第六章　公务员简明法律知识','1')");
                startExamJSList.append("sps.detail('2093','第七章　公务员法律制度','1')");
                startExamJSList.append("sps.detail('2094','第八章　公务员依法行政概述','1')");
                startExamJSList.append("sps.detail('2095','第九章　公务员依法行政法律制度','1')");
                startExamJSList.append("sps.detail('2096','第十章　公务员依法行政常见违法问题','1')");
                startExamJSList.append("sps.detail('2097','第十一章　公务员廉政建设和常见职务犯罪预防','1')");
                startExamJSList.append("sps.detail('2098','第十二章　党内法规的学习宣传','1')");
            }
            startExamJSListIndex = 0;
            startExamJSListIndexBack = 0;
            allPageNewAdd = 0;
            runExamJs();
        }
    }else{
        ui->pushButtonGetExam->setText("获取题库");
    }
}

void starPlug::runExamJs()
{
    qDebug()<<"runExamJs "<<startExamJSListIndex;
    if(ui->pushButtonGetExam->text().contains("结束获取")){
        m_WebViewClass->page()->runJavaScript(startExamJSList.at(allPageNewAdd!=0?startExamJSListIndexBack:startExamJSListIndex));//tabUrlChanged
        if(allPageNewAdd==0){
            startExamJSListIndexBack = startExamJSListIndex;
            startExamJSListIndex++;
        }
        allPageNewAdd = 0;
        if(startExamJSListIndex==startExamJSList.size()){
            startExamJSListIndex = 0;
        }
        dealExam3over = true;
        dealExam4over = true;
        doExamMode3 = false;
    }
}

void starPlug::commitExam()
{
    m_WebViewExam->page()->runJavaScript("sps.myCommit()");
    m_WebViewExam->page()->runJavaScript("$('#popwinConfirm').click()");
}

void starPlug::doExam()
{
//$("[name='allitem'][value='A']").click()
    TiMuIndexLast = 0;
    doExamHtml();
}

void starPlug::doExamHtml()
{
    QString html;
    QWebEnginePage *page = m_TabWindow->currentWebView()->page();
    QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
    page->toHtml([loop,&html] (const QVariant& r){
        if(loop->isRunning()){
            html = r.toString();
            loop->quit();
        }
    });
    loop->exec();

    QStringList TiMuIdList;
    int TiMuIndex;
    QString CurTiMuId;

    QRegExp allTiMu("hids.*=\"(.*),\">");
    if(allTiMu.indexIn(html)!=-1){
        TiMuIdList = allTiMu.cap(1).split(",");
        //qDebug()<<"TiMuIdList "<<TiMuIdList.size()<<" "<<TiMuIdList;
    }

    QRegExp idExp("id=\"curti\">(\\w+)<");
    if(idExp.indexIn(html)!=-1){
        TiMuIndex = idExp.cap(1).toInt();
        qDebug()<<"TiMuIndex "<<TiMuIndex;
    }

    if(TiMuIndex==TiMuIdList.size())
        doExamOneOver = true;
    TiMuIndex--;
    if(TiMuIndex<TiMuIdList.size()){
        CurTiMuId = QString("d%1").arg(TiMuIdList.at(TiMuIndex));
        //qDebug()<<"CurTiMuId "<<CurTiMuId;
        emit signal_doExamCurTiMuId(CurTiMuId);
        if(m_exam.contains(CurTiMuId)){
            QStringList list = m_exam.value(CurTiMuId);
            QString DA = list.last();
            //qDebug()<<"DA "<<DA<<DA.size();
            for(int i=0;i<DA.size();i++){
                QString iDA = QString(DA[i]);//答案列表的第一个字母
                QString DAString;
                if(iDA==QString('A')){//单选多选，正确答案存储的为字母
                    DAString = list.at(1);
                }else if(iDA==QString('B')){
                    DAString = list.at(2);
                }else if(iDA==QString('C')){
                    DAString = list.at(3);
                }else if(iDA==QString('D')){
                    DAString = list.at(4);
                }
                DAString.remove(0,3);//答案前后一个空格……
                if(iDA==QString("对")){//判断题，正确答案储存的为对/错
                    DAString = QString("对");
                }else if(iDA==QString("错")){
                    DAString = QString("错");
                }
                //qDebug()<<"iDA "<<iDA<<"DAString "<<DAString;
                QRegExp eee(QString("name=\"allitem\".*value=\"(\\w)\"> \\w\\.%1</li>").arg(DAString));//html中value="XXX"
                if(eee.indexIn(html)!=-1){
                    QString realDAIndex = eee.cap(1);
                    page->runJavaScript(QString("$(\"[name='allitem'][value='%1']\").click()").arg(realDAIndex));
                }
            }
        }else{
            doExamHaveUnKnownId = true;
            qDebug()<<"doExamHaveUnKnownId "<<CurTiMuId;
        }
        QTimer::singleShot(500,this,SLOT(doExamNext()));
    }
}

void starPlug::doExamNext()
{
    QWebEnginePage *page = m_TabWindow->currentWebView()->page();
    if(!doExamOneOver){//没有做完
        page->runJavaScript("sps.next()");
        QTimer::singleShot(500,this,SLOT(doExamHtml()));
    }else{//题目做完了
        QTimer::singleShot(1*1000,this,SLOT(commitExam()));
    }
}

void starPlug::slot_captchaChanged(const QString &c)
{
    qDebug()<<"slot_captchaChanged";
    if(net_manager){
        qDebug()<<c;
        net_manager->get(QNetworkRequest(QUrl(c)));
    }
}

void starPlug::replyFinished(QNetworkReply *reply)
{
    QByteArray bytes = reply->readAll();
    qDebug()<<"replyFinished "<<bytes.size();
    QImage img = QImage::fromData(bytes);
    QLabel *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(img));
    label->show();
}

void starPlug::downloadRequested(QWebEngineDownloadItem *dItem)
{
    qDebug()<<"downloadRequested";
    qDebug()<<dItem->mimeType();
    dItem->setSavePageFormat(QWebEngineDownloadItem::CompleteHtmlSaveFormat);//single HTML page and the resources
    dItem->setPath(QCoreApplication::applicationDirPath()+"/faxuan.html");
    connect(dItem,SIGNAL(finished()),this,SLOT(htmlDownloadFinished()));
    dItem->accept();
}
QRect starPlug::PixelList2(QStringList list) const
{
    int max = list.size()-1;
    int widStart(0),widEnd(0),heightStart(max),heightEnd(0);
    bool findS(false);
    for(int i=0;i<list.size();i++){
        QString hLine = list.at(i);
        if(!findS){
            findS = hLine.contains('1');
            widStart = i;
        }
        if(findS){
            if(!hLine.contains('1')){
                widEnd = i-1;
                break;
            }else
                widEnd = i;
            heightStart = qMin(heightStart,hLine.indexOf('1'));
            heightEnd = qMax(heightEnd,hLine.lastIndexOf('1'));
        }
    }

    return QRect(widStart,heightStart,widEnd-widStart+1,heightEnd-heightStart+1);
}

QStringList starPlug::getStrLFromStrL(QStringList list, QRect rect)
{
    int startW = rect.x();
    int endW = startW+rect.width();
    int startH = rect.y();

    QStringList rList;
    for(int i=startW;i<endW;i++){
        QString lineH = list.at(i);
        rList.append(lineH.mid(startH,rect.height()));
    }
    return rList;
}
QString starPlug::autoOCR(const QList<QStringList> &ll)
{
    QString rt;
    foreach (QStringList lr, ll) {
        QMapIterator<QString,QStringList> i(oneCharDataMap);
        QString key;
        while(i.hasNext()){
            i.next();
            if(lr==i.value()){
                key = i.key();
                break;
            }
        }
        if(!key.isEmpty()){
            rt.append(key.mid(0,key.indexOf('-')));
        }else
            rt.append('*');
    }
    return rt;
}
void starPlug::htmlDownloadFinished()
{
    QImage img;
    bool b = img.load(QCoreApplication::applicationDirPath()+"/faxuan_files/gc.html");
    if(!b){
        qDebug()<<"load fail";
        return;
    }

    int RGB(380),R(123),G(128),B(123);
    int imgH = img.height();
    int imgW = img.width();

    QImage img_t(imgW,imgH,QImage::Format_RGB888);

    for(int w=0;w<imgW;w++){
        for(int h=0;h<imgH;h++){
            QColor c = img.pixelColor(w,h);
            if((c.red()+c.green()+c.blue())<RGB && (c.red()<R || c.green()<G || c.blue()<B) ){
                img_t.setPixel(w,h,QColor(0,0,0).rgb());
            }else
                img_t.setPixel(w,h,QColor(255,255,255).rgb());
        }
    }

    QStringList t_list;//将打开图片转换成字符串列表
    QImage t_img = img_t.convertToFormat(QImage::Format_Mono);
    for(int w=0;w<t_img.width();w++){
        QString s;
        for(int h=0;h<t_img.height();h++){
            s.append(t_img.pixelColor(w,h).red()==255?"0":"1");//1为有，0为无
        }
        t_list.append(s);
    }

    QList<QStringList> fourCharListList;
    while(1){
        QRect rect = PixelList2(t_list);//一个个的获取t_list中单独字符区域
        //qDebug()<<rect<<" "<<rect.topLeft()<<" "<<rect.topRight()<<" "<<rect.bottomLeft()<<" "<<rect.bottomRight();
        if(rect.isValid()){
            QStringList oneCharList = getStrLFromStrL(t_list,rect);//将指定区域转换成字符串列表，即一个字母的图形字符串列表
            fourCharListList.append(oneCharList);
            for(int i=rect.x();i<rect.x()+rect.width();i++){//将已获取区域清零，以显示下一个
                if(i<t_list.size())
                    t_list[i].fill('0');
            }
        }else
            break;
    }
    qDebug()<<"fourCharListList"<<fourCharListList.size();
    m_captcha = autoOCR(fourCharListList);
    qDebug()<<"OCR:"<<m_captcha;
}

