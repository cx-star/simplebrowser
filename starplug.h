#ifndef STARPLUG_H
#define STARPLUG_H

#include <QTimer>
#include <QWidget>
#include <QSettings>
#include "browserwindow.h"
#include "tabwidget.h"
#include "webview.h"

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
private slots:
    void on_pushButton_clicked();
    void newTabViewCreated();
    void view_loadFinished(bool b);

    void on_pushButtonLogin_clicked();
    void autoRun(int index);
    void autoRun();

    void haveAResultSlot(const QString&s);


    void Save(bool b_TotalNum,bool b_ErrorId,const QString& s);
    void copy(const QString& s);

    void tabUrlChanged(QUrl url);
    void on_pushButtonExam_clicked();

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
    QTimer *m_timer;
    void waitTimer(int t=1*1000);
    int autoRunIndex;
    enum autoRunType{autoRunMain,autoRunLogin,autoRunNum,autoRunCheckNum,autoRunBindQQ,
                     autoRunOpenClass,autoRunClassOpened,autoRunSetTimer,autoRunExitTimer,autoRunPopwinConfirm,
                    autoRunCloseNumForReload,autoRunReLogin,autoRunReNum,autoRunReNumReload,autoRunGetResult,autoRunNext,
                    execLineEdit,execLineEditWithResult};
    QString m_runJavaScriptResult;
    void waitViewNum(bool &toNext, bool &needTimer);
    QStringList userInfoAll,userInfoCur,userStudyResult;
    int userInfoListIndex;
    void getTodayResult(int* todayNum, int* TotalNum, QString *curName);

    QSettings *m_setting;

    QMap<QString,QStringList> m_exam;
    void dealExam();
};

#endif // STARPLUG_H
