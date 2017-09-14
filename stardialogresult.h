#ifndef STARDIALOGRESULT_H
#define STARDIALOGRESULT_H

#include <QDialog>

namespace Ui {
class starDialogResult;
}

class starDialogResult : public QDialog
{
    Q_OBJECT

public:
    explicit starDialogResult(const QString& s,QWidget *parent = 0);
    ~starDialogResult();
signals:
    void pushButtonSave(bool b_TotalNum,bool b_ErrorId,const QString& s);
    void pushButtonCopy(const QString& s);
private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonQuit_clicked();

    void on_pushButtonCopy_clicked();

private:
    Ui::starDialogResult *ui;
};

#endif // STARDIALOGRESULT_H
