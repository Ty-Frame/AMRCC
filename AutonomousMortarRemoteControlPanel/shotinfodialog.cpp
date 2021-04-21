#include "shotinfodialog.h"
#include "ui_shotinfodialog.h"

ShotInfoDialog::ShotInfoDialog(QWidget *parent, ShotInfo* inShot) :
    QDialog(parent),
    ui(new Ui::ShotInfoDialog)
{
    ui->setupUi(this);

    // Set shot attribute
    shot = inShot;

    // Set ui values
    ui->shotNameLabel->setText(QString("Shot Name: " + shot->name));
    ui->targetLatLonLineEdit->setText(shot->latLon);
    ui->launchAngleineEdit->setText(QString::number(shot->launchAngle));
    ui->exitValocityLineEdit->setText(QString::number(shot->exitVelocity)); // End product didn't have variable exit velocity so I just put this hardcoded
    ui->impactDistanceLineEdit->setText(QString::number(shot->impactDistance));
    ui->compassDirectionSpinBox->setValue(shot->compassReading);

}

ShotInfoDialog::~ShotInfoDialog()
{
    delete ui;
}

void ShotInfoDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button->text()=="Save"){
        // Save ui values to shot
        shot->launchAngle = ui->launchAngleineEdit->text().toDouble();
        shot->compassReading = ui->compassDirectionSpinBox->value();
        shot->impactDistance = ui->impactDistanceLineEdit->text().toDouble();
    }
}
