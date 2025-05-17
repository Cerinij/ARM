#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dishdialog.h"
#include "databasedialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QColor>
#include <QRegularExpression>
#include <QCloseEvent>
#include <QStyle>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QGroupBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <algorithm>
#include <QFont>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QSlider>
#include <QPixmap>
#include <QFileInfo>
#include <QSettings>
#include <QRadioButton>
#include <QLabel>
#include <QResizeEvent>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    setupUI();
    setupTrayIcon();
    loadSettings();
    loadDishInfo();
    loadBatchInfo();
    
    // Настраиваем таймер для проверки сроков годности
    expiryTimer = new QTimer(this);
    connect(expiryTimer, &QTimer::timeout, this, &MainWindow::onCheckExpiredDishes);
    expiryTimer->start(60000); // Проверяем каждую минуту
}

MainWindow::~MainWindow()
{
    saveBatchInfo(); // Сохраняем информацию перед закрытием
    delete ui;
}

void MainWindow::setupUI()
{
    // Настройка таблицы
    tableWidget = ui->tableWidget;
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"Название блюда", "Количество", "Срок годности до"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QTableWidget::SelectRows);
    tableWidget->setSelectionMode(QTableWidget::SingleSelection);
    tableWidget->setEditTriggers(QTableWidget::NoEditTriggers); // Запрещаем редактирование
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(tableWidget, &QWidget::customContextMenuRequested, 
            [this](const QPoint &pos) 
            {
                QMenu *menu = new QMenu(this);
                QAction *deleteAction = menu->addAction("Удалить");
                connect(deleteAction, &QAction::triggered, 
                        this, &MainWindow::onDeleteSelectedDishClicked);
                menu->exec(tableWidget->viewport()->mapToGlobal(pos));
            });

    // Подключаем сигналы кнопок
    connect(ui->loadBatchButton, &QPushButton::clicked, 
            this, &MainWindow::onLoadBatchClicked);
    connect(ui->viewDatabaseButton, &QPushButton::clicked, 
            this, &MainWindow::onViewDatabaseClicked);
    connect(ui->settingsButton, &QPushButton::clicked, 
            this, &MainWindow::onSettingsClicked);
    connect(ui->deleteSelectedButton, &QPushButton::clicked, 
            this, &MainWindow::onDeleteSelectedDishClicked);
    connect(ui->loadReportButton, &QPushButton::clicked, 
            this, &MainWindow::onLoadReportClicked);
    connect(tableWidget, &QTableWidget::itemSelectionChanged, 
            [this]() 
            {
                ui->deleteSelectedButton->setEnabled(!tableWidget->selectedItems().isEmpty());
            });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::onLoadBatchClicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        "Выберите файл партии", "", "CSV Files (*.csv)");
    
    if (!filename.isEmpty())
    {
        processCSVFile(filename);
        saveBatchInfo();
    }
}

QString MainWindow::normalizeDishName(const QString &name) const
{

    QString normalized = name.trimmed();
    

    static QRegularExpression weightRegex(R"(\s*\d+\s*(?:г|гр|g|gr|грамм|гр\.).*$)", 
        QRegularExpression::CaseInsensitiveOption);
    normalized = normalized.remove(weightRegex);
    

    normalized = normalized.simplified();
    
    return normalized;
}

QString MainWindow::formatDishName(const QString &name) const
{
    QStringList words = name.simplified().split(' ');
    for (QString &word : words)
    {
        if (!word.isEmpty())
        {
            word[0] = word[0].toUpper();
            if (word.length() > 1)
            {
                word = word[0] + word.mid(1).toLower();
            }
        }
    }
    return words.join(' ');
}

