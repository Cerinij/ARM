#include "dishdialog.h"
#include "ui_dishdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QPixmap>
#include <QCoreApplication>

DishDialog::DishDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DishDialog)
{
    ui->setupUi(this);
    setupUI();
}

DishDialog::~DishDialog()
{
    delete ui;
}

void DishDialog::setupUI()
{
    setWindowTitle("Добавить блюдо");
    setMinimumWidth(400);
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
        "QLineEdit, QSpinBox { "
        "   padding: 8px; "
        "   background-color: #3a3a3a; "
        "   color: #ffffff; "
        "   border: 1px solid #4a4a4a; "
        "   border-radius: 4px; "
        "}"
        "QLineEdit:focus, QSpinBox:focus { "
        "   border: 1px solid #5a5a5a; "
        "}"
        "QSpinBox::up-button, QSpinBox::down-button { "
        "   background-color: #4a4a4a; "
        "   border: none; "
        "   width: 20px; "
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover { "
        "   background-color: #5a5a5a; "
        "}"
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
        "}"
        "QDialogButtonBox QPushButton { "
        "   padding: 8px 20px; "
        "   border-radius: 4px; "
        "   min-width: 80px; "
        "}"
        "QDialogButtonBox QPushButton[text='OK'] { "
        "   background-color: #2e7d32; "
        "   color: white; "
        "   border: none; "
        "}"
        "QDialogButtonBox QPushButton[text='OK']:hover { "
        "   background-color: #388e3c; "
        "}"
        "QDialogButtonBox QPushButton[text='Cancel'] { "
        "   background-color: #c62828; "
        "   color: white; "
        "   border: none; "
        "}"
        "QDialogButtonBox QPushButton[text='Cancel']:hover { "
        "   background-color: #d32f2f; "
        "}"
    );

    // Настраиваем SpinBox
    ui->expiryHoursBox->setRange(1, 720);
    ui->expiryHoursBox->setSuffix(" часов");

    // Настраиваем превью фотографии
    ui->photoPreview->setFixedSize(200, 200);
    ui->photoPreview->setAlignment(Qt::AlignCenter);
    ui->photoPreview->setText("Нет фотографии");

    // Отключаем кнопку удаления фото изначально
    ui->removePhotoButton->setEnabled(false);

    // Подключаем обработчики для кнопок фотографии
    connect(ui->selectPhotoButton, &QPushButton::clicked, this, &DishDialog::onSelectPhotoClicked);
    connect(ui->removePhotoButton, &QPushButton::clicked, this, &DishDialog::onRemovePhotoClicked);

    // Подключаем обработчики для кнопок OK и Cancel
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void DishDialog::onSelectPhotoClicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        "Выберите фотографию блюда",
        "",
        "Изображения (*.png *.jpg *.jpeg *.bmp)");

    if (!filename.isEmpty())
    {
        QString userPath = QDir::homePath();
        QDir dir(userPath + "/DishExpiryTracker/photos");
        if (!dir.exists())
        {
            if (!dir.mkpath("."))
            {
                QMessageBox::critical(this, "Ошибка",
                    "Не удалось создать папку для фотографий. Проверьте права доступа.");
                return;
            }
        }

        QFileInfo sourceFile(filename);
        QString baseName = sourceFile.baseName();
        QString extension = sourceFile.suffix();
        QString newPath;
        int counter = 1;
        
        do {
            QString fileName = counter == 1 ? 
                QString("%1.%2").arg(baseName).arg(extension) :
                QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension);
            newPath = dir.filePath(fileName);
            counter++;
        } while (QFile::exists(newPath));

        // Копируем файл
        QFile source(filename);
        QFile destination(newPath);
        
        if (source.open(QIODevice::ReadOnly) && destination.open(QIODevice::WriteOnly))
        {
            if (destination.write(source.readAll()) != -1)
            {
                currentPhotoPath = newPath;
                updatePhotoPreview();
                ui->removePhotoButton->setEnabled(true);
            } else {
                QMessageBox::critical(this, "Ошибка",
                    "Не удалось записать файл. Проверьте права доступа и свободное место на диске.");
            }
            source.close();
            destination.close();
        } else
        {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось открыть файлы для копирования. Проверьте права доступа.");
        }
    }
}

void DishDialog::onRemovePhotoClicked()
{
    if (!currentPhotoPath.isEmpty())
    {
        QFile::remove(currentPhotoPath);
        currentPhotoPath.clear();
        updatePhotoPreview();
        ui->removePhotoButton->setEnabled(false);
    }
}

void DishDialog::updatePhotoPreview()
{
    if (!currentPhotoPath.isEmpty())
    {
        QPixmap pixmap(currentPhotoPath);
        if (!pixmap.isNull())
        {
            pixmap = pixmap.scaled(ui->photoPreview->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            ui->photoPreview->setPixmap(pixmap);
        }
    } else
    {
        ui->photoPreview->clear();
        ui->photoPreview->setText("Нет фотографии");
    }
}

DishInfo DishDialog::getDishInfo() const
{
    DishInfo info;
    info.name = ui->nameEdit->text();
    info.expiryHours = ui->expiryHoursBox->value();
    info.photoPath = currentPhotoPath;
    return info;
}

void DishDialog::setDishInfo(const DishInfo &info)
{
    ui->nameEdit->setText(info.name);
    ui->expiryHoursBox->setValue(info.expiryHours);
    currentPhotoPath = info.photoPath;
    updatePhotoPreview();
    ui->removePhotoButton->setEnabled(!currentPhotoPath.isEmpty());
} 
