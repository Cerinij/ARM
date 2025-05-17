/********************************************************************************
** Form generated from reading UI file 'databasedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DATABASEDIALOG_H
#define UI_DATABASEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DatabaseDialog
{
public:
    QVBoxLayout *verticalLayout;
    QListWidget *dishList;
    QHBoxLayout *horizontalLayout;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *closeButton;

    void setupUi(QDialog *DatabaseDialog)
    {
        if (DatabaseDialog->objectName().isEmpty())
            DatabaseDialog->setObjectName("DatabaseDialog");
        DatabaseDialog->resize(600, 400);
        verticalLayout = new QVBoxLayout(DatabaseDialog);
        verticalLayout->setObjectName("verticalLayout");
        dishList = new QListWidget(DatabaseDialog);
        dishList->setObjectName("dishList");

        verticalLayout->addWidget(dishList);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        addButton = new QPushButton(DatabaseDialog);
        addButton->setObjectName("addButton");

        horizontalLayout->addWidget(addButton);

        editButton = new QPushButton(DatabaseDialog);
        editButton->setObjectName("editButton");

        horizontalLayout->addWidget(editButton);

        deleteButton = new QPushButton(DatabaseDialog);
        deleteButton->setObjectName("deleteButton");

        horizontalLayout->addWidget(deleteButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        closeButton = new QPushButton(DatabaseDialog);
        closeButton->setObjectName("closeButton");

        horizontalLayout->addWidget(closeButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(DatabaseDialog);

        QMetaObject::connectSlotsByName(DatabaseDialog);
    } // setupUi

    void retranslateUi(QDialog *DatabaseDialog)
    {
        DatabaseDialog->setWindowTitle(QCoreApplication::translate("DatabaseDialog", "\320\232\320\260\321\202\320\260\320\273\320\276\320\263 \320\221\320\273\321\216\320\264", nullptr));
        addButton->setText(QCoreApplication::translate("DatabaseDialog", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214", nullptr));
        editButton->setText(QCoreApplication::translate("DatabaseDialog", "\320\240\320\265\320\264\320\260\320\272\321\202\320\270\321\200\320\276\320\262\320\260\321\202\321\214", nullptr));
        deleteButton->setText(QCoreApplication::translate("DatabaseDialog", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214", nullptr));
        closeButton->setText(QCoreApplication::translate("DatabaseDialog", "\320\227\320\260\320\272\321\200\321\213\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DatabaseDialog: public Ui_DatabaseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DATABASEDIALOG_H