void MainWindow::processCSVFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл CSV.");
        return;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    
    // Пропускаем заголовок
    if (!in.atEnd()) {
        in.readLine();
    }

    bool hasErrors = false;
    QStringList notFoundDishes;
    QList<DishEntry> newDishes; // Временный список для новых блюд

    // Читаем данные
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList fields = line.split(';');
        
        if (fields.size() >= 4) {
            QString originalDishName = fields[1].trimmed();
            if (originalDishName.isEmpty()) continue;
            
            // Нормализуем название блюда
            QString normalizedDishName = normalizeDishName(originalDishName);
            
            int quantity = fields[3].toInt();
            
            // Ищем блюдо в базе данных, игнорируя регистр
            QString matchedDishName;
            for (auto it = dishDatabase.begin(); it != dishDatabase.end(); ++it) {
                if (normalizeDishName(it.key()).compare(normalizedDishName, Qt::CaseInsensitive) == 0) {
                    matchedDishName = it.key();
                    break;
                }
            }
            
            if (!matchedDishName.isEmpty()) {
                DishEntry entry;
                // Форматируем название при добавлении в таблицу
                entry.name = formatDishName(originalDishName);
                entry.quantity = quantity;
                entry.expiryDateTime = QDateTime::currentDateTime()
                    .addSecs(dishDatabase[matchedDishName].expiryHours * 3600);
                
                newDishes.append(entry);
            } else {
                hasErrors = true;
                notFoundDishes.append(originalDishName);
            }
        }
    }
    
    file.close();

    if (hasErrors && !notFoundDishes.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Предупреждение");
        msgBox.setText("Следующие блюда не найдены в базе данных:\n- " + 
            notFoundDishes.join("\n- ") + 
            "\n\nХотите добавить найденные блюда в таблицу?");
        msgBox.setIcon(QMessageBox::Warning);
        
        msgBox.addButton("Да", QMessageBox::YesRole);
        QPushButton *noButton = msgBox.addButton("Нет", QMessageBox::NoRole);

        msgBox.exec();

        if (msgBox.clickedButton() == noButton) {
            return;
        }
    }

    // Добавляем найденные блюда в таблицу
    for (const DishEntry &entry : newDishes) {
        activeDishes.append(entry);
        addDishToTable(entry);
    }

    // Сохраняем изменения
    saveBatchInfo();
}

QColor MainWindow::getExpiryColor(const QDateTime &expiryDateTime) const
{
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 secsToExpiry = currentTime.secsTo(expiryDateTime);
    double hoursToExpiry = secsToExpiry / 3600.0;

    if (hoursToExpiry <= 0) {
        return expiredColor;
    } else if (hoursToExpiry <= criticalHours) {
        return criticalColor;
    } else if (hoursToExpiry <= warningHours) {
        return warningColor;
    }
    return QColor(); // Прозрачный цвет (цвет темы)
}

void MainWindow::updateRowColors()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QDateTime expiryDateTime = QDateTime::fromString(
            tableWidget->item(row, 2)->text(), "dd.MM.yyyy hh:mm");
        
        QColor color = getExpiryColor(expiryDateTime);
        if (!color.isValid()) {
            color = isDarkTheme ? QColor("#2b2b2b") : Qt::white;
        }
        
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            if (QTableWidgetItem *item = tableWidget->item(row, col)) {
                item->setBackground(color);
                item->setForeground(isDarkTheme ? Qt::white : Qt::black);
            }
        }
    }
}

void MainWindow::addDishToTable(const DishEntry &entry)
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);
    
    QTableWidgetItem *nameItem = new QTableWidgetItem(entry.name);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    nameItem->setForeground(isDarkTheme ? Qt::white : Qt::black);
    
    QTableWidgetItem *quantityItem = new QTableWidgetItem(QString::number(entry.quantity));
    quantityItem->setFlags(quantityItem->flags() & ~Qt::ItemIsEditable);
    quantityItem->setForeground(isDarkTheme ? Qt::white : Qt::black);
    
    QTableWidgetItem *dateItem = new QTableWidgetItem(
        entry.expiryDateTime.toString("dd.MM.yyyy hh:mm"));
    dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
    dateItem->setForeground(isDarkTheme ? Qt::white : Qt::black);
    
    // Устанавливаем цвет фона для новой строки
    QColor color = getExpiryColor(entry.expiryDateTime);
    if (!color.isValid()) {
        color = isDarkTheme ? QColor("#2b2b2b") : Qt::white;
    }
    nameItem->setBackground(color);
    quantityItem->setBackground(color);
    dateItem->setBackground(color);
    
    tableWidget->setItem(row, 0, nameItem);
    tableWidget->setItem(row, 1, quantityItem);
    tableWidget->setItem(row, 2, dateItem);
}

void MainWindow::updateDishesTable()
{
    tableWidget->setRowCount(0);
    for (const DishEntry &entry : activeDishes) {
        addDishToTable(entry);
    }
    // Обновляем цвета после обновления всей таблицы
    updateRowColors();
}

