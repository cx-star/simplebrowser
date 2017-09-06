#ifndef STARPLUG_H
#define STARPLUG_H

#include <QWidget>

namespace Ui {
class starPlug;
}

class starPlug : public QWidget
{
    Q_OBJECT

public:
    explicit starPlug(QWidget *parent = 0);
    ~starPlug();

private:
    Ui::starPlug *ui;
};

#endif // STARPLUG_H
