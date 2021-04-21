#ifndef SHOTINFO_H
#define SHOTINFO_H

#endif // SHOTINFO_H

#include <QString>

// Default shotinfo object information for this structure
struct ShotInfo{
    QString name = "N/A";
    QString latLon = "N/A";
    double launchAngle = 0;
    double exitVelocity = 20.22409;
    double impactDistance = 0;
    int compassReading = 0;
};
