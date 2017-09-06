#include "starplug.h"
#include "ui_starplug.h"
#include "tabwidget.h"
#include <QDebug>

starPlug::starPlug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::starPlug)
{
    ui->setupUi(this);
    m_comboBoxList<<"打开首页"<<"自动填表"<<"登录";
    ui->comboBox->addItems(m_comboBoxList);
}

starPlug::~starPlug()
{
    delete ui;
}

void starPlug::on_pushButton_clicked()
{
    int tabCount = m_TabWindow->count();
    switch (ui->comboBox->currentIndex()) {
    case 0:
        for(int i=tabCount-1;i>=0;i--)
            m_TabWindow->closeTab(i);
        m_browserWindow->loadPage("http://www.faxuan.net/site/hubei/");
        break;
    default:
        break;
    }
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

