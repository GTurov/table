#pragma once

#include "formula.h"
#include "position.h"

#include <optional>
#include <unordered_set>

class Sheet;

class CellInterface {
public:
    // Либо текст ячейки, либо значение формулы, либо сообщение об ошибке из
    // формулы
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() = default;

    // Возвращает видимое значение ячейки.
    // В случае текстовой ячейки это её текст (без экранирующих символов). В
    // случае формулы - числовое значение формулы или сообщение об ошибке.
    virtual Value GetValue() const = 0;
    // Возвращает внутренний текст ячейки, как если бы мы начали её
    // редактирование. В случае текстовой ячейки это её текст (возможно,
    // содержащий экранирующие символы). В случае формулы - её выражение.
    virtual std::string GetText() const = 0;

    // Возвращает список ячеек, которые непосредственно задействованы в данной
    // формуле. Список отсортирован по возрастанию и не содержит повторяющихся
    // ячеек. В случае текстовой ячейки список пуст.
    virtual std::vector<Position> GetReferencedCells() const = 0;
};



class Cell : public CellInterface {
public:
    Cell(Sheet& sheet, Position cellPosition);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    // можно передавать аргументом указатель на ячейку, и это будет работать быстрее,
    // но для масштабируемости лучше передавать позицию
    void RegisterAsDependent(const Position& pos) const;

    void UnregisterAsDependent(const Position& pos) const;

    void ResetCache() const;

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
        FormulaImpl(SheetInterface& sheet, const std::string& text)
            : sheet_(dynamic_cast<SheetInterface&>(sheet)), formula_(ParseFormula(text.substr(1))) {
        }
        virtual void Set(std::string text) override {
            formula_ = ParseFormula(text);
        }
        virtual void Clear() override {
            delete formula_.release();
        }

        virtual Value GetValue() const override {
            auto result = formula_->Evaluate(sheet_);
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
            } else {
                return std::get<FormulaError>(result);
            }
        }

        virtual std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }

        std::vector<Position> GetReferencedCells() const {
            return formula_->GetReferencedCells();
        }
    private:
        SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
    };

    void Unregister() const;
    void Register() const;
    bool IsIndependent(const std::vector<Position>& cellPositions);

    Sheet& sheet_;
    Position ownPosition_;
    std::unique_ptr<Impl> impl_;
    mutable std::optional<Value> cachedValue_;
    mutable std::unordered_set<Position, PositionHasher> dependencesUp_;
    std::vector<Position> dependencesDown_;

};
