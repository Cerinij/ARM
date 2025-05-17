#include <QXlsx/xlsxdocument.h>
#include <QXlsx/xlsxformat.h>

int main()
{
    QXlsx::Document xlsx;
    QXlsx::Format headerFormat;
    headerFormat.setFontBold(true);
    headerFormat.setFontSize(12);
    
    // Заголовки
    xlsx.write("A1", "№", headerFormat);
    xlsx.write("B1", "Название блюда", headerFormat);
    xlsx.write("C1", "Категория", headerFormat);
    xlsx.write("D1", "Количество", headerFormat);
    xlsx.write("E1", "Примечание", headerFormat);
    
    // Данные
    xlsx.write("A2", 1);
    xlsx.write("B2", "Борщ");
    xlsx.write("C2", "Первые блюда");
    xlsx.write("D2", 10);
    xlsx.write("E2", "");
    
    xlsx.write("A3", 2);
    xlsx.write("B3", "Котлеты куриные");
    xlsx.write("C3", "Вторые блюда");
    xlsx.write("D3", 25);
    xlsx.write("E3", "");
    
    xlsx.write("A4", 3);
    xlsx.write("B4", "Салат Оливье");
    xlsx.write("C4", "Салаты");
    xlsx.write("D4", 15);
    xlsx.write("E4", "");
    
    xlsx.write("A5", 4);
    xlsx.write("B5", "Компот");
    xlsx.write("C5", "Напитки");
    xlsx.write("D5", 30);
    xlsx.write("E5", "");
    
    // Установка ширины столбцов
    xlsx.setColumnWidth(1, 5);  // A
    xlsx.setColumnWidth(2, 25); // B
    xlsx.setColumnWidth(3, 15); // C
    xlsx.setColumnWidth(4, 12); // D
    xlsx.setColumnWidth(5, 20); // E
    
    xlsx.saveAs("sample_batch.xlsx");
    return 0;
} 