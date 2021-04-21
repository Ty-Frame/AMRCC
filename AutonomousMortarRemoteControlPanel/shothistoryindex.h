#ifndef SHOTHISTORYINDEX_H
#define SHOTHISTORYINDEX_H

#include <QWidget>
//#include "shotinfo.h"
#include "shotinfodialog.h"

namespace Ui {
class ShotHistoryIndex;
}

class ShotHistoryIndex : public QWidget
{
    Q_OBJECT

public:
    explicit ShotHistoryIndex(QWidget *parent = nullptr, QString n = "N/A", QString LL = "N/A", double LA = 0, double EV = 20.22409, double ID = 0, double CR = 0);
    explicit ShotHistoryIndex(QWidget *parent = nullptr, ShotInfo* s = nullptr);
    ~ShotHistoryIndex();

    // Checks to see if this index is locked in queue, only disables automatic deletion by shotqueuemanager. Manual deletion is not stopped
    bool isLocked();

    // Returns the shotinfo object that this index holds
    ShotInfo* getShot();

private slots:
    // Button on ui for toggling locked state
    void on_LockButton_toggled(bool checked);

    // Button on ui to set this index as the selectedShot in mainwindow
    void on_SelectButton_clicked();

    // Button on ui that opens a dialog to show detail and allows edits to the shotinfo held object
    void on_moreButton_clicked();

    // Button on ui that deletes this index
    void on_pushButton_clicked();

signals:
    // Signal to tell mainwindow that this index is the new selectedShot
    void selected(ShotInfo*);

    // Signal to tell mainwindow that this index is being deleted
    void deleted(ShotInfo*);

private:
    // Default ui object of shothistoryindex
    Ui::ShotHistoryIndex *ui;

    // Locked state of this object
    bool locked;

    // Pointer to the shotinfo object held by this index
    ShotInfo* shot;

    // Dialog for editing the shot's information
    ShotInfoDialog* dialog;
};

#endif // SHOTHISTORYINDEX_H
