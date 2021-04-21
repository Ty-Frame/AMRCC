#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QAction>
#include <QProcess>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDateTime>
#include <QCloseEvent>
#include "shothistoryindex.h"
#include "shotqueuemanager.h"
#include "chartpopulator.h"
#include "chartsettingsdialog.h"
#include "mapsettingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_ENUMS(SaveTypes)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Types of save modes
    enum SaveTypes{
        None,
        QueueSave,
        InputSave,
        QueueAndInputSave
    };

    // For being able to end the RCCBot before closing the mainwindow if it's running
    void closeEvent(QCloseEvent *event);

public slots:

    // Changes the shot that it highlighted red when the map is updated
    void SetSelectedShot(ShotInfo* shot);

    // Renews the map static image, the in shot is one that is being removed so it won't show on the map
    void UpdateMapView(ShotInfo* in);

    // Renews the map static image without removing one from the map
    void UpdateMapView();

    // Refreshes the chart is there is a selected shot once the user makes any changes to the chart settings
    void UpdateChart();

private slots:
    // Changes save mode to QueueSave
    // If unselected, the save mode is set to None
    void on_checkBox_2_toggled(bool checked);

    // Changes save mode to InputSave
    // If unselected, the save mode is set to None
    void on_checkBox_toggled(bool checked);

    // Changes save mode to QueueAndInputSave
    // If unselected, the save mode is set to None
    void on_checkBox_3_toggled(bool checked);

    // Starts up the RCCBot python code
    void on_pushButton_2_clicked();

    // Reads in messages being passed from the RCCBot back to the mainwindow
    void handleMessage();

    // Uses the RCCBot to request the Turret's location
    void GetTurretLocation();

    // Uses the RCCBot to request the Marker's location
    void GetMarkerLocation();

    // Manual assignment of the turret's location
    void SetTurretLocation();

    // Manual creation of new markers in the queue and map
    void SetMarkerLocation();

    // Sends message to RCCBot that is forwarded to the TurretBot and tells it to fire from X location to Y location
    void FireAtSelectedShot();

    // Called by "SetTurret/Marker" functions and depending on i it sets does the correct action
    void ManualAssignmentDialog(int i);

    // If SaveAction is not None then it does the needed save function
    void SaveShots(int i);

    // The button that activates the save function
    void on_pushButton_clicked();

    // Opens ChartSettingsDialog to edit the chart settings
    void openChartSettings();

    // Opens MapSettingsDialog to edit the map settings
    void openMapSettings();

    // RCCBot pings the marker for the number of satellites it currently sees
    void satCountMarker();

    // RCCBot pings the turret for the number of satellites it currently sees
    void satCountTurret();

    // Imports one of the save files
    void on_importButton_clicked();

    // Activates resizing of the queue
    void on_resizeButton_clicked();

    // Opens file dialog to select a file that will be appended to by the save function
    void on_appendButton_clicked();

    // Function that will reset the AMRCC as if it just started up
    void Reset();

signals:
    void manualAssignment(int i);

private:
    // Main default ui
    Ui::MainWindow *ui;

    // Function to make sure the RCCBot, selectedShot, and the turretInfo is not null
    bool nullCheck(int test);

    // Holds the current save type selection
    SaveTypes SaveActive;

    // The shot that is held at the current marker location that is being focused on
    ShotInfo* selectedShot;

    // The turret's current location on the map
    ShotInfo* turretInfo;

    // The chart that is put in the chartview object for the 2D Chart widget tab
    QChart* chart;

    // Pointer for the RCCBot's QProcess object
    QProcess* bot;

    // Chart settings structure
    ChartSettings* chartSettingsStruct;

    // Pointer to the chart settings dialog for editing the chart settings
    ChartSettingsDialog* chartSD;

    // Map settings structure
    MapSettings* mapSettingsStruct;

    // Pointer to the map settings dialog for editing the chart settings
    MapSettingsDialog* mapSD;
};
#endif // MAINWINDOW_H
