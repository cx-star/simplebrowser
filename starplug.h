
#ifndef STARPLUG_H
#define STARPLUG_H

#include <QtNetwork>
#include <QTimer>
#include <QWidget>
#include <QSettings>
#include <QWebEngineDownloadItem>
#include <QWebEnginePage>
#include "stardialogexam.h"
#include "webview.h"
#include "browserwindow.h"

class BrowserWindow;
class TabWidget;

namespace Ui {
class starPlug;
}

class starPlug : public QWidget
{
    Q_OBJECT

public:
    explicit starPlug(TabWidget* tw,QWidget *parent = 0);
    ~starPlug();

    TabWidget *TabWindow() const;
    void setTabWindow(TabWidget *TabWindow);

    BrowserWindow *browserWindow() const;
    void setBrowserWindow(BrowserWindow *browserWindow);
signals:
    void haveAResultSignal(const QString& r);
    void signal_examDataChanged(const QMap<QString,QStringList>& d);
    void signal_doExamCurTiMuId(const QString& id);
private slots:
    void on_pushButton_clicked();
    void slot_newTabViewCreated();
    void slot_viewLoadFinished(bool b);

    void on_pushButtonLogin_clicked();
    void autoRun(int index);
    void autoRun();

    void haveAResultSlot(const QString&s);

    void Save(bool b_TotalNum,bool b_ErrorId,const QString& s);
    void copy(const QString& s);

    void slot_tabUrlChanged(QUrl url);
    void on_pushButtonExam_clicked();
    void dealExamAnswer();
    void on_checkBox_2_clicked();
    void on_pushButtonGetExam_clicked();
    void runExamJs();
    void commitExam();
    void doExam();
    void doExamHtml();
    void doExamNext();

    void slot_captchaChanged(const QString & c);
    void replyFinished(QNetworkReply *reply);

    void downloadRequested(QWebEngineDownloadItem*dItem);
    void htmlDownloadFinished();
private:
    Ui::starPlug *ui;

    bool errorNeedReboot;

    QStringList m_comboBoxList;
    TabWidget *m_TabWindow;
    BrowserWindow *m_browserWindow;
    QString jQuery;
    WebView *m_WebViewLogin;
    WebView *m_WebViewNum,*m_WebViewClass,*m_WebViewExam;
    int currentNameIndex;
    QString getCurrentUserId();
    QString getCurrentPassword();
    int getIniTotalNum();
    int getCurrentOldNum();
    QTimer *m_timer,*m_timerDoExam;
    void waitTimer(int t=1*1000);
    int autoRunIndex;
    enum autoRunType{autoRunMain,autoRunCheckCaptchaAndJquery,autoRunLogin,autoRunNum,autoRunCloseNumForReload,autoRunReLogin,autoRunReNum,autoRunCheckNum,autoRunBindQQ,
                     autoRunOpenClass,autoRunClassOpened,autoRunSetTimer,autoRunDoExam,autoRunDoExamOver,autoRunDoExamDealOver,autoRunExitTimer,autoRunPopwinConfirm,
                    autoRunReNumReload,autoRunGetResult,autoRunNext,
                    execLineEdit,execLineEditWithResult};
    QString m_runJavaScriptResult;
    void waitViewNum(bool &toNext, bool &needTimer);
    QStringList userInfoAll,userInfoCur,userStudyResult;
    int userInfoListIndex;
    void getTodayResult(int* todayNum, int* TotalNum, QString *curName);
    QPair<bool, QVariant>  syncRunJavaScript(QWebEnginePage *page, const QString &javascript, int msec);

    QSettings *m_setting;

    QMap<QString,QStringList> m_exam;
    DialogExam *m_dialogExam;
    QStringList startExamJSList;
    int startExamJSListIndex,startExamJSListIndexBack;
    bool dealExam3over,dealExam4over,doExamMode3,doExamMode4;
    int allPageNewAdd;
    bool doExamHaveUnKnownId,doExamOneOver;
    int TiMuIndexLast;

    QNetworkAccessManager *net_manager;
    QString m_captcha;
    int m_captchaWaitTimes;
    QMap<QString,QStringList> oneCharDataMap;

    QRect PixelList2(QStringList list) const;
    QStringList getStrLFromStrL(QStringList list, QRect rect);
    QString autoOCR(const QList<QStringList> &ll);
};

#endif // STARPLUG_H
