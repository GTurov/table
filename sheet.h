#pragma once

#include "errors.h"
#include "position.h"

#include <functional>
#include <memory>

struct Size {
    int rows = 0;
    int cols = 0;

    bool operator==(Size rhs) const {
        return rows == rhs.rows && cols == rhs.cols;
    }
};

class CellInterface;
class Cell;

// Интерфейс таблицы
class SheetInterface {
public:
    virtual ~SheetInterface() = default;

    // Задаёт содержимое ячейки. Если текст начинается со знака "=", то он
    // интерпретируется как формула. Если задаётся синтаксически некорректная
    // формула, то бросается исключение FormulaException и значение ячейки не
    // изменяется. Если задаётся формула, которая приводит к циклической
    // зависимости (в частности, если формула использует текущую ячейку), то
    // бросается исключение CircularDependencyException и значение ячейки не
    // изменяется.
    // Уточнения по записи формулы:
    // * Если текст содержит только символ "=" и больше ничего, то он не считается
    // формулой
    // * Если текст начинается с символа "'" (апостроф), то при выводе значения
    // ячейки методом GetValue() он опускается. Можно использовать, если нужно
    // начать текст со знака "=", но чтобы он не интерпретировался как формула.
    virtual void SetCell(Position pos, std::string text) = 0;

    // Возвращает значение ячейки.
    // Если ячейка пуста, может вернуть nullptr.
    virtual const CellInterface* GetCell(Position pos) const = 0;
    virtual CellInterface* GetCell(Position pos) = 0;

    // Очищает ячейку.
    // Последующий вызов GetCell() для этой ячейки вернёт либо nullptr, либо
    // объект с пустым текстом.
    virtual void ClearCell(Position pos) = 0;

    // Вычисляет размер области, которая участвует в печати.
    // Определяется как ограничивающий прямоугольник всех ячеек с непустым
    // текстом.
    virtual Size GetPrintableSize() const = 0;

    // Выводит всю таблицу в переданный поток. Столбцы разделяются знаком
    // табуляции. После каждой строки выводится символ перевода строки. Для
    // преобразования ячеек в строку используются методы GetValue() или GetText()
    // соответственно. Пустая ячейка представляется пустой строкой в любом случае.
    virtual void PrintValues(std::ostream& output) const = 0;
    virtual void PrintTexts(std::ostream& output) const = 0;
};

// Создаёт готовую к работе пустую таблицу.
std::unique_ptr<SheetInterface> CreateSheet();

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void Resize(Size newSize);

private:
    void Squeeze();
    void PrintCellValue(const CellInterface* cell, std::ostream& out) const;
    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> cells_;
    Size printableSize_;
};
