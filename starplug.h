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

private:
    Ui::starPlug *ui;
    QStringList m_comboBoxList;
    TabWidget *m_TabWindow;
    BrowserWindow *m_browserWindow;
    QString jQuery;
    WebView *m_loginWebWiew;
    int currentNameIndex;
    const QString& getCurrentName(int ci);
    const QString& getCurrentPassword(int ci);
    QTimer *timer;
    int autoRunIndex;
    void autoRun(int index);
};

#endif // STARPLUG_H
