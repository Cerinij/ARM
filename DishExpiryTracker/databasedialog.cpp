#include "databasedialog.h"
#include "ui_databasedialog.h"
#include "dishdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QFileDialog>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QLineEdit>

DatabaseDialog::DatabaseDialog(const QMap<QString, DishInfo>& database, QWidget *parent)
    : QDialog(parent)
    , dishDatabase(database)
{
    setupUI();
    updateDishList();
}

void DatabaseDialog::setupUI()
{
    setWindowTitle("Каталог блюд");
    setMinimumWidth(600);
    setMinimumHeight(400);
    setStyleSheet(
        "QDialog { "
        "   background-color: #2b2b2b; "
        "   color: #ffffff; "
        "}"
        "QGroupBox { "
        "   color: #ffffff; "
        "   font-weight: bold; "
        "   border: 1px solid #3a3a3a; "
        "   border-radius: 5px; "
        "   margin-top: 10px; "
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "}"
    );

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Левая панель со списком блюд
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(15);
    
    QGroupBox *listGroup = new QGroupBox("Список блюд", this);
    QVBoxLayout *listGroupLayout = new QVBoxLayout(listGroup);
    
    dishList = new QListWidget(this);
    dishList->setStyleSheet(
        "QListWidget { "
        "   background-color: #3a3a3a; "
        "   color: #ffffff; "
        "   border: 1px solid #4a4a4a; "
        "   border-radius: 4px; "
        "   padding: 5px; "
        "}"
        "QListWidget::item { "
        "   padding: 8px; "
        "   border-bottom: 1px solid #4a4a4a; "
        "}"
        "QListWidget::item:selected { "
        "   background-color: #4a4a4a; "
        "   color: #ffffff; "
        "}"
        "QListWidget::item:hover { "
        "   background-color: #454545; "
        "}"
    );
    listGroupLayout->addWidget(dishList);
    leftLayout->addWidget(listGroup);

    // Кнопки управления
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);
    
    addButton = new QPushButton("Добавить", this);
    editButton = new QPushButton("Изменить", this);
    deleteButton = new QPushButton("Удалить", this);
    viewPhotoButton = new QPushButton("Просмотр фото", this);

    QString buttonStyle = 
        "QPushButton { "
        "   padding: 8px 15px; "
        "   background-color: #4a4a4a; "
        "   color: #ffffff; "
        "   border: none; "
        "   border-radius: 4px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #5a5a5a; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #3a3a3a; "
        "   color: #666666; "
        "}";

    addButton->setStyleSheet(buttonStyle);
    editButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(buttonStyle);
    viewPhotoButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(viewPhotoButton);

    leftLayout->addLayout(buttonLayout);
    mainLayout->addLayout(leftLayout);

    // Правая панель с предпросмотром фото
    QGroupBox *photoGroup = new QGroupBox("Предпросмотр фото", this);
    QVBoxLayout *rightLayout = new QVBoxLayout(photoGroup);
    rightLayout->setSpacing(10);
    
    photoPreview = new QLabel(this);
    photoPreview->setFixedSize(200, 200);
    photoPreview->setAlignment(Qt::AlignCenter);
    photoPreview->setStyleSheet(
        "QLabel { "
        "   background-color: #3a3a3a; "
        "   border: 2px solid #4a4a4a; "
        "   border-radius: 4px; "
        "   padding: 5px; "
        "   color: #888888; "
        "}"
    );
    photoPreview->setText("Выберите блюдо для просмотра фото");
    rightLayout->addWidget(photoPreview, 0, Qt::AlignCenter);
    mainLayout->addWidget(photoGroup);

    // Подключаем сигналы
    connect(addButton, &QPushButton::clicked, this, &DatabaseDialog::onAddDishClicked);
    connect(editButton, &QPushButton::clicked, this, &DatabaseDialog::onEditDishClicked);
    connect(deleteButton, &QPushButton::clicked, this, &DatabaseDialog::onDeleteDishClicked);
    connect(viewPhotoButton, &QPushButton::clicked, this, &DatabaseDialog::onViewPhotoClicked);
    connect(dishList, &QListWidget::itemSelectionChanged, this, &DatabaseDialog::onDishSelected);

    // Изначально отключаем кнопки
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);
    viewPhotoButton->setEnabled(false);
}

