#include <QDebug>
#include "stardialogexam.h"
#include "ui_stardialogexam.h"

#include <QStringListModel>

DialogExam::DialogExam(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExam)
{
    ui->setupUi(this);
    model = new QStringListModel();
    ui->comboBoxID->setModel(model);
    connect(ui->comboBoxID,SIGNAL(currentIndexChanged(QString)),ui->textEdit,SLOT(scrollToAnchor(QString)));
}

DialogExam::~DialogExam()
{
    delete ui;
}

QMap<QString, QStringList> DialogExam::examData() const
{
    return m_examData;
}

void DialogExam::setExamData(const QMap<QString, QStringList> &examData)
{
    m_examData = examData;
}

void DialogExam::updateExamData(const QMap<QString, QStringList> &data)
{
    QStringList IDs;
    IDs = data.keys();
    model->setStringList(IDs);

    ui->label->setText(QString("%1").arg(data.size()));

    ui->textEdit->clear();
    QMapIterator<QString, QStringList> iterator(data);
    while(iterator.hasNext()){
        iterator.next();
        QStringList list = iterator.value();
        QString id = iterator.key();

        QString TiMu = list.at(0);
        int index = TiMu.indexOf("、");
        TiMu.remove(0,index);
        TiMu.prepend(QString("<A name=\"%1\"><span style=\" color:#ff0000;\">%2").arg(id).arg(id));
        TiMu.append("</span></A>");
        list[0] = TiMu;

        QString DaAn = list.last();
        DaAn.prepend("<span style=\" color:#00ff00;\">正确答案：");
        TiMu.append("</span>");
        list.last() = DaAn;

        for(int i=0;i<list.size();i++){
            ui->textEdit->append(list.at(i));
        }
        ui->textEdit->append("");
    }
}

void DialogExam::on_pushButtonSearch_clicked()
{
    bool b = ui->textEdit->find(ui->lineEditSearch->text(),flag);
    if(!b){
        flag.setFlag(QTextDocument::FindBackward,!flag.testFlag(QTextDocument::FindBackward));
        b = ui->textEdit->find(ui->lineEditSearch->text(),flag);
    }
}
