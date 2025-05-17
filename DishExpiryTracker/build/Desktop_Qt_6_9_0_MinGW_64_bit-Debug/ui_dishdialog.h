/********************************************************************************
** Form generated from reading UI file 'dishdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DISHDIALOG_H
#define UI_DISHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DishDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *nameEdit;
    QLabel *label_2;
    QSpinBox *expiryHoursBox;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *photoPreview;
    QHBoxLayout *horizontalLayout;
    QPushButton *selectPhotoButton;
    QPushButton *removePhotoButton;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DishDialog)
    {
        if (DishDialog->objectName().isEmpty())
            DishDialog->setObjectName("DishDialog");
        DishDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(DishDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(DishDialog);
        groupBox->setObjectName("groupBox");
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName("formLayout");
        label = new QLabel(groupBox);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        nameEdit = new QLineEdit(groupBox);
        nameEdit->setObjectName("nameEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, nameEdit);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_2);

        expiryHoursBox = new QSpinBox(groupBox);
        expiryHoursBox->setObjectName("expiryHoursBox");
        expiryHoursBox->setMinimum(1);
        expiryHoursBox->setMaximum(9999);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, expiryHoursBox);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(DishDialog);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        photoPreview = new QLabel(groupBox_2);
        photoPreview->setObjectName("photoPreview");
        photoPreview->setMinimumSize(QSize(0, 150));
        photoPreview->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(photoPreview);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        selectPhotoButton = new QPushButton(groupBox_2);
        selectPhotoButton->setObjectName("selectPhotoButton");

        horizontalLayout->addWidget(selectPhotoButton);

        removePhotoButton = new QPushButton(groupBox_2);
        removePhotoButton->setObjectName("removePhotoButton");

        horizontalLayout->addWidget(removePhotoButton);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(groupBox_2);

        buttonBox = new QDialogButtonBox(DishDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DishDialog);

        QMetaObject::connectSlotsByName(DishDialog);
    } // setupUi

    void retranslateUi(QDialog *DishDialog)
    {
        DishDialog->setWindowTitle(QCoreApplication::translate("DishDialog", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \320\261\320\273\321\216\320\264\320\276", nullptr));
        groupBox->setTitle(QCoreApplication::translate("DishDialog", "\320\230\320\275\321\204\320\276\321\200\320\274\320\260\321\206\320\270\321\217 \320\276 \320\261\320\273\321\216\320\264\320\265", nullptr));
        label->setText(QCoreApplication::translate("DishDialog", "\320\235\320\260\320\267\320\262\320\260\320\275\320\270\320\265:", nullptr));
        label_2->setText(QCoreApplication::translate("DishDialog", "\320\241\321\200\320\276\320\272 \320\263\320\276\320\264\320\275\320\276\321\201\321\202\320\270 (\321\207\320\260\321\201\321\213):", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("DishDialog", "\320\244\320\276\321\202\320\276\320\263\321\200\320\260\321\204\320\270\321\217", nullptr));
        photoPreview->setText(QCoreApplication::translate("DishDialog", "\320\235\320\265\321\202 \321\204\320\276\321\202\320\276\320\263\321\200\320\260\321\204\320\270\320\270", nullptr));
        selectPhotoButton->setText(QCoreApplication::translate("DishDialog", "\320\222\321\213\320\261\321\200\320\260\321\202\321\214 \321\204\320\276\321\202\320\276", nullptr));
        removePhotoButton->setText(QCoreApplication::translate("DishDialog", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214 \321\204\320\276\321\202\320\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DishDialog: public Ui_DishDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DISHDIALOG_H