void MainWindow::onDeleteSelectedDishClicked()
{
    QList<QTableWidgetItem*> items = tableWidget->selectedItems();
    if (items.isEmpty()) return;
    
    int row = items[0]->row();
    QString dishName = tableWidget->item(row, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение удаления",
        QString("Вы уверены, что хотите удалить \"%1\" из таблицы?").arg(dishName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        removeDish(row);
        saveBatchInfo();
    }
}

void MainWindow::removeDish(int row)
{
    if (row >= 0 && row < activeDishes.size()) {
        activeDishes.removeAt(row);
        tableWidget->removeRow(row);
        // Обновляем цвета после удаления
        updateRowColors();
    }
}

void MainWindow::onCheckExpiredDishes()
{
    updateRowColors();
    
    QStringList expiredDishes;
    QStringList criticalDishes;
    QStringList warningDishes;
    
    // Собираем информацию о всех блюдах
    for (DishEntry &entry : activeDishes) {
        QDateTime currentTime = QDateTime::currentDateTime();
        qint64 secsToExpiry = currentTime.secsTo(entry.expiryDateTime);
        double hoursToExpiry = secsToExpiry / 3600.0;

        // Проверяем просрочку
        if (hoursToExpiry <= 0 && !entry.expiredShown) {
            expiredDishes.append(entry.name);
            entry.expiredShown = true;
        }
        // Проверяем критический срок
        else if (hoursToExpiry <= criticalHours && !entry.criticalShown) {
            criticalDishes.append(entry.name);
            entry.criticalShown = true;
        }
        // Проверяем предупреждение
        else if (hoursToExpiry <= warningHours && !entry.warningShown) {
            warningDishes.append(QString("%1 (%2ч)").arg(entry.name).arg(qRound(hoursToExpiry)));
            entry.warningShown = true;
        }
    }
    
    // Отправляем групповые уведомления
    if (!expiredDishes.isEmpty()) {
        showNotification(
            "Блюда просрочены!",
            QString("Следующие блюда просрочены:\n- %1").arg(expiredDishes.join("\n- ")),
            QSystemTrayIcon::Critical
        );
    }
    
    if (!criticalDishes.isEmpty()) {
        showNotification(
            "Критический срок годности!",
            QString("У следующих блюд осталось меньше часа:\n- %1").arg(criticalDishes.join("\n- ")),
            QSystemTrayIcon::Warning
        );
    }
    
    if (!warningDishes.isEmpty()) {
        showNotification(
            "Внимание!",
            QString("Скоро истекает срок годности:\n- %1").arg(warningDishes.join("\n- ")),
            QSystemTrayIcon::Information
        );
    }
    
    // Обновляем цвета после проверки
    updateRowColors();
}

QString MainWindow::getBatchFilePath() const
{
    return QDir::currentPath() + "/active_dishes.json";
}

void MainWindow::saveBatchInfo()
{
    QJsonObject root;
    QJsonArray dishes;
    
    for (const DishEntry &entry : activeDishes) {
        QJsonObject dish;
        dish["name"] = entry.name;
        dish["quantity"] = entry.quantity;
        dish["expiryDateTime"] = entry.expiryDateTime.toString(Qt::ISODate);
        dishes.append(dish);
    }
    
    root["dishes"] = dishes;
    
    QJsonDocument doc(root);
    QString filePath = getBatchFilePath();
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::loadBatchInfo()
{
    QFile file(getBatchFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        
        activeDishes.clear();
        QJsonArray dishes = root["dishes"].toArray();
        for (const QJsonValue &value : dishes) {
            QJsonObject dish = value.toObject();
            DishEntry entry;
            entry.name = dish["name"].toString();
            entry.quantity = dish["quantity"].toInt();
            entry.expiryDateTime = QDateTime::fromString(
                dish["expiryDateTime"].toString(), Qt::ISODate);
            
            activeDishes.append(entry);
        }
        
        updateDishesTable();
        // Обновляем цвета после загрузки
        updateRowColors();
        file.close();
    }
}

void MainWindow::onViewDatabaseClicked()
{
    DatabaseDialog dialog(dishDatabase, this);
    connect(&dialog, &DatabaseDialog::databaseChanged, this, &MainWindow::onDatabaseChanged);
    dialog.exec();
}

void MainWindow::onDatabaseChanged(const QMap<QString, DishInfo>& database)
{
    dishDatabase = database;
    saveDishInfo();
}

void MainWindow::setupTrayIcon()
{
    // Создаем иконку в трее
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    // Создаем контекстное меню
    trayMenu = new QMenu(this);
    minimizeAction = new QAction("Свернуть", this);
    restoreAction = new QAction("Восстановить", this);
    quitAction = new QAction("Выход", this);
    
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    trayMenu->addAction(minimizeAction);
    trayMenu->addAction(restoreAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::onNotificationClicked);
    
    trayIcon->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (runInBackground && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (!isVisible()) {
            show();
            activateWindow();
        } else {
            hide();
        }
    }
}

void MainWindow::showNotification(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon)
{
    if (trayIcon && trayIcon->isVisible() && notificationsEnabled) {
        // Устанавливаем иконку в зависимости от типа уведомления
        QIcon notificationIcon;
        switch (icon) {
            case QSystemTrayIcon::Critical:
                notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
                break;
            case QSystemTrayIcon::Warning:
                notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
                break;
            default:
                notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
                break;
        }
        
        trayIcon->setIcon(notificationIcon);
        trayIcon->showMessage(title, message, icon, 5000);
    }
}

void MainWindow::saveDishInfo()
{
    QJsonObject root;
    QJsonArray dishes;
    
    for (auto it = dishDatabase.begin(); it != dishDatabase.end(); ++it) {
        QJsonObject dish;
        dish["name"] = it.key();
        dish["expiryHours"] = it.value().expiryHours;
        dish["photoPath"] = it.value().photoPath;  // Сохраняем путь к фотографии
        dishes.append(dish);
    }
    
    root["dishes"] = dishes;
    
    QJsonDocument doc(root);
    QString filePath = QDir::currentPath() + "/dishes.json";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::loadDishInfo()
{
    QFile file("dishes.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        
        QJsonArray dishes = root["dishes"].toArray();
        for (const QJsonValue &value : dishes) {
            QJsonObject dish = value.toObject();
            DishInfo info;
            info.name = dish["name"].toString();
            info.expiryHours = dish["expiryHours"].toInt();
            info.photoPath = dish["photoPath"].toString();  // Загружаем путь к фотографии
            dishDatabase[info.name] = info;
        }
        
        file.close();
    }
}

void MainWindow::onSettingsClicked()
{
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle("Настройки");
    settingsDialog.setMinimumWidth(500);
    
    // Применяем текущую тему к диалогу настроек
    if (isDarkTheme) {
        settingsDialog.setStyleSheet(
            "QDialog { "
            "   background-color: #2b2b2b; "
            "   color: white; "
            "}"
            "QGroupBox { "
            "   color: white; "
            "   font-weight: bold; "
            "   border: 1px solid #3a3a3a; "
            "   border-radius: 8px; "
            "   margin-top: 15px; "
            "   padding: 15px; "
            "}"
            "QGroupBox::title { "
            "   subcontrol-origin: margin; "
            "   left: 15px; "
            "   padding: 0 10px; "
            "   color: white; "
            "}"
            "QLabel { "
            "   color: white; "
            "}"
            "QSpinBox, QCheckBox { "
            "   color: white; "
            "   background-color: #3a3a3a; "
            "   border: 1px solid #4a4a4a; "
            "   border-radius: 4px; "
            "   padding: 5px; "
            "}"
            "QSpinBox:hover, QCheckBox:hover { "
            "   background-color: #454545; "
            "}"
            "QPushButton { "
            "   padding: 8px 15px; "
            "   background-color: #4a4a4a; "
            "   color: white; "
            "   border: none; "
            "   border-radius: 6px; "
            "   font-weight: bold; "
            "}"
            "QPushButton:hover { "
            "   background-color: #5a5a5a; "
            "}"
            "QPushButton:pressed { "
            "   background-color: #3a3a3a; "
            "}"
            "QSlider::groove:horizontal { "
            "   border: 1px solid #4a4a4a; "
            "   height: 8px; "
            "   background: #3a3a3a; "
            "   margin: 2px 0; "
            "   border-radius: 4px; "
            "}"
            "QSlider::handle:horizontal { "
            "   background: #5a5a5a; "
            "   border: 1px solid #4a4a4a; "
            "   width: 18px; "
            "   margin: -2px 0; "
            "   border-radius: 9px; "
            "}"
            "QSlider::handle:horizontal:hover { "
            "   background: #6a6a6a; "
            "}"
            "QDialogButtonBox QPushButton { "
            "   min-width: 80px; "
            "}"
        );
    } else {
        settingsDialog.setStyleSheet(
            "QDialog { "
            "   background-color: #f0f0f0; "
            "   color: black; "
            "}"
            "QGroupBox { "
            "   color: black; "
            "   font-weight: bold; "
            "   border: 1px solid #d0d0d0; "
            "   border-radius: 8px; "
            "   margin-top: 15px; "
            "   padding: 15px; "
            "}"
            "QGroupBox::title { "
            "   subcontrol-origin: margin; "
            "   left: 15px; "
            "   padding: 0 10px; "
            "   color: black; "
            "}"
            "QLabel { "
            "   color: black; "
            "}"
            "QSpinBox, QCheckBox { "
            "   color: black; "
            "   background-color: white; "
            "   border: 1px solid #d0d0d0; "
            "   border-radius: 4px; "
            "   padding: 5px; "
            "}"
            "QSpinBox:hover, QCheckBox:hover { "
            "   background-color: #f5f5f5; "
            "}"
            "QPushButton { "
            "   padding: 8px 15px; "
            "   background-color: #e0e0e0; "
            "   color: black; "
            "   border: none; "
            "   border-radius: 6px; "
            "   font-weight: bold; "
            "}"
            "QPushButton:hover { "
            "   background-color: #d0d0d0; "
            "}"
            "QPushButton:pressed { "
            "   background-color: #c0c0c0; "
            "}"
            "QSlider::groove:horizontal { "
            "   border: 1px solid #d0d0d0; "
            "   height: 8px; "
            "   background: #e0e0e0; "
            "   margin: 2px 0; "
            "   border-radius: 4px; "
            "}"
            "QSlider::handle:horizontal { "
            "   background: #d0d0d0; "
            "   border: 1px solid #c0c0c0; "
            "   width: 18px; "
            "   margin: -2px 0; "
            "   border-radius: 9px; "
            "}"
            "QSlider::handle:horizontal:hover { "
            "   background: #c0c0c0; "
            "}"
            "QDialogButtonBox QPushButton { "
            "   min-width: 80px; "
            "}"
        );
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(&settingsDialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Настройки темы
    QGroupBox *themeGroup = new QGroupBox("Тема оформления", &settingsDialog);
    QHBoxLayout *themeLayout = new QHBoxLayout(themeGroup);
    
    QRadioButton *darkThemeRadio = new QRadioButton("Тёмная тема", &settingsDialog);
    QRadioButton *lightThemeRadio = new QRadioButton("Светлая тема", &settingsDialog);
    
    // Создаем иконки
    QLabel *moonIcon = new QLabel(&settingsDialog);
    QLabel *sunIcon = new QLabel(&settingsDialog);
    
    // Создаем иконки с помощью Unicode символов
    moonIcon->setText("🌙");
    sunIcon->setText("☀️");
    
    // Устанавливаем размер шрифта для иконок
    QFont iconFont = moonIcon->font();
    iconFont.setPointSize(16);
    moonIcon->setFont(iconFont);
    sunIcon->setFont(iconFont);
    
    // Добавляем отступы для иконок
    moonIcon->setContentsMargins(5, 0, 5, 0);
    sunIcon->setContentsMargins(5, 0, 5, 0);
    
    // Создаем горизонтальные компоновщики для каждой темы
    QHBoxLayout *darkLayout = new QHBoxLayout();
    QHBoxLayout *lightLayout = new QHBoxLayout();
    
    darkLayout->addWidget(moonIcon);
    darkLayout->addWidget(darkThemeRadio);
    darkLayout->addStretch();
    
    lightLayout->addWidget(sunIcon);
    lightLayout->addWidget(lightThemeRadio);
    lightLayout->addStretch();
    
    // Добавляем компоновщики в основной layout темы
    themeLayout->addLayout(darkLayout);
    themeLayout->addLayout(lightLayout);
    
    darkThemeRadio->setChecked(isDarkTheme);
    lightThemeRadio->setChecked(!isDarkTheme);
    
    mainLayout->addWidget(themeGroup);

    // Настройки уведомлений
    QGroupBox *notificationGroup = new QGroupBox("Уведомления", &settingsDialog);
    QVBoxLayout *notificationLayout = new QVBoxLayout(notificationGroup);
    
    QSpinBox *warningHoursBox = new QSpinBox(&settingsDialog);
    warningHoursBox->setRange(1, 72);
    warningHoursBox->setValue(warningHours);
    warningHoursBox->setSuffix(" часов");
    warningHoursBox->setToolTip("За сколько часов показывать предупреждение о приближающемся сроке годности");
    
    QSpinBox *criticalHoursBox = new QSpinBox(&settingsDialog);
    criticalHoursBox->setRange(0, 24);
    criticalHoursBox->setValue(criticalHours);
    criticalHoursBox->setSuffix(" часов");
    criticalHoursBox->setToolTip("За сколько часов показывать критическое предупреждение о сроке годности");

    QCheckBox *notificationsCheckBox = new QCheckBox("Включить уведомления", &settingsDialog);
    notificationsCheckBox->setChecked(notificationsEnabled);

    QFormLayout *notificationFormLayout = new QFormLayout;
    notificationFormLayout->addRow("Предупреждение за:", warningHoursBox);
    notificationFormLayout->addRow("Критический срок за:", criticalHoursBox);
    
    notificationLayout->addLayout(notificationFormLayout);
    notificationLayout->addWidget(notificationsCheckBox);
    mainLayout->addWidget(notificationGroup);

    // Настройки цветов
    QGroupBox *colorsGroup = new QGroupBox("Цвета состояний", &settingsDialog);
    QVBoxLayout *colorsLayout = new QVBoxLayout(colorsGroup);

    QPushButton *expiredColorButton = new QPushButton("Цвет просроченных", &settingsDialog);
    expiredColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(expiredColor.name()));
    
    QPushButton *criticalColorButton = new QPushButton("Цвет критического срока", &settingsDialog);
    criticalColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(criticalColor.name()));
    
    QPushButton *warningColorButton = new QPushButton("Цвет предупреждения", &settingsDialog);
    warningColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(warningColor.name()));

    colorsLayout->addWidget(expiredColorButton);
    colorsLayout->addWidget(criticalColorButton);
    colorsLayout->addWidget(warningColorButton);
    mainLayout->addWidget(colorsGroup);

    // Общие настройки
    QGroupBox *generalGroup = new QGroupBox("Общие", &settingsDialog);
    QVBoxLayout *generalLayout = new QVBoxLayout(generalGroup);
    
    QCheckBox *backgroundCheckBox = new QCheckBox("Работать в фоновом режиме при закрытии окна", &settingsDialog);
    backgroundCheckBox->setChecked(runInBackground);
    backgroundCheckBox->setToolTip("Если включено, программа будет сворачиваться в трей при закрытии окна");
    
    generalLayout->addWidget(backgroundCheckBox);
    mainLayout->addWidget(generalGroup);

    // Кнопки OK и Cancel
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        &settingsDialog
    );
    buttonBox->button(QDialogButtonBox::Ok)->setText("ОК");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    // Подключаем обработчики выбора цвета
    connect(expiredColorButton, &QPushButton::clicked, [&]() {
        QColor color = QColorDialog::getColor(expiredColor, &settingsDialog, "Выберите цвет для просроченных блюд");
        if (color.isValid()) {
            expiredColor = color;
            expiredColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(color.name()));
        }
    });

    connect(criticalColorButton, &QPushButton::clicked, [&]() {
        QColor color = QColorDialog::getColor(criticalColor, &settingsDialog, "Выберите цвет для критического срока");
        if (color.isValid()) {
            criticalColor = color;
            criticalColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(color.name()));
        }
    });

    connect(warningColorButton, &QPushButton::clicked, [&]() {
        QColor color = QColorDialog::getColor(warningColor, &settingsDialog, "Выберите цвет для предупреждений");
        if (color.isValid()) {
            warningColor = color;
            warningColorButton->setStyleSheet(QString("background-color: %1; color: white;").arg(color.name()));
        }
    });

    connect(buttonBox, &QDialogButtonBox::accepted, &settingsDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &settingsDialog, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox);

    if (settingsDialog.exec() == QDialog::Accepted) {
        // Сохраняем новые значения
        int newWarningHours = warningHoursBox->value();
        int newCriticalHours = criticalHoursBox->value();
        
        // Проверяем, что критическое время не больше времени предупреждения
        if (newCriticalHours >= newWarningHours) {
            QMessageBox::warning(&settingsDialog, "Ошибка",
                "Критический срок должен быть меньше срока предупреждения.\n"
                "Значения не были сохранены.");
            return;
        }
        
        // Применяем новые значения
        warningHours = newWarningHours;
        criticalHours = newCriticalHours;
        notificationsEnabled = notificationsCheckBox->isChecked();
        runInBackground = backgroundCheckBox->isChecked();
        isDarkTheme = darkThemeRadio->isChecked();
        
        // Сохраняем настройки в файл
        saveSettings();
        
        // Обновляем интерфейс
        updateRowColors();
        
        // Обновляем поведение в трее
        if (!runInBackground) {
            trayIcon->hide();
        } else if (!trayIcon->isVisible()) {
            trayIcon->show();
        }
        
        applyTheme();
    }
}

