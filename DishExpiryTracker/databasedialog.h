#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QMap>
#include "dishinfo.h"
#include <QListWidget>
#include <QLabel>

namespace Ui
{
class DatabaseDialog;
}

class DatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseDialog(const QMap<QString, DishInfo>& database, QWidget *parent = nullptr);
    QMap<QString, DishInfo> getDishDatabase() const { return dishDatabase; }

signals:
    void databaseChanged(const QMap<QString, DishInfo>& database);

private slots:
    void onAddDishClicked();
    void onEditDishClicked();
    void onDeleteDishClicked();
    void onDishSelected();
    void onViewPhotoClicked();

private:
    QMap<QString, DishInfo> dishDatabase;
    QString currentPhotoPath;

    // Виджеты
    QListWidget *dishList;
    QLabel *photoPreview;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *viewPhotoButton;

    void setupUI();
    void updateDishList();
    void updatePhotoPreview(const QString &photoPath);
};
