#include "chartsettingsdialog.h"
#include "ui_chartsettingsdialog.h"

ChartSettingsDialog::ChartSettingsDialog(QWidget *parent, ChartSettings* s) :
    QDialog(parent),
    ui(new Ui::ChartSettingsDialog)
{
    ui->setupUi(this);

    // Assign the settings attribute of this dialog
    settings = s;

    // Set up the divisions focused ui objects for the current value
    if(settings->divisions!=1){
        ui->checkBox->setChecked(true);
        ui->divisionsSpinBox->setValue(settings->divisions);
    }

    // Set the "output information" check box to the current state
    ui->checkBox_2->setChecked(settings->infoOutput);
}

ChartSettingsDialog::~ChartSettingsDialog()
{
    delete ui;
}

void ChartSettingsDialog::on_checkBox_toggled(bool checked)
{
    // If checkbox is toggled then make the divisions spinbox read only depending on the current state of the checkbox
    ui->divisionsSpinBox->setReadOnly(!checked);

    // If the checkbox was unchecked then reset the divisions count back to 1
    if(!checked) ui->divisionsSpinBox->setValue(1);
}

void ChartSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    // If the button used to close this window was "OK" then save the division count and output information states
    if(button->text()=="OK"){
        settings->divisions = ui->divisionsSpinBox->value();
        settings->infoOutput = ui->checkBox_2->isChecked();
        emit updateChart();
    }
}
