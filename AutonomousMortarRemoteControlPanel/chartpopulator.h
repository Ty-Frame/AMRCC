#ifndef CHARTPOPULATOR_H
#define CHARTPOPULATOR_H

#include <QObject>
#include <QtCharts>
#include <QtMath>
#include <math.h>
#include <QDebug>
#include "shothistoryindex.h"
#include "chartsettingsdialog.h"

class ChartPopulator : public QObject
{
    Q_OBJECT
public:
    explicit ChartPopulator(QObject *parent = nullptr, QChart* chart = nullptr, ShotInfo* marker = nullptr, ShotInfo* turret = nullptr, ChartSettings* settings = nullptr);

private:
    // Function for taking needed values to generate the points needed to represent the shot's path based on formulas that can be found at https://www.desmos.com/calculator/on4xzwtdwz
    void genLine(QChart* chart, double angle, float dist, ChartSettings* settings, ShotInfo* marker, ShotInfo* turret);

    // Function that calls a python program to get the distance between the marker and turret locations
    float getDist(QString marker, QString turret);

signals:

};

#endif // CHARTPOPULATOR_H
