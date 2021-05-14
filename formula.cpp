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
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression)) {

    }
    Value Evaluate() const override {
        Value tmp;
        try {
            tmp = ast_.Execute();
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

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
