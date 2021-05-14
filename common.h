#pragma once

#include <stdexcept>
#include <ostream>

// Описывает ошибки, которые могут возникнуть при вычислении формулы.
class FormulaError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

// Исключение, выбрасываемое при попытке задать синтаксически некорректную
// формулу
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
