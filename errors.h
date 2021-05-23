#pragma once

#include <stdexcept>
#include <string>

using namespace std::literals;

// Описывает ошибки, которые могут возникнуть при вычислении формулы.
class FormulaError {
  public:
      enum class Category {
          Ref,    // ссылка на ячейку с некорректной позицией
          Value,  // ячейка не может быть трактована как число
          Div0,  // в результате вычисления возникло деление на ноль
      };

      FormulaError(Category category)
          : category_(category) {
      }

      Category GetCategory() const {
          return category_;
      }

      bool operator==(FormulaError rhs) const {
          return category_ == rhs.category_;
      }

      std::string_view ToString() const {
          switch (category_) {
          case Category::Ref: return "#REF!"sv; break;
          case Category::Value: return "#VALUE!"sv; break;
          case Category::Div0: return "#DIV0!"sv; break;
          default: throw std::runtime_error("Something wrong");
          }
      }

  private:
      Category category_;
};

inline std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

// Исключение, выбрасываемое при попытке передать в метод некорректную позицию
class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

// Исключение, выбрасываемое при попытке задать синтаксически некорректную
// формулу
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое при попытке задать формулу, которая приводит к
// циклической зависимости между ячейками
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
