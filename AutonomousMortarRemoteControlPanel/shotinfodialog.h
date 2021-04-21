#ifndef SHOTINFODIALOG_H
#define SHOTINFODIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "shotinfo.h"


namespace Ui {
class ShotInfoDialog;
}

class ShotInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShotInfoDialog(QWidget *parent = nullptr, ShotInfo* inShot = nullptr);
    ~ShotInfoDialog();

private slots:
    // Detects which button was clicked. If that button is "OK" save the needed information
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    // Default ui for shotinfodialog
    Ui::ShotInfoDialog *ui;

    // ShotInfo object that this dialog can make edits to
    ShotInfo* shot;
};

#endif // SHOTINFODIALOG_H
