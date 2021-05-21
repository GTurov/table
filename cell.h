#pragma once

#include "common.h"
#include "formula.h"

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:

    class Impl {
    public:
        virtual ~Impl() {}

        virtual void Set(std::string text) = 0;
        virtual void Clear() = 0;

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        virtual void Set(std::string text) override {
        }
        virtual void Clear() override {
        }

        virtual Value GetValue() const override {
            return "";
        }

        virtual std::string GetText() const override {
            return "";
        }
    };

    class TextImpl : public Impl {
    public:
        TextImpl(const std::string& text)
            : text_(text) {
        }
        virtual void Set(std::string text) override {
            text_ = text;
        }
        virtual void Clear() override {
            text_.clear();
        }

        virtual Value GetValue() const override {
            if (text_[0] == '\'') {
                return text_.substr(1);
            } else {
                return text_;
            }
        }

        virtual std::string GetText() const override {
            return text_;
        }

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(const std::string& text)
            : formula_(ParseFormula(text.substr(1))) {
        }
        virtual void Set(std::string text) override {
            formula_ = ParseFormula(text);
        }
        virtual void Clear() override {
            delete formula_.release();
        }

        virtual Value GetValue() const override {
            auto result = formula_->Evaluate();
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
            } else {
                return FormulaError("");
            }
        }

        virtual std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }
    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

    std::unique_ptr<Impl> impl_;

};
