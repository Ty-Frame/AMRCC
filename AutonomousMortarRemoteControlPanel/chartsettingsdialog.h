#ifndef CHARTSETTINGSDIALOG_H
#define CHARTSETTINGSDIALOG_H

#include <QDialog>
#include <QAbstractButton>

// Structure to hold all of the needed attributes that could change the behavior/appearance of the 2D Chart
struct ChartSettings{
    int divisions = 1;
    bool infoOutput = false;
};

namespace Ui {
class ChartSettingsDialog;
}

class ChartSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChartSettingsDialog(QWidget *parent = nullptr, ChartSettings* s = nullptr);
    ~ChartSettingsDialog();

private slots:
    // This check box controls whether the divisions spinbox is read only or not
    // Checked makes the division spinbox editable
    // Unchecking makes the division spinbox read only and resets it back to 1
    void on_checkBox_toggled(bool checked);

    // This function sees which button was pressed. If that button is "OK" then save needed information
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    // Default ui pointer for this dialog
    Ui::ChartSettingsDialog *ui;

    // Chart settings that can be edited by this dialog
    ChartSettings* settings;

signals:
    // Signal to tell the mainwindow to update its chart with the new settings
    void updateChart();
};

#endif // CHARTSETTINGSDIALOG_H