void DatabaseDialog::updateDishList()
{
    dishList->clear();
    for (auto it = dishDatabase.begin(); it != dishDatabase.end(); ++it)
    {
        QListWidgetItem *item = new QListWidgetItem(it.key());
        item->setData(Qt::UserRole, it.key()); // Сохраняем оригинальное название
        dishList->addItem(item);
    }
}

void DatabaseDialog::onAddDishClicked()
{
    DishDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        DishInfo info = dialog.getDishInfo();
        if (!info.name.isEmpty())
        {
            dishDatabase[info.name] = info;
            updateDishList();
            emit databaseChanged(dishDatabase);
        }
    }
}

void DatabaseDialog::onEditDishClicked()
{
    QListWidgetItem *currentItem = dishList->currentItem();
    if (!currentItem) return;

    QString oldName = currentItem->data(Qt::UserRole).toString();
    DishInfo oldInfo = dishDatabase[oldName];

    DishDialog dialog(this);
    dialog.setDishInfo(oldInfo);
    
    if (dialog.exec() == QDialog::Accepted)
    {
        DishInfo newInfo = dialog.getDishInfo();
        if (!newInfo.name.isEmpty())
        {
            // Если имя изменилось, удаляем старую запись
            if (oldName != newInfo.name)
            {
                dishDatabase.remove(oldName);
            }
            dishDatabase[newInfo.name] = newInfo;
            updateDishList();
            emit databaseChanged(dishDatabase);
        }
    }
}

void DatabaseDialog::onDeleteDishClicked()
{
    QListWidgetItem *currentItem = dishList->currentItem();
    if (!currentItem) return;

    QString name = currentItem->data(Qt::UserRole).toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение удаления",
        QString("Вы уверены, что хотите удалить блюдо \"%1\" из базы данных?").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes)
    {
        // Удаляем фотографию, если она есть
        if (!dishDatabase[name].photoPath.isEmpty())
        {
            QFile::remove(dishDatabase[name].photoPath);
        }
        
        dishDatabase.remove(name);
        updateDishList();
        emit databaseChanged(dishDatabase);
    }
}

void DatabaseDialog::onDishSelected()
{
    bool hasSelection = !dishList->selectedItems().isEmpty();
    editButton->setEnabled(hasSelection);
    deleteButton->setEnabled(hasSelection);
    viewPhotoButton->setEnabled(hasSelection);

    if (hasSelection)
    {
        QString name = dishList->currentItem()->data(Qt::UserRole).toString();
        updatePhotoPreview(dishDatabase[name].photoPath);
    } else
    {
        photoPreview->clear();
        photoPreview->setText("Выберите блюдо для просмотра фото");
    }
}

void DatabaseDialog::onViewPhotoClicked()
{
    QListWidgetItem *currentItem = dishList->currentItem();
    if (!currentItem) return;

    QString name = currentItem->data(Qt::UserRole).toString();
    QString photoPath = dishDatabase[name].photoPath;

    if (!photoPath.isEmpty())
    {
        QDialog *photoDialog = new QDialog(this);
        photoDialog->setWindowTitle("Фотография блюда: " + name);
        
        QVBoxLayout *layout = new QVBoxLayout(photoDialog);
        QLabel *photoLabel = new QLabel(photoDialog);
        
        QPixmap pixmap(photoPath);
        if (!pixmap.isNull())
        {
            // Масштабируем изображение, чтобы оно поместилось на экране
            QSize screenSize = QApplication::primaryScreen()->size();
            pixmap = pixmap.scaled(screenSize * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            photoLabel->setPixmap(pixmap);
        }
        
        layout->addWidget(photoLabel);
        photoDialog->exec();
        delete photoDialog;
    }
}

void DatabaseDialog::updatePhotoPreview(const QString &photoPath)
{
    if (!photoPath.isEmpty())
    {
        QPixmap pixmap(photoPath);
        if (!pixmap.isNull())
        {
            pixmap = pixmap.scaled(photoPreview->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            photoPreview->setPixmap(pixmap);
        } else
        {
            photoPreview->clear();
            photoPreview->setText("Ошибка загрузки фото");
        }
    } else
    {
        photoPreview->clear();
        photoPreview->setText("Фото отсутствует");
    }
} 
