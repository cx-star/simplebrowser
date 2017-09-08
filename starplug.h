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

private slots:
    void on_pushButton_clicked();
    void login_loadFinished(bool b);
    void newTabViewCreated();
    void main_loadFinished(bool b);

    void on_pushButtonLogin_clicked();
    void autoRun(int index);
    void autoRun();

private:
    Ui::starPlug *ui;
    QStringList m_comboBoxList;
    TabWidget *m_TabWindow;
    BrowserWindow *m_browserWindow;
    QString jQuery;
    WebView *m_loginWebWiew;
    int currentNameIndex;
    QString getCurrentName(int ci);
    QString getCurrentPassword(int ci);
    QTimer *m_timer;
    void waitTimer();
    int autoRunIndex;
};

#endif // STARPLUG_H
