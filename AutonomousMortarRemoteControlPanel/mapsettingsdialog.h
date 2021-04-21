#ifndef MAPSETTINGSDIALOG_H
#define MAPSETTINGSDIALOG_H

#include <QDialog>
#include <QAbstractButton>

struct MapSettings{
    bool scaledContents = false;
    QString style_id = "streets-v11";
    QString zoom = "19.5";
    QString bearing = "0";
    QString pitch = "0";
};

namespace Ui {
class MapSettingsDialog;
}

class MapSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapSettingsDialog(QWidget *parent = nullptr, MapSettings* s = nullptr);
    ~MapSettingsDialog();

private slots:
    // For detecting which button is pressed. If that button is "OK" then save the necessary things
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    // Default dialog ui
    Ui::MapSettingsDialog *ui;

    // Settings that the dialog is editing
    MapSettings* settings;

signals:
    // Signal to tell the mainwindow to update its map with the new settings
    void updateMap();
};

#endif // MAPSETTINGSDIALOG_H