void MainWindow::applyTheme()
{
    if (isDarkTheme) {
        setStyleSheet(
            "QMainWindow, QDialog, QWidget { "
            "   background-color: #2b2b2b; "
            "   color: white; "
            "}"
            "QTableWidget { "
            "   background-color: #2b2b2b; "
            "   color: white; "
            "   gridline-color: #3a3a3a; "
            "   border: none; "
            "}"
            "QTableWidget::item { "
            "   padding: 5px; "
            "   color: white; "
            "}"
            "QTableWidget::item:selected { "
            "   background-color: #3a3a3a; "
            "}"
            "QHeaderView::section { "
            "   background-color: #3a3a3a; "
            "   color: white; "
            "   padding: 5px; "
            "   border: 1px solid #4a4a4a; "
            "}"
            "QHeaderView::section:first { "
            "   border-top-left-radius: 4px; "
            "}"
            "QHeaderView::section:last { "
            "   border-top-right-radius: 4px; "
            "}"
            "QPushButton { "
            "   background-color: #4a4a4a; "
            "   color: white; "
            "   border: none; "
            "   padding: 8px 15px; "
            "   border-radius: 6px; "
            "}"
            "QPushButton:hover { "
            "   background-color: #5a5a5a; "
            "}"
            "QPushButton:pressed { "
            "   background-color: #3a3a3a; "
            "}"
            "QGroupBox { "
            "   color: white; "
            "   border: 1px solid #3a3a3a; "
            "}"
            "QLabel { "
            "   color: white; "
            "}"
            "QSpinBox, QCheckBox { "
            "   color: white; "
            "   background-color: #3a3a3a; "
            "}"
        );
    } else {
        setStyleSheet(
            "QMainWindow, QDialog, QWidget { "
            "   background-color: #f0f0f0; "
            "   color: black; "
            "}"
            "QTableWidget { "
            "   background-color: white; "
            "   color: black; "
            "   gridline-color: #d0d0d0; "
            "   border: none; "
            "}"
            "QTableWidget::item { "
            "   padding: 5px; "
            "   color: black; "
            "}"
            "QTableWidget::item:selected { "
            "   background-color: #e0e0e0; "
            "}"
            "QHeaderView::section { "
            "   background-color: #e0e0e0; "
            "   color: black; "
            "   padding: 5px; "
            "   border: 1px solid #d0d0d0; "
            "}"
            "QHeaderView::section:first { "
            "   border-top-left-radius: 4px; "
            "}"
            "QHeaderView::section:last { "
            "   border-top-right-radius: 4px; "
            "}"
            "QPushButton { "
            "   background-color: #e0e0e0; "
            "   color: black; "
            "   border: none; "
            "   padding: 8px 15px; "
            "   border-radius: 6px; "
            "   font-weight: bold; "
            "}"
            "QPushButton:hover { "
            "   background-color: #d0d0d0; "
            "}"
            "QPushButton:pressed { "
            "   background-color: #c0c0c0; "
            "}"
            "QGroupBox { "
            "   color: black; "
            "   border: 1px solid #d0d0d0; "
            "}"
            "QLabel { "
            "   color: black; "
            "}"
            "QSpinBox, QCheckBox { "
            "   color: black; "
            "   background-color: white; "
            "}"
        );
    }
    
    // Обновляем цвета в таблице
    updateRowColors();
}

