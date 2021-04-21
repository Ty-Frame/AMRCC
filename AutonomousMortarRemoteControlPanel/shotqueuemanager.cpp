#include "shotqueuemanager.h"

ShotQueueManager::ShotQueueManager(QObject *parent, QVBoxLayout* v, QSpinBox* s) : QObject(parent)
{

    // Assign attributes
    indexDisplay = v;
    queueSize = s;
}

void ShotQueueManager::addShot(QString LL, double LA, double EV, double ID, int CR)
{

    if(indexDisplay->count()-1==queueSize->value() && !resizeOA){

        // Only put out question if the queue is full and the user is not adding shots while not importing
        QMessageBox::StandardButton resBtn = QMessageBox::question( (QWidget*)this->parent(), "Shot Queue Manager",
                                                                    tr("Shot queue is full! If you proceed the oldest shot that is unlocked will be removed. Are you sure you want to proceed?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes) {
            return;
        } else {

            // If user says to delete an index to make room then remove one. If it didn't work then return
            bool worked = removeShot();
            if(!worked) return;
        }
    }
    else if(indexDisplay->count()-1==queueSize->value() && resizeOA){

        // If resizeOA is true then just increment the queue size value
        queueSize->setValue(queueSize->value()+1);
    }

    // Get available name for the new shot
    QString name = getAvailableName();

    // Make new shot and fill informtion
    ShotInfo* temp = new ShotInfo();
    temp->name = name;
    temp->latLon = LL;
    temp->launchAngle = LA;
    temp->exitVelocity = EV;
    temp->impactDistance = ID;
    temp->compassReading = CR;

    // Make index for this shot, make necessary connects, and add it to the queue
    ShotHistoryIndex* hold = new ShotHistoryIndex((QWidget*)this->parent(),temp);
    connect(hold,SIGNAL(deleted(ShotInfo*)),this->parent(),SLOT(UpdateMapView(ShotInfo*)));
    connect(hold,SIGNAL(selected(ShotInfo*)),this->parent(),SLOT(SetSelectedShot(ShotInfo*)));
    indexDisplay->insertWidget(0,hold);
}

void ShotQueueManager::resizeOnAdd(bool b)
{
    resizeOA = b;
}

void ShotQueueManager::resize(int i)
{
    bool worked;
    if(i>=indexDisplay->count()-1){
        // Do nothing if the count is less than the new queue size
        return;
    }
    do{
        // Delete shots until the number of indexes in queue is less than the new queue size
        worked = removeShot();
    }while(worked && i<indexDisplay->count()-1);
}

bool ShotQueueManager::removeShot()
{

    // Loop through older shots first (the ones lower in the queue), if not locked then delete it and return true
    for(int i = indexDisplay->count()-2; i>-1; i--){
        ShotHistoryIndex* hold = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(i)->widget());
        if(!(hold->isLocked())){
            indexDisplay->removeWidget(hold);
            delete hold;
            return true;
        }
    }

    // If all shots were locked ask if the user wants to remove a locked shot
    QMessageBox::StandardButton resBtn = QMessageBox::question( (QWidget*)this->parent(), "Shot Queue Manager",
                                                                tr("All shots are locked and could not be removed. Would you like to remove the oldest shot that is locked?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        return false;
    } else {
        // Delete the oldest locked shot
        ShotHistoryIndex* temp = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(indexDisplay->count()-2)->widget());
        indexDisplay->removeWidget(temp);
        delete temp;
        return true;
    }
    return false;
}

QString ShotQueueManager::toStringAll()
{

    // Loop through all indexes, append all info to string for .csv format, return string. For Queue and QueueAndInput saves
    QString hold;
    if(indexDisplay->count()>1){
        ShotInfo* temp;
        for(int i = indexDisplay->count()-2; i>-1; i--){
            temp = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(i)->widget())->getShot();
            hold.append(temp->name+","+temp->latLon+","+QString::number(temp->launchAngle)+","+QString::number(temp->exitVelocity)+","+QString::number(temp->impactDistance)+","+QString::number(temp->compassReading)+"\n");
        }
    }
    return hold;
}

QString ShotQueueManager::toStringLast()
{

    // Return the last shot's info in .csv format. For input saves
    QString hold;
    if(indexDisplay->count()>1){
        ShotInfo* temp;
        temp = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(0)->widget())->getShot();
        hold.append(temp->name+","+temp->latLon+","+QString::number(temp->launchAngle)+","+QString::number(temp->exitVelocity)+","+QString::number(temp->impactDistance)+","+QString::number(temp->compassReading)+"\n");
    }
    return hold;
}

void ShotQueueManager::updateAngle(QString location, QString angle)
{

    // Make sure the queue isnt empty
    if(indexDisplay->count()-1==0){
        QMessageBox::warning( (QWidget*)this->parent(), "RCC",
                                                                    tr("There are no shots in queue and can't be updated!\n"),
                                                                    QMessageBox::Ok,
                                                                    QMessageBox::Ok);
        return;
    }


    // Loop through indexes, if found set the new angle
    for(int i = 0; i<indexDisplay->count()-1; i++){
        if(static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(i)->widget())->getShot()->latLon==location){
            static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(i)->widget())->getShot()->launchAngle = angle.toDouble();
            return;
        }
    }


    // Warn user if the shot wasnt able to be found
    QString temp = "There are no shots that matched the location!\nLocation: " + location;
    QMessageBox::warning( (QWidget*)this->parent(), "RCC",
                                                                tr(temp.toStdString().c_str()),
                                                                QMessageBox::Ok,
                          QMessageBox::Ok);
}

void ShotQueueManager::clear()
{

    // Get current index display size
    int count = indexDisplay->count()-1;
    for(int i = 0; i<count; i++){

        // Delete all indexes in queue
        ShotHistoryIndex* hold = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(0)->widget());
        indexDisplay->removeWidget(hold);
        delete hold;
    }

    // Set the queue size back to the standard 10
    queueSize->setValue(10);
}

QString ShotQueueManager::getAvailableName()
{

    // Return 1 if the list is empty
    if(indexDisplay->count()-1==0) return QString::number(1);

    // Make array that is as long as the queue's current max size
    int len = queueSize->value();
    int* arr = new int[len];

    // Fill array with all names (converted to ints)
    for(int i = 0; i<indexDisplay->count()-1; i++){ // Don't go all the way to the end because of the spacer.
        arr[i] = static_cast<ShotHistoryIndex*>(indexDisplay->itemAt(i)->widget())->getShot()->name.toInt();
    }

    // Put all the names in their correct indexes (1 in 0, 2 in 1, etc)
    int current, next;
    for(int i = 0; i<indexDisplay->count()-1; i++){
        current = arr[i];
        while(0<=current-1 && current-1 < len && current != (next=arr[current-1])){
            arr[current-1] = current;
            current = next;
        }
    }

    // Whichever index doesn't have its correct value is the name we choose
    for(int i = 0; i<indexDisplay->count()-1; i++){
        if(arr[i] != i+1){
            return QString::number(i+1);
        }
    }

    // If all names are there then use the name corresponding to the queue max size
    return QString::number(indexDisplay->count());

}
