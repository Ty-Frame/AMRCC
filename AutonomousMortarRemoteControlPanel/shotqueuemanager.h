#ifndef SHOTQUEUEMANAGER_H
#define SHOTQUEUEMANAGER_H

#include <QDebug>
#include <QObject>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSpinBox>
#include "shothistoryindex.h"

class ShotQueueManager : public QObject
{
    Q_OBJECT
public:
    explicit ShotQueueManager(QObject *parent = nullptr, QVBoxLayout* v = nullptr, QSpinBox* s = nullptr);

    // Function for adding shot to the shot queue
    void addShot(QString targetLL = "N/A", double LA = 0, double EV = 20.22409, double ID = 0, int CR = 0);

    // Setter for the resizeOA boolean variable. Only used when importing files in case the importing file is bigger than the current queue
    void resizeOnAdd(bool b);

    // Function for resizing the shot queue, really only deletes shots as needed
    void resize(int i);

    // Funciton for removing a shotindex from the queue
    bool removeShot();

    // Returns the entire queue as a string for file export purposes
    QString toStringAll();

    // Returns the last shot in queue for the InputSave save mode
    QString toStringLast();

    // Updates the firing angle of a shot when it's returned from the TurretBot
    void updateAngle(QString location, QString angle);

    // Function for clearing the queue on reset
    void clear();

private:
    // Function for getting a name that is not currently used in the queue
    QString getAvailableName();

    // The layout that is the queue
    QVBoxLayout* indexDisplay;

    // The spinbox on the mainwindow's ui that tells the queue's size
    QSpinBox* queueSize;
    bool resizeOA = false;
};

#endif // SHOTQUEUEMANAGER_H
