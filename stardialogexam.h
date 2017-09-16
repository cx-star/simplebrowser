#ifndef DIALOGEXAM_H
#define DIALOGEXAM_H

#include <QDialog>
#include <QMap>
#include <QStringListModel>
#include <QFlags>
#include <QTextDocument>

namespace Ui {
class DialogExam;
}

class DialogExam : public QDialog
{
    Q_OBJECT

public:
    explicit DialogExam(QWidget *parent = 0);
    ~DialogExam();


    QMap<QString, QStringList> examData() const;

    void setExamData(const QMap<QString, QStringList> &examData);

public slots:
    void updateExamData(const QMap<QString,QStringList>& data);
private slots:
    void on_pushButtonSearch_clicked();

private:
    Ui::DialogExam *ui;

    QMap<QString,QStringList> m_examData;
    QStringListModel *model;

    QFlags<QTextDocument::FindFlag> flag;
};

#endif // DIALOGEXAM_H
