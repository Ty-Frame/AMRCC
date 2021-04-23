#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Area of initializing
    chartSettingsStruct = new ChartSettings();
    mapSettingsStruct = new MapSettings();
    turretInfo = nullptr;
    selectedShot = nullptr;
    bot = nullptr;
    chartSD = nullptr;
    mapSD = nullptr;
    SaveActive = SaveTypes::None;
    ui->MapImage->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    // Size policy's for the queue and tab widget so the queue doesn't take up half the screen when it doesn't need to
    QSizePolicy spLeft(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spLeft.setHorizontalStretch(1);
    ui->scrollArea->setSizePolicy(spLeft);

    QSizePolicy spRight(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spRight.setHorizontalStretch(2);
    ui->tabWidget->setSizePolicy(spRight);

    // Set default save file name
    ui->lineEdit_2->setText("SavedShots-" + QDateTime().currentDateTime().toString().replace(" ","-").replace(":","-"));

    // Create and setup chart for 2D chart display
    chart = new QChart();
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->setTitle("Projectile Path for Selected Shot");
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Distance (m)");
    chart->addAxis(axisX, Qt::AlignBottom);
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Height (m)");
    chart->addAxis(axisY, Qt::AlignLeft);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->gridLayout_5->addWidget(chartView);

    // Connect Statements for actions
    connect(ui->actionMarker,SIGNAL(triggered()),this,SLOT(satCountMarker()));
    connect(ui->actionTurret,SIGNAL(triggered()),this,SLOT(satCountTurret()));
    connect(ui->actionMap,SIGNAL(triggered()),this,SLOT(openMapSettings()));
    connect(ui->action2D_Chart,SIGNAL(triggered()),this,SLOT(openChartSettings()));
    connect(ui->actionAutomatically,SIGNAL(triggered()),this,SLOT(GetTurretLocation()));
    connect(ui->actionAutomatically_2,SIGNAL(triggered()),this,SLOT(GetMarkerLocation()));
    connect(ui->actionManually,SIGNAL(triggered()),this,SLOT(SetTurretLocation()));
    connect(ui->actionManually_2,SIGNAL(triggered()),this,SLOT(SetMarkerLocation()));
    connect(ui->actionFire_At_Selected_Shot,SIGNAL(triggered()),this,SLOT(FireAtSelectedShot()));
    connect(this,SIGNAL(manualAssignment(int)),this,SLOT(ManualAssignmentDialog(int)));
    connect(ui->actionReset_AMRCC,SIGNAL(triggered()),this,SLOT(Reset()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetSelectedShot(ShotInfo* shot)
{

    // Set selected Shot object
    selectedShot = shot;
    chart->removeAllSeries();
    if(turretInfo!=nullptr){
        ChartPopulator* pop = new ChartPopulator(this, chart, shot, turretInfo, chartSettingsStruct);
        delete pop;
    }
    UpdateMapView(nullptr);
}

void MainWindow::UpdateMapView(ShotInfo* in)
{

    // Make sure the turret has been set and that the in shot is not empty or null
    if(turretInfo==nullptr)return;
    if(in != nullptr && in->name == "" && in->latLon == "") delete in;

    // Set the selectedSHot back to null if the in shot is the selectedShot
    if(selectedShot!=nullptr && in != nullptr && in->latLon == selectedShot->latLon){
        selectedShot=nullptr;
    }

    // Start the markers string with the turret infor
    QString markers("pin-l-t+000(" + turretInfo->latLon + ")");

    // If the selectedShot isn't null, add it as a red marker
    if(selectedShot!=nullptr){
        markers = markers + ",pin-l-" + selectedShot->name + "+cd0000(" + selectedShot->latLon + ")";
    }

    // Loop through the queue adding shot information to the markers string
    if(ui->verticalLayout->count()>1){
        for (int i =0; i<ui->verticalLayout->count()-1; i++) {
            if(selectedShot == nullptr || static_cast<ShotHistoryIndex*>(ui->verticalLayout->itemAt(i)->widget())->getShot()->latLon != selectedShot->latLon){
                if(in == nullptr || in->latLon != static_cast<ShotHistoryIndex*>(ui->verticalLayout->itemAt(i)->widget())->getShot()->latLon){
                    markers = markers + ",pin-l-" + static_cast<ShotHistoryIndex*>(ui->verticalLayout->itemAt(i)->widget())->getShot()->name + "+000(" + static_cast<ShotHistoryIndex*>(ui->verticalLayout->itemAt(i)->widget())->getShot()->latLon + ")";
                }
            }
        }

    }

    // Set where the image is centered (on the turret)
    markers = markers + "/" + turretInfo->latLon;

    // Token for mapbox: pk.eyJ1IjoianRmMDAxMyIsImEiOiJja2p6MWhoN2swMHVjMndwZXpoNmxkdGNtIn0.8EWwI1zAXTe69dVdz0tYJQ
    QProcess proc;
    QString cmd("curl \"https://api.mapbox.com/styles/v1/mapbox/" + mapSettingsStruct->style_id + "/static/" + markers + "," + mapSettingsStruct->zoom + "," + mapSettingsStruct->bearing + "," + mapSettingsStruct->pitch + "/1280x1280?access_token=pk.eyJ1IjoianRmMDAxMyIsImEiOiJja2p6MWhoN2swMHVjMndwZXpoNmxkdGNtIn0.8EWwI1zAXTe69dVdz0tYJQ\" --output mapbox-static.png");
    proc.start(cmd);

    // Wait for process to finish
    while(!proc.waitForFinished());

    // Pull static image from files and put it on the display widget
    QPixmap* mapImage = new QPixmap("mapbox-static.png");
    ui->MapImage->setScaledContents(mapSettingsStruct->scaledContents);
    ui->MapImage->setPixmap(*mapImage);
}

void MainWindow::UpdateMapView()
{
    // For allowing connect statements that don't use signals giving a ShotInfo* meaning no shot is being looked over
    UpdateMapView(nullptr);
}

void MainWindow::UpdateChart()
{

    // If there is a selected shot clear the chart and then let chartpopulator make a new line for itd
    if(selectedShot!=nullptr){
        chart->removeAllSeries();
        ChartPopulator* pop = new ChartPopulator(this, chart, selectedShot, turretInfo, chartSettingsStruct);
        delete pop;
        return;
    }
}

void MainWindow::on_checkBox_2_toggled(bool checked)
{

    // Set save mode to QueueSave if check == true, else set save mode to None
    if(checked)SaveActive=QueueSave;
    else SaveActive=None;

    // Will set the filename line edit to read only while a save option is toggled on
    ui->lineEdit_2->setReadOnly(checked);

    // Make other checkbox's uncheckable while this one is checked
    ui->checkBox->setCheckable(!checked);
    ui->checkBox_3->setCheckable(!checked);
}

void MainWindow::on_checkBox_toggled(bool checked)
{

    // Set save mode to InputSave if check == true, else set save mode to None
    if(checked)SaveActive=InputSave;
    else SaveActive=None;

    // Will set the filename line edit to read only while a save option is toggled on
    ui->lineEdit_2->setReadOnly(checked);

    // Make other checkbox's uncheckable while this one is checked
    ui->checkBox_2->setCheckable(!checked);
    ui->checkBox_3->setCheckable(!checked);
}

void MainWindow::on_checkBox_3_toggled(bool checked)
{

    // Set save mode to QueueAndInputSave if check == true, else set save mode to None
    if(checked)SaveActive=QueueAndInputSave;
    else SaveActive=None;

    // Will set the filename line edit to read only while a save option is toggled on
    ui->lineEdit_2->setReadOnly(checked);

    // Make other checkbox's uncheckable while this one is checked
    ui->checkBox->setCheckable(!checked);
    ui->checkBox_2->setCheckable(!checked);
}

void MainWindow::on_pushButton_2_clicked()
{

    // Kill RCCBot if it is already in progress
    if(bot!=nullptr){
        bot->write("end\n");
        while(bot->waitForFinished());
        bot = nullptr;
    }

    // Clear window of any previous chat
    ui->textEdit->clear();

    // Make sure there is a token in the line edit, RCCBot will handle if the token is bad. It just won't be able to login
    if(ui->lineEdit_3->text()==""){
        QString temp = "No discord token given!";
        QMessageBox::warning( this, "AMRCC",
            tr(temp.toStdString().c_str()),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return;
    }

    // Take information from line edit, clear it, and then turn it inot a byte array to be sent to the qprocess start call for RCCBot
    QString str1 = ui->lineEdit_3->text();
    ui->lineEdit_3->clear();
    QByteArray ba = str1.toLocal8Bit();

    // Start RCCBot and set up as needed to allow communication between mainwindow and the process
    bot = new QProcess(this);
    bot->setWorkingDirectory(qApp->applicationDirPath());
    connect(bot,SIGNAL(readyReadStandardOutput()),this,SLOT(handleMessage()));
    bot->setProcessChannelMode(QProcess::MergedChannels);
    bot->setCurrentWriteChannel(QProcess::StandardOutput);
    bot->setCurrentReadChannel(QProcess::StandardOutput);
    bot->setReadChannel(QProcess::StandardOutput);
    bot->start("py -3 " + qApp->applicationDirPath() + "/RCCBot.py " + str1);

}

void MainWindow::handleMessage()
{

    // Read standard output to get RCCBot's message that it is passing along
    QByteArray message = bot->readAllStandardOutput();

    // Append message to the chat display
    ui->textEdit->append(QString(message));

    // See what this message is about
    if(QString(message).indexOf("Response To: Get Position")!=-1){

        // Parsing
        QString temp = "Answer: ((";
        int j = QString(message).indexOf(temp) + temp.length();
        QString sub = message.mid(j,QString(message).indexOf("), ")-j);

        // See who it is from
        if(QString(message).indexOf("Turret") != -1){

            // If turret's location is not found warn the user
            if(QString(message).indexOf("Location Not Found")!=-1){
                QMessageBox::warning( this, "AMRCC",
                                                                            tr("Turret location not found!\n"),
                                                                            QMessageBox::Ok,
                                                                            QMessageBox::Ok);
                return;
            }

            // Make new ShotInfo object for turretInfo if it has not already been made and then fill out needed information
            if(turretInfo==nullptr){
                turretInfo = new ShotInfo();
            }
            turretInfo->name = "t";
            turretInfo->latLon = sub;

            // Update map given new turret
            UpdateMapView(nullptr);

            // Save if SaveActive is not None
            if(SaveActive!=None){
                SaveShots(1);
            }
        }
        else if(QString(message).indexOf("Marker") != -1){
            // If marker's location is not found warn the user
            if(QString(message).indexOf("Location Not Found")!=-1){
                QMessageBox::warning( this, "AMRCC",
                                                                            tr("Marker location not found!\n"),
                                                                            QMessageBox::Ok,
                                                                            QMessageBox::Ok);
                return;
            }

            // Create shotqueuemanager to add the shot
            ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
            m->addShot(sub);
            delete m;

            // Update map view given new marker
            UpdateMapView(nullptr);

            // Save if SaveAcive is not None
            if(SaveActive!=None){
                SaveShots(2);
            }
        }

    }
    else if(QString(message).indexOf("Response To: Fire At")!=-1){

        // Parsing
        QString temp = "Answer: Marker is ";
        QString temp2 = " at angle ";
        QString temp3 = " in progress.";
        QString location = QString(message).mid(QString(message).indexOf(temp)+temp.length(),QString(message).indexOf(temp2)-(QString(message).indexOf(temp)+temp.length()));
        QString angle = QString(message).mid(QString(message).indexOf(temp2)+temp2.length(),QString(message).indexOf(temp3)-(QString(message).indexOf(temp2)+temp2.length()));

        // Create shotqueuemanager to update the angle of the marker that is being shot at
        ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
        m->updateAngle(location.replace("!",","), angle);
        delete m;
    }

}

void MainWindow::GetTurretLocation()
{

    // Make sure RCCBot is running
    if(nullCheck(1))return;

    // Send prompt to RCCBot
    QString temp = "Get Turret Location\n";
    ui->textEdit->append(QString(temp));
    bot->write(temp.toLocal8Bit());
}

void MainWindow::GetMarkerLocation()
{

    // Make sure RCCBot is running
    if(nullCheck(1))return;

    // Send prompt to RCCBot
    QString temp = "Get Marker Location\n";
    bot->write(temp.toLocal8Bit());
}

void MainWindow::satCountMarker()
{

    // Make sure RCCBot is running
    if(nullCheck(1))return;

    // Send prompt to RCCBot
    QString temp = "Get Marker Satellite Count\n";
    bot->write(temp.toLocal8Bit());
}

void MainWindow::satCountTurret()
{

    // Make sure RCCBot is running
    if(nullCheck(1))return;

    // Send prompt to RCCBot
    QString temp = "Get Turret Satellite Count\n";
    bot->write(temp.toLocal8Bit());
}

void MainWindow::SetTurretLocation()
{
    // Opens the dialog to take in a location, the input parameter tells whether to set that location as the turret or as a new marker
    emit manualAssignment(1);
}

void MainWindow::SetMarkerLocation()
{
    // Opens the dialog to take in a location, the input parameter tells whether to set that location as the turret or as a new marker
    emit manualAssignment(2);
}

void MainWindow::FireAtSelectedShot()
{

    // Make sure RCCBot is running
    if(nullCheck(0))return;

    // Send prompt to RCCBot
    QString mark = selectedShot->latLon;
    QString turr = turretInfo->latLon;
    QString temp = "Command: Fire At " + mark.replace(",","!") + " From " + turr.replace(",","!") + " Location\n";
    bot->write(temp.toLocal8Bit());
}

void MainWindow::ManualAssignmentDialog(int i)
{

    // Prompt user for location
    bool ok;
    QString text = QInputDialog::getText(this, tr("Manual Assignment"),
                                         tr("Latitude/Longitude:\n--Can be easily aquired using Google Maps. Right click the location and then copy the latitude and longitude.\nIF YOU DO THIS YOU HAVE TO SWAP THE NUMBERS FOR THE CORRECT LOCATION!!!\n--Input format is expected as '{longitude},{latittude}' any deviations from this format may result in errors."), QLineEdit::Normal,
                                         "", &ok);

    // If the user gave a location and hit ok
    if (ok && !text.isEmpty()){
        if(i==1){
            // Create new turretInfo if it hasnt been created already and set the needed info
            if(turretInfo==nullptr){
                turretInfo = new ShotInfo();
            }
            turretInfo->name = "t";
            turretInfo->latLon = text;

            // Update map view given new turret
            UpdateMapView(nullptr);
        }
        else if(i==2){

            // Create shotqueuemanager to handle adding a new shot
            ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
            m->addShot(text);
            delete m;

            // Update map view given new marker
            UpdateMapView(nullptr);
        }

        // If save is active then save
        if(SaveActive!=None) SaveShots(i);
    }
}

void MainWindow::SaveShots(int i)
{

    // Make sure the save directory exists, create it if it doesn't
    QString savePath = qApp->applicationDirPath() + "/SavedShots";
    if(!QDir(savePath).exists()){
        QDir().mkdir(savePath);
    }

    // Open file giventhe savepath and filename
    QString filename = savePath + "/" + ui->lineEdit_2->text() + ".csv";
    QFile Fout(filename);

    // Detect if it exists in order to put the header if it has just been created
    bool test = Fout.exists();

    // If it doesn't open send warning and quit this function
    if(!Fout.open(QIODevice::ReadWrite | QIODevice::Append)){
        QString temp = "Was not able to open file!\nFile name: "+filename;
        QMessageBox::warning( this, "AMRCC",
            tr(temp.toStdString().c_str()),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return;
    }

    // Create text stream for writing to file, fill out header if it has just been created, along with the turretInfo if it exists
    QTextStream out(&Fout);
    if(!test){
        out<<"Name,"<<"Latitude,"<<"Longitude,"<<"Launch Angle,"<<"Exit Velocity,"<<"Impact Distance,"<<"Compass Reading\n";
        if(turretInfo!=nullptr){
            out<<turretInfo->name+","<<turretInfo->latLon+","<<QString::number(turretInfo->launchAngle)+","<<QString::number(turretInfo->exitVelocity)+","<<QString::number(turretInfo->impactDistance)+","<<QString::number(turretInfo->compassReading)+"\n";
        }
    }

    // Create shotqueuemanager for getting strings from it
    ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
    QString str;

    switch(SaveActive){
        case None:
            break;
        case QueueAndInputSave: // Start here because it does a queue save first
        case QueueSave: // Start here because it is a queue save to begin with

            // Use shotqueue manager to get string for entire queue and shove it in the text stream
            str = m->toStringAll();
            out<<str;
            if(SaveActive==QueueSave){

                // If this was just a queue save then toggle it's checkbox, the toggle will handle setting the SaveAction to None
                ui->checkBox_2->toggle();
                break;
            }

            // If it got to this point it was a QueueAndInputSave, so the "Both" checkbox is toggled followed by the "Input" checkbox to signal the queue save has completed
            ui->checkBox_3->toggle();
            ui->checkBox->toggle();
            SaveActive = InputSave;
            break;
        case InputSave:

            // The call for this function has a parameter that says if this is being called for a turret or marker
            if(i==1){
                out<<turretInfo->name+","<<turretInfo->latLon+","<<QString::number(turretInfo->launchAngle)+","<<QString::number(turretInfo->exitVelocity)+","<<QString::number(turretInfo->impactDistance)+","<<QString::number(turretInfo->compassReading)+"\n";
            }
            else{
                str = m->toStringLast();
                out<<str;
            }
            break;
    }
    delete m;
    Fout.close();
}

bool MainWindow::nullCheck(int test)
{
    if(bot==nullptr){

        // If bot is null send warning and return true to signal something was found as null
        QMessageBox::warning( this, "AMRCC",
            tr("Bot has not been started yet!\n"),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return true;
    }
    else if(test == 1){

        // Test is for signaling if just the bot needs to be tested
        return false;
    }
    else if(selectedShot==nullptr){

        // If selectedShot is null send warning and return true to signal something was found as null
        QMessageBox::warning( this, "AMRCC",
            tr("No shot has been selcted!\n"),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return true;
    }
    else if(turretInfo==nullptr){

        // If turretInfo is null send warning and return true to signal something was found as null
        QMessageBox::warning( this, "AMRCC",
            tr("Turret location has not been acquired!\n"),
            QMessageBox::Ok,
            QMessageBox::Ok);
        return true;
    }
    return false;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "AMRCC",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {

        // If user really wants to quit then end the bot if it is running and then exit
        if(bot!=nullptr){
            bot->write("end\n");
            while(!bot->waitForFinished());
            bot = nullptr;
        }
        event->accept();
    }
}

void MainWindow::on_pushButton_clicked()
{
    if(ui->pushButton->text()=="Activate Save"){

        // If the user is engaging a save, change the button text
        ui->pushButton->setText("Deactivate Save");

        if(SaveActive==QueueSave){

            // If this was just a queue save, do it, and then reset the text
            SaveShots(0);
            ui->pushButton->setText("Activate Save");
        }
        else if(SaveActive!=InputSave){

            // If this is an input save it doesn't have to be called yet, call for queue and input save
            SaveShots(0);
        }
    }
    else{

        // This checkbox is the only one that can be checked when this button is clicked with the "Deactivate Save" text so toggle it and reset the text
        ui->checkBox->toggle();
        ui->pushButton->setText("Activate Save");
    }
}

void MainWindow::openChartSettings()
{

    // If this already exists close it
    if(chartSD!=nullptr){
        chartSD->close();
    }

    // Make new one and open it
    chartSD = new ChartSettingsDialog(this,chartSettingsStruct);
    chartSD->show();
    connect(chartSD,SIGNAL(updateChart()),this,SLOT(UpdateChart()));
}

void MainWindow::openMapSettings()
{

    // If this already exists close it
    if(mapSD!=nullptr){
        mapSD->close();
    }

    // Make new one and open it
    mapSD = new MapSettingsDialog(this,mapSettingsStruct);
    mapSD->show();
    connect(mapSD,SIGNAL(updateMap()),this,SLOT(UpdateMapView()));
}

void MainWindow::on_importButton_clicked()
{

    // OPen file dialog so the user can select one or more files to import
    QFileDialog *dialogMe = new QFileDialog(this, tr("Open Files"),(qApp->applicationDirPath() + "/SavedShots").toStdString().c_str(),tr("CSV Files (*.csv)"));
    dialogMe->show();

    if(dialogMe->exec()){

        // Get selected files list
        QStringList files = dialogMe->selectedFiles();

        // Loop through each one
        for(int i = 0; i<files.length(); i++){

            // Open this file
            QFile Fin(files[i]);

            if(Fin.open(QIODevice::ReadOnly | QIODevice::Text)){

                // If it opens make a textstream
                QTextStream in(&Fin);

                // Skip first line
                in.readLine();
                QString line;

                while(!in.atEnd()){

                    line = in.readLine();

                    // Split line on commas
                    QStringList spots = line.split(",");
                    if(spots[0]=="t"){

                        // If this line is the turret set the turretInfo with the needed infomration
                        if(turretInfo==nullptr){
                            turretInfo = new ShotInfo();
                        }
                        turretInfo->name = "t";
                        turretInfo->latLon = spots[1]+","+spots[2];
                    }
                    else{

                        // If this is a marker let shotqueemanager add another shot, this time settings resizeOA to true so that is the file is importing more than the queue size then it doesn't have to ask everytime if you ewant to resize it by 1
                        ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
                        m->resizeOnAdd(true);
                        m->addShot(spots[1]+","+spots[2],spots[3].toDouble(),spots[4].toDouble(),spots[5].toDouble(),spots[6].toInt());
                        delete m;
                    }
                }
            }
            else{

                // Couldn't open file
                QString temp = "Was not able to open file!\nFile name: "+files[i];
                QMessageBox::warning( this, "AMRCC",
                    tr(temp.toStdString().c_str()),
                    QMessageBox::Ok,
                    QMessageBox::Ok);
            }
        }

        // Update map view
        UpdateMapView(nullptr);
    }
}

void MainWindow::on_resizeButton_clicked()
{

    // Let shotqueuemanager resize the queue
    ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
    m->resize(ui->spinBox->value());
    delete m;
    UpdateMapView();
}

void MainWindow::on_appendButton_clicked()
{

    // Open file dialog to let the user choose which file to append to
    QFileDialog *dialogMe = new QFileDialog(this, tr("Open File"),(qApp->applicationDirPath() + "/SavedShots").toStdString().c_str(),tr("CSV Files (*.csv)"));
    dialogMe->show();

    if(dialogMe->exec()){
        QStringList files = dialogMe->selectedFiles();

        // Make sure only one file was selected
        if(files.length()!=1){
            QString temp = "Error: Either no file was selected or more than one was selected!";
            QMessageBox::warning( this, "AMRCC",
                tr(temp.toStdString().c_str()),
                QMessageBox::Ok,
                QMessageBox::Ok);
            return;
        }

        // Set the filename line edit to the filename selected
        QString file = files[0].split("/").last().split(".").first();
        ui->lineEdit_2->setText(file);
    }
}

void MainWindow::Reset()
{

    //
    QString temp = "Are you sure you want to reset the Autonomous Mortar Remote Control Center?\nThis will reset the following:\n\tCurrent Turret Information\n\tCurrent Selected Shot\n\tShot Queue\n\t2D Chart\n\tMap\n\tSave File Filename\n\tSave File State\n\tSettings for Map and Chart";
    QMessageBox::StandardButton resBtn = QMessageBox::question( (QWidget*)this->parent(), "AMRCC",
                                                                temp.toStdString().c_str(),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        return;
    } else {

        // Reset turretInfo
        if(turretInfo!=nullptr){
            delete turretInfo;
            turretInfo = nullptr;
        }

        // Reset RCCBot
        if(bot!=nullptr){
            bot->write("end\n");
            while(bot->waitForFinished());
            bot = nullptr;
        }

        // Reset selectedShot
        if(selectedShot!=nullptr){
            // No need to delete because shotqueuemanager will handle it when it clears the queue
            selectedShot = nullptr;
        }

        // Reset Map
        ui->MapImage->clear();

        // Reset Chart
        chart->removeAllSeries();

        // Reset Queue
        ShotQueueManager* m = new ShotQueueManager(this,ui->verticalLayout,ui->spinBox);
        m->clear();
        delete m;

        // Reet default save file name
        ui->lineEdit_2->setText("SavedShots-" + QDateTime().currentDateTime().toString().replace(" ","-").replace(":","-"));

        // Reset save file state
        if(ui->checkBox->isChecked()){
            ui->checkBox->setChecked(false);
        }
        else if(ui->checkBox_2->isChecked()){
            ui->checkBox_2->setChecked(false);
        }
        else if(ui->checkBox_3->isChecked()){
            ui->checkBox_3->setChecked(false);
        }

        // Reset the map settings
        if(mapSettingsStruct!=nullptr){
            delete mapSettingsStruct;
            mapSettingsStruct = new MapSettings();
        }

        // Reset the chart settings
        if(chartSettingsStruct!=nullptr){
            delete chartSettingsStruct;
            chartSettingsStruct = new ChartSettings();
        }

        // Clear RCCBot chat window of any previous chat
        ui->textEdit->clear();
    }

}
