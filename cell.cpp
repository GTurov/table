#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

using namespace std::literals;

Cell::Cell(Sheet& sheet, Position cellPosition)
    : sheet_(sheet), ownPosition_(cellPosition), impl_(std::make_unique<EmptyImpl>()){
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text != impl_->GetText()) {
        if (text.size() > 1 && text[0] == '=') {

            // Проверка на корректность синтаксиса формулы
            std::unique_ptr<FormulaImpl> newFormula;
            try {
                newFormula = std::make_unique<FormulaImpl>(sheet_, text);
            }  catch (...) {
                throw FormulaException("Formula syntax error"s);
            }

            // Проверка на циклические зависимости
            auto newDependencesDown_ = newFormula->GetReferencedCells();
            if (!IsIndependent(newDependencesDown_)) {
                throw CircularDependencyException("Circular dependency found"s);
            }

            // Отмена регистрации в зависимостях вниз
            Unregister();

            // Очистка кэша в ячейке и в зависимостях вверх
            ResetCache();

            // Обновление содержимого ячейки
            delete impl_.release();
            impl_ = std::move(newFormula);

            // Формирование списка зависимостей вниз
            dependencesDown_ = newDependencesDown_;

            // Регистрация в зависимостях вниз
            Register();


        } else {
            // Очистка кэша в ячейке и в зависимостях вверх
            ResetCache();

            // Отмена регистрации в зависимостях вниз
            Unregister();

            // Очистка списка зависимосей вниз
            dependencesDown_.clear();

            // Обновление содержимого ячейки
            delete impl_.release();
            impl_ = std::make_unique<TextImpl>(text);
        }
    }
}

void Cell::Clear() {
    delete impl_.release();
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (!cachedValue_) {
        cachedValue_ = impl_->GetValue();
    }
    return *cachedValue_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return dependencesDown_;
}

bool Cell::IsReferenced() const {
    return dependencesUp_.size() != 0;
}

void Cell::RegisterAsDependent(const Position &pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    dependencesUp_.insert(pos);
}

void Cell::UnregisterAsDependent(const Position &pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (auto it = dependencesUp_.find(pos); it != dependencesUp_.end()) {
        dependencesUp_.erase(it);
    }
}

void Cell::ResetCache() const {
    cachedValue_.reset();
    for (const Position pos: dependencesUp_) {
        if (const Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos)); cell) {
            cell->ResetCache();
        }
    }
}

void Cell::Unregister() const {
    for (const Position pos: dependencesDown_) {
        if (const Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos)); cell) {
            cell->UnregisterAsDependent(ownPosition_);
        }
    }
}

void Cell::Register() const {
    for (const Position pos: dependencesDown_) {
        if (const Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos)); cell) {
            cell->RegisterAsDependent(ownPosition_);
        } else {
            sheet_.SetCell(pos, "");
            dynamic_cast<Cell*>(sheet_.GetCell(pos))->RegisterAsDependent(pos);
        }
    }
}

bool Cell::IsIndependent(const std::vector<Position>& cellPositions) {
    std::stack<Position> toVisit;
    std::unordered_set<Position, PositionHasher> visited;
    for (auto pos: cellPositions) {
        toVisit.push(pos);
    }

    while (!toVisit.empty()) {
        Position currentPos = toVisit.top();
        if (currentPos == ownPosition_) {
            return false;
        }
        toVisit.pop();
        visited.insert(currentPos);
        if (auto* cellPtr = sheet_.GetCell(currentPos); cellPtr) {
            for (auto& pos: cellPtr->GetReferencedCells()) {
                if (visited.count(pos)==0) {
                    toVisit.push(pos);
                }
            }
        }
    }
    return true;
}
