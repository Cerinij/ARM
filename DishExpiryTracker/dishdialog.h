#pragma once

#include "dishinfo.h"
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

namespace Ui
{
class DishDialog;
}

class DishDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DishDialog(QWidget *parent = nullptr);
    ~DishDialog();
    DishInfo getDishInfo() const;
    void setDishInfo(const DishInfo &info);

    QString getDishName() const;
    int getExpiryHours() const;

private slots:
    void onSelectPhotoClicked();
    void onRemovePhotoClicked();

private:
    Ui::DishDialog *ui;
    QString currentPhotoPath;

    // Виджеты
    QLineEdit *nameEdit;
    QSpinBox *expiryHoursBox;
    QLabel *photoPreview;
    QPushButton *selectPhotoButton;
    QPushButton *removePhotoButton;

    void setupUI();
    void updatePhotoPreview();
};