void MainWindow::saveSettings()
{
    QSettings settings("DishExpiryTracker", "Settings");
    settings.setValue("notificationsEnabled", notificationsEnabled);
    settings.setValue("isDarkTheme", isDarkTheme);
    settings.setValue("warningHours", warningHours);
    settings.setValue("criticalHours", criticalHours);
    settings.setValue("runInBackground", runInBackground);
    settings.setValue("expiredColor", expiredColor);
    settings.setValue("criticalColor", criticalColor);
    settings.setValue("warningColor", warningColor);
}

void MainWindow::loadSettings()
{
    QSettings settings("DishExpiryTracker", "Settings");
    notificationsEnabled = settings.value("notificationsEnabled", true).toBool();
    isDarkTheme = settings.value("isDarkTheme", true).toBool();
    warningHours = settings.value("warningHours", 24).toInt();
    criticalHours = settings.value("criticalHours", 1).toInt();
    runInBackground = settings.value("runInBackground", true).toBool();
    
    // Устанавливаем цвета по умолчанию, если они не были сохранены
    expiredColor = settings.value("expiredColor", QColor(255, 102, 102)).value<QColor>();
    criticalColor = settings.value("criticalColor", QColor(255, 153, 153)).value<QColor>();
    warningColor = settings.value("warningColor", QColor(255, 150, 0)).value<QColor>();
    
    // Проверяем, что цвета действительно установлены
    if (!expiredColor.isValid()) expiredColor = QColor(255, 102, 102);
    if (!criticalColor.isValid()) criticalColor = QColor(255, 153, 153);
    if (!warningColor.isValid()) warningColor = QColor(255, 150, 0);
    
    applyTheme();
    
    // Обновляем цвета в таблице после загрузки настроек
    updateRowColors();
}

