#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : sheet_(sheet), impl_(std::make_unique<EmptyImpl>()){
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text != impl_->GetText()) {
        delete impl_.release();
        if (text.size() > 1 && text[0] == '=') {
            impl_ = std::make_unique<FormulaImpl>(sheet_, text);
        } else {
            impl_ = std::make_unique<TextImpl>(text);
        }
    }
}

void Cell::Clear() {
    delete impl_.release();
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return {};
}
