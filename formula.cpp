#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression)) {

    }
    Value Evaluate(const SheetInterface& sheet) const override {
        auto func = [&sheet](Position pos) {
            CellInterface::Value value = sheet.GetCell(pos)->GetValue();
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            } else if (std::holds_alternative<std::string>(value)) {
                std::string string_value = sheet.GetCell(pos)->GetText();
                if (string_value[0] == '\'') {
                    throw FormulaError(FormulaError::Category::Value);
                }
                try {
                    return std::stod(string_value);
                }  catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
            } else if (std::holds_alternative<FormulaError>(value)) {
                throw std::get<FormulaError>(value);
            } else {
                throw std::runtime_error("Cell lookup error"s);
            }
        };
        Value tmp;
        try {
            tmp = ast_.Execute(func);
        }  catch (FormulaError& e) {
            tmp = e;
        }
        return tmp;
    }
    std::string GetExpression() const override {
        std::stringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    virtual std::vector<Position> GetReferencedCells() const override {
        return {ast_.GetCells().begin(), ast_.GetCells().end()};
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