void MainWindow::onNotificationClicked()
{
    show();
    activateWindow();
    raise();
}

void MainWindow::onLoadReportClicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        "Выберите файл отчёта", "", "CSV Files (*.csv)");
    
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл отчёта.");
        return;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif


    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(';');
    

    QDate today = QDate::currentDate();
    int todayColumn = -1;
    
    for (int i = 1; i < headers.size() - 2; ++i)
    {
        QString dateStr = headers[i].trimmed().remove('"');

        QStringList parts = dateStr.split(',');
        if (parts.size() == 2)
        {
            QString dayMonth = parts[1].trimmed();
            QStringList dayMonthParts = dayMonth.split(' ');
            if (dayMonthParts.size() == 2) {
                int day = dayMonthParts[0].toInt();
                QString monthStr = dayMonthParts[1].toLower();
                int month = 1;
                if (monthStr == "января") month = 1;
                else if (monthStr == "февраля") month = 2;
                else if (monthStr == "марта") month = 3;
                else if (monthStr == "апреля") month = 4;
                else if (monthStr == "мая") month = 5;
                else if (monthStr == "июня") month = 6;
                else if (monthStr == "июля") month = 7;
                else if (monthStr == "августа") month = 8;
                else if (monthStr == "сентября") month = 9;
                else if (monthStr == "октября") month = 10;
                else if (monthStr == "ноября") month = 11;
                else if (monthStr == "декабря") month = 12;

                QDate reportDate(today.year(), month, day);
                if (reportDate == today) {
                    todayColumn = i;
                    break;
                }
            }
        }
    }

    if (todayColumn == -1) {
        QMessageBox::warning(this, "Предупреждение", "Не найдена колонка с сегодняшней датой в отчёте.");
        file.close();
        return;
    }

    // Пропускаем строки 2 и 3 (общая статистика и прогнозы)
    in.readLine();
    in.readLine();

    // Читаем данные, начиная с 4-й строки
    int lineNumber = 4;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        // Проверяем, что это строка с названием (четная)
        if (lineNumber % 2 == 0) {
            QStringList fields = line.split(';');
            if (fields.size() > todayColumn) {
                QString dishName = fields[0].trimmed();
                int quantity = fields[todayColumn].toInt();

                if (quantity > 0) {
                    // Нормализуем название блюда
                    QString normalizedDishName = normalizeDishName(dishName);
                    
                    // Ищем блюдо в базе данных
                    QString matchedDishName;
                    for (auto it = dishDatabase.begin(); it != dishDatabase.end(); ++it) {
                        if (normalizeDishName(it.key()).compare(normalizedDishName, Qt::CaseInsensitive) == 0) {
                            matchedDishName = it.key();
                            break;
                        }
                    }

                    if (!matchedDishName.isEmpty()) {
                        // Сортируем индексы блюд по сроку годности (от раннего к позднему)
                        QList<int> dishIndices;
                        for (int i = 0; i < activeDishes.size(); ++i) {
                            if (normalizeDishName(activeDishes[i].name).compare(normalizedDishName, Qt::CaseInsensitive) == 0) {
                                dishIndices.append(i);
                            }
                        }
                        
                        // Сортируем индексы по сроку годности
                        std::sort(dishIndices.begin(), dishIndices.end(),
                            [this](int a, int b) {
                                return activeDishes[a].expiryDateTime < activeDishes[b].expiryDateTime;
                            });

                        // Списываем блюда, начиная с тех, у которых срок годности раньше
                        int remainingQuantity = quantity;
                        for (int i = 0; i < dishIndices.size(); ++i) {
                            int idx = dishIndices[i];
                            if (remainingQuantity <= 0) break;

                            if (activeDishes[idx].quantity <= remainingQuantity) {
                                remainingQuantity -= activeDishes[idx].quantity;
                                removeDish(idx);
                            } else {
                                activeDishes[idx].quantity -= remainingQuantity;
                                remainingQuantity = 0;
                            }
                        }
                    }
                }
            }
        }
        ++lineNumber;
    }

    file.close();
    updateDishesTable();
    saveBatchInfo();
    QMessageBox::information(this, "Успех", "Отчёт успешно обработан.");
}

void MainWindow::onAddDishClicked()
{
    DishDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DishInfo info = dialog.getDishInfo();
        dishDatabase[info.name] = info;
        saveDishInfo();
    }
}

void MainWindow::onDatabaseClicked()
{
    onViewDatabaseClicked(); // Используем существующую реализацию
}

void MainWindow::checkExpiry()
{
    onCheckExpiredDishes(); // Используем существующую реализацию
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
        if (!selectedItems.isEmpty())
        {

            QSet<int> rowsToDelete;
            for (QTableWidgetItem* item : selectedItems)
            {
                rowsToDelete.insert(item->row());
            }
            

            QList<int> sortedRows = rowsToDelete.values();
            std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
            
            for (int row : sortedRows)
            {
                removeDish(row);
            }
            

            saveBatchInfo();
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
} 
