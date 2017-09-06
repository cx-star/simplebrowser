#ifndef STARPLUG_H
#define STARPLUG_H

#include <QWidget>
#include "browserwindow.h"
#include "tabwidget.h"

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

private:
    Ui::starPlug *ui;
    QStringList m_comboBoxList;
    TabWidget *m_TabWindow;
    BrowserWindow *m_browserWindow;
};

#endif // STARPLUG_H
