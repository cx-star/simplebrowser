#ifndef STARPLUG_H
#define STARPLUG_H

#include <QTimer>
#include <QWidget>
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
    explicit starPlug(QWidget *parent = 0);
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
private:
    Ui::starPlug *ui;
    QStringList m_comboBoxList;
    TabWidget *m_TabWindow;
    BrowserWindow *m_browserWindow;
    QString jQuery;
    WebView *m_WebViewLogin;
    WebView *m_WebViewNum,*m_WebViewClass;
    int currentNameIndex;
    QString getCurrentName();
    QString getCurrentPassword();
    QTimer *m_timer;
    void waitTimer(int t=1*1000);
    int autoRunIndex;
    enum autoRunType{autoRunMain,autoRunLogin,autoRunNum,autoRunBindQQ,
                     autoRunOpenClass,autoRunClassOpened,autoRunSetTimer,autoRunExitTimer,autoRunPopwinConfirm,
                    autoRunCloseNumForReload,autoRunReLogin,autoRunReNum,autoRunReNumReload,autoRunGetResult,autoRunNext,
                    execLineEdit,execLineEditWithResult};
    QString mm_runJavaScriptResult;
    void waitViewNum(bool &toNext, bool &needTimer);
    QStringList userInfoAll,userInfoCur,userStudyResult;
    int userInfoListIndex;
    int getTodayResult();
};

#endif // STARPLUG_H
