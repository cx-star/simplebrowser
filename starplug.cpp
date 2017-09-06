#include "starplug.h"
#include "ui_starplug.h"

starPlug::starPlug(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::starPlug)
{
    ui->setupUi(this);
}

starPlug::~starPlug()
{
    delete ui;
}
