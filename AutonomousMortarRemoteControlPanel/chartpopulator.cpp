#include "chartpopulator.h"

ChartPopulator::ChartPopulator(QObject *parent, QChart* chart, ShotInfo* marker, ShotInfo* turret, ChartSettings* settings) : QObject(parent)
{

    // If launch angle for this shot has not been set then don't try to generate a line for the chart
    if(marker->launchAngle==0){
        QString temp("Shot " + marker->name + " has no launch angle and cannot be graphed in the 2D chart!\n");
        QMessageBox::warning( (QWidget*)this->parent(), "RCC",
                                                                    tr(temp.toStdString().c_str()),
                                                                    QMessageBox::Ok,
                                                                    QMessageBox::Ok);
        return;
    }

    // Get the distance from the marker to the turret
    float dist = getDist(marker->latLon,turret->latLon);

    // Generate the line and add it to the chart, adjust the chart as needed
    genLine(chart, marker->launchAngle, dist, settings, marker, turret);

}

float ChartPopulator::getDist(QString marker, QString turret)
{

    // Create the QProcess object and set the write and read channel modes so that it can return a value to this object
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setCurrentWriteChannel(QProcess::StandardOutput);
    proc.setCurrentReadChannel(QProcess::StandardOutput);
    proc.setReadChannel(QProcess::StandardOutput);

    // Start the distance.py script with the input of the two locations in order to get the distance
    proc.start("py -3 " + qApp->applicationDirPath() + "/distance.py " + marker + "!" + turret);

    // Wait for process to finish
    while(!proc.waitForFinished());

    // Read the standar output for this process to get the distance answer
    QString ans = proc.readAllStandardOutput();
    return ans.toFloat();
}

void ChartPopulator::genLine(QChart* chart, double angle, float dist, ChartSettings* settings, ShotInfo* marker, ShotInfo* turret)
{
    // Formulas that are in this function can be found at https://www.desmos.com/calculator/on4xzwtdwz

    // Instantiating variables needed for formulas
    int divisions = settings->divisions;
    double m = 0.145;
    double g = 9.81;
    double p = 1.225;
    double A = 0.004418; //surface area of ball on one side
    double C = 0.47;
    double initialV = 20.22409;
    double n = (M_PI/180)*angle;
    double k = 0.5*p*A*C;

    // Point of the apex
    double b1 = sqrt(m/g/k)*atan(initialV*sin(n)*sqrt(k/m/g));
    // Point of the landing
    double b2 = sqrt(m/(g*k))*(std::acosh(sqrt(k*pow(initialV,2)/(m*g)*pow(sin(n),2) + 1)) + atan(initialV*sin(n)*sqrt(k/(m*g))));

    // Formulas to calculate the apex and landing points for this given shot, not tied to the marker's location
    double apex = m/(2*k)*log(k*pow(initialV,2)*pow(sin(n),2)/(m*g)+1);
    double landing = m/k*log(k*initialV*cos(n)*b2/m+1);

    /*
     *  Explination: QtCharts did not want to plot points that were not on integer x values.
     *               This resulted in very "not smooth" trajectory lines that would be expected.
     *               Usually this is accounted for by adding more points in between, but as stated in the first sentence that was not working.
     *               To account for this I made the chart scale up based on the divisions attribute of the ChartSettings object.
     */

    // Create an array of doubles that can hold the number of points needed depending on if the landing point or the marker's location is the further of the two
    int totalPoints = int((dist>landing)?ceil(dist*divisions):ceil(landing*divisions));
    double t[totalPoints];

    // Filling out the times that each point is reached
    for(int i = 0; i<totalPoints;i++){
        t[i] = (m/(k*initialV*cos(n)))*(exp(k*(float(i)/divisions)/m)-1);
    }

    // Temp variable to hold the points that are going to be put on the chart
    QLineSeries* temp = new QLineSeries();

    // Integer variable to see when this loop stops so that the next one can pick up where it left off
    int stop;

    // For loop that goes from the launch point to the apex
    for(int i = 0; t[i]<b1; i++){
        double point = (m/k)*log(cos(float(t[i])*sqrt(g*k/m) - atan(initialV*sin(n)*sqrt(k/(m*g))))) + (m/(2*k))*log((k*pow(initialV,2))/(m*g)*pow(sin(n),2)+1);
        *temp<<QPoint(float(i),point*divisions);
        stop = i+1;
    }

    // Hold variable to make sure the points are still traveling downwards, becuase once it landed the chart wanted to show it bouncing or something like that
    double hold = -1.0;

    // For loop that goes from the apex to the landing point
    for(int i = stop; t[i]<b2; i++){
        double point = (-m/k)*log(std::cosh(float(t[i])*sqrt(g*k/m) - atan(initialV*sin(n)*sqrt(k/(m*g))))) + (m/(2*k))*log((k*pow(initialV,2))/(m*g)*pow(sin(n),2)+1);

        // If the hold vairable shows that if is doing the "bounce" thing then break from the loop becuase the line has reached the landing point
        if(hold==-1 || point*divisions<hold)hold = divisions*point;
        else break;

        *temp<<QPoint(float(i),point*divisions);
    }

    // Set the x and y boundaries
    chart->axes(Qt::Horizontal).back()->setRange(0,(totalPoints/divisions+1)*divisions);
    chart->axes(Qt::Vertical).back()->setRange(0,(apex+1)*divisions);

    // Add the temp variable to the chart
    chart->addSeries(temp);

    // Attatch each axis to this line object
    temp->attachAxis(chart->axes(Qt::Horizontal).back());
    temp->attachAxis(chart->axes(Qt::Vertical).back());

    // If the user has selected the information output enter here
    if(settings->infoOutput){

        /*
         * String that holds the information that is going to be put on the message box that tells about this shot's line
         * Has the following information:
         *  Marker name
         *  Marker location
         *  Turret location
         *  Launch Angle
         *  Division count
         *  The distance of from the turret to the marker's location
         *  The apex of the shot's flight path
         *  The landing point of the shot's flight path
         *
         * If the division count is not 1 then the distance, apex, and landing point information also has points adjusted to the division count displayed
         * beside the unadjusted values so the user can map each of them to the chart.
         */

        QString tempS = "Shot Info:\n----------\nMaker Name: " + marker->name +
                "\n----------\nMarker Location: " + marker->latLon +
                "\n----------\nTurret Location: "  + turret->latLon +
                "\n----------\nLaunch Angle: "  + QString::number(angle) +
                "\n----------\nScale: " + QString::number(divisions) +
                "\n----------\nShot Distance: " + QString::number(dist) + " meters" +((divisions==1) ? "":(" or " + QString::number(dist*divisions) + " meters based on the scale"))+
                "\n----------\nApex: ("+QString::number(m/k*log(k*initialV*cos(n)*b1/m + 1))+"m, "+QString::number(apex)+"m)"+ ((divisions==1) ? "":(" or ("+QString::number((m/k*log(k*initialV*cos(n)*b1/m + 1))*divisions)+"m, "+QString::number(apex*divisions)+"m) based on the scale"))+
                "\n----------\nLanging: ("+QString::number(landing)+"m, 0m)"+ ((divisions==1) ? "":(" or ("+QString::number(landing*divisions)+"m, 0m) based on the scale"))+
                ((divisions==1) ? "":"\n----------\nThe 'based on the scale' values are mapped to chart values.");
        QMessageBox::information((QWidget*)this->parent(), "RCC",
                                                                    tr(tempS.toStdString().c_str()),
                                                                    QMessageBox::Ok,
                                                                    QMessageBox::Ok);
    }
}
