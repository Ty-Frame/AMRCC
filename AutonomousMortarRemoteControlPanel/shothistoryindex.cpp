#include "shothistoryindex.h"
#include "ui_shothistoryindex.h"

ShotHistoryIndex::ShotHistoryIndex(QWidget *parent, QString n, QString LL, double LA, double EV, double ID, double CR) :
    QWidget(parent),
    ui(new Ui::ShotHistoryIndex)
{

    // Make new shotinfo object and fill information
    shot = new ShotInfo();
    shot->name = n;
    shot->latLon = LL;
    shot->launchAngle = LA;
    shot->exitVelocity = EV;
    shot->impactDistance = ID;
    shot->compassReading = CR;

    ui->setupUi(this);

    // Set values and variables as needed
    ui->label->setText(shot->name);
    dialog = nullptr;
    locked = false;
}

ShotHistoryIndex::ShotHistoryIndex(QWidget *parent, ShotInfo *s) :
    QWidget(parent),
    ui(new Ui::ShotHistoryIndex)
{

    // Set shot
    shot = s;

    ui->setupUi(this);

    // Set values and variables as needed
    ui->label->setText(shot->name);
    dialog = nullptr;
    locked = false;
}

ShotHistoryIndex::~ShotHistoryIndex()
{
    delete ui;
}

bool ShotHistoryIndex::isLocked()
{
    return locked;
}

ShotInfo* ShotHistoryIndex::getShot()
{
    return shot;
}

void ShotHistoryIndex::on_LockButton_toggled(bool checked)
{
    locked = checked;
}

void ShotHistoryIndex::on_SelectButton_clicked()
{

    // Tell mainwindow that this shot has been selected
    emit selected(shot);
}

void ShotHistoryIndex::on_moreButton_clicked()
{

    // Close dialog if it already exists, make a new one, and show it
    if(dialog!=nullptr){
        dialog->close();
    }

    dialog = new ShotInfoDialog(nullptr,shot);
    dialog->show();
}

void ShotHistoryIndex::on_pushButton_clicked()
{

    // Tell mainwindow to update the map without this shot and then delete itself
    emit deleted(this->shot);
    delete this;
}
