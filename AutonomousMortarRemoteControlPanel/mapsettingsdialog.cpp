#include "mapsettingsdialog.h"
#include "ui_mapsettingsdialog.h"

MapSettingsDialog::MapSettingsDialog(QWidget *parent, MapSettings* s) :
    QDialog(parent),
    ui(new Ui::MapSettingsDialog)
{
    ui->setupUi(this);

    // Set the settings attribute and set ui values as needed
    settings = s;
    ui->checkBox->setChecked(settings->scaledContents);
    ui->doubleSpinBox->setValue(settings->zoom.toFloat());
    ui->bearingSpinBox->setValue(settings->bearing.toInt());
    ui->pitchSpinBox->setValue(settings->pitch.toInt());
    ui->comboBox->setCurrentText(settings->style_id);
}

MapSettingsDialog::~MapSettingsDialog()
{
    delete ui;
}

void MapSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button->text()=="OK"){

        // Save values and then tell mainwindow to update the map
        settings->scaledContents = ui->checkBox->isChecked();
        settings->zoom = QString::number(ui->doubleSpinBox->value());
        settings->pitch = QString::number(ui->pitchSpinBox->value());
        settings->bearing = QString::number(ui->bearingSpinBox->value());
        settings->style_id = ui->comboBox->currentText();
        emit updateMap();
    }
}
