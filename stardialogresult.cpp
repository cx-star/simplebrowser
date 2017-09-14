#include "stardialogresult.h"
#include "ui_stardialogresult.h"

starDialogResult::starDialogResult(const QString &s, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::starDialogResult)
{
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText(s);
}

starDialogResult::~starDialogResult()
{
    delete ui;
}

void starDialogResult::on_pushButtonSave_clicked()
{
    emit pushButtonSave(ui->checkBoxSaveTotalNum->isChecked(),ui->checkBoxUpdateErrorId->isChecked(),ui->plainTextEdit->toPlainText());
}

void starDialogResult::on_pushButtonQuit_clicked()
{
    this->reject();
}

void starDialogResult::on_pushButtonCopy_clicked()
{
    emit pushButtonCopy(ui->plainTextEdit->toPlainText());
}
