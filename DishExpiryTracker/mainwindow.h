#pragma once

#include "dishinfo.h"
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QColor>
#include <QSettings>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui 
{ 
    class MainWindow; 
}
QT_END_NAMESPACE

struct DishEntry 
{
    QString name;
    int quantity;
    QDateTime expiryDateTime;
    bool warningShown = false;
    bool criticalShown = false;
    bool expiredShown = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onLoadBatchClicked();
    void onViewDatabaseClicked();
    void onSettingsClicked();
    void onDatabaseChanged(const QMap<QString, DishInfo>& database);
    void onDeleteSelectedDishClicked();
    void onCheckExpiredDishes();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onNotificationClicked();
    void onLoadReportClicked();
    void onAddDishClicked();
    void onDatabaseClicked();
    void checkExpiry();

private:
    // Настройки уведомлений
    int warningHours = 24;    // Раннее предупреждение
    int criticalHours = 1;    // Критическое предупреждение
    bool notificationsEnabled = true;  // Включены ли уведомления
    int notificationDuration = 5000;   // Длительность показа уведомления (мс)
    bool soundEnabled = true;          // Включен ли звук уведомлений

    // Общие настройки
    int updateInterval = 60000;        // Интервал обновления таблицы (мс)
    QString dataPath;                  // Путь для сохранения файлов
    bool autoBackup = true;            // Автоматическое резервное копирование
    int fontSize = 10;                 // Размер шрифта
    bool runInBackground = true;       // Работа в фоновом режиме

    // Цвета для разных состояний
    QColor expiredColor = QColor(255, 102, 102);    // Цвет для просроченных
    QColor criticalColor = QColor(255, 153, 153);   // Цвет для критических
    QColor warningColor = QColor(255, 150, 0);      // Цвет для предупреждений

    // Основные виджеты
    Ui::MainWindow *ui;
    QTableWidget *tableWidget;
    
    // Данные
    QMap<QString, DishInfo> dishDatabase;
    QList<DishEntry> activeDishes;

    // Таймеры
    QTimer *expiryTimer;
    QTimer *checkTimer;
    
    // Системный трей
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    // Тема
    bool isDarkTheme = true;

    // Методы инициализации
    void setupUI();
    void setupTrayIcon();
    void applyTheme();

    // Методы работы с данными
    void processCSVFile(const QString &filename);
    void addDishToTable(const DishEntry &entry);
    void updateDishesTable();
    void removeDish(int row);
    QString getBatchFilePath() const;
    QColor getExpiryColor(const QDateTime &expiryDateTime) const;
    QString normalizeDishName(const QString &name) const;
    QString formatDishName(const QString &name) const;
    void showNotification(const QString &title, const QString &message, 
                         QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void updateRowColors();

    // Методы сохранения/загрузки
    void saveBatchInfo();
    void loadBatchInfo();
    void saveDishInfo();
    void loadDishInfo();
    void saveSettings();
    void loadSettings();
};
