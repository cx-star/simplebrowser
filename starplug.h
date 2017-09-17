#ifndef STARPLUG_H
#define STARPLUG_H

#include <QTimer>
#include <QWidget>
#include <QSettings>
#include "browserwindow.h"
#include "tabwidget.h"
#include "webview.h"
#include "stardialogexam.h"

namespace Ui {
class starPlug;
}
class BrowserWindow;
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
    QPair<bool, QVariant>  starPlug::syncRunJavaScript(QWebEnginePage *page, const QString &javascript, int msec);

    void Save(bool b_TotalNum,bool b_ErrorId,const QString& s);
    void copy(const QString& s);

    void slot_tabUrlChanged(QUrl url);
    void on_pushButtonExam_clicked();
    void dealExam();
    void on_checkBox_2_clicked();
    void on_pushButtonGetExam_clicked();
    void runExamJs();
    void commitExam();
    void doExam();
    void doExamHtml();
    void doExamNext();

private:
    Ui::starPlug *ui;
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
    enum autoRunType{autoRunMain,autoRunLogin,autoRunNum,autoRunCheckNum,autoRunBindQQ,
                     autoRunOpenClass,autoRunClassOpened,autoRunSetTimer,autoRunDoExam,autoRunDoExamOver,autoRunDoExamDealOver,autoRunExitTimer,autoRunPopwinConfirm,
                    autoRunCloseNumForReload,autoRunReLogin,autoRunReNum,autoRunReNumReload,autoRunGetResult,autoRunNext,
                    execLineEdit,execLineEditWithResult};
    QString m_runJavaScriptResult;
    void waitViewNum(bool &toNext, bool &needTimer);
    QStringList userInfoAll,userInfoCur,userStudyResult;
    int userInfoListIndex;
    void getTodayResult(int* todayNum, int* TotalNum, QString *curName);

    QSettings *m_setting;

    QMap<QString,QStringList> m_exam;
    DialogExam *m_dialogExam;
    QStringList startExamJSList;
    int startExamJSListIndex,startExamJSListIndexBack;
    bool dealExam3over,dealExam4over,doExamMode3,doExamMode4;
    int allPageNewAdd;
    bool doExamHaveUnKnownId,doExamOneOver;
    int TiMuIndexLast;
};

#endif // STARPLUG_H
