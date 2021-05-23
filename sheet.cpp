#include "sheet.h"

#include "cell.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (pos.col >= printableSize_.cols || pos.row >= printableSize_.rows) {
        Resize({pos.row >= printableSize_.rows ? pos.row+1 : printableSize_.rows,
                pos.col >= printableSize_.cols ? pos.col+1 : printableSize_.cols});
    }
    cells_[pos] = std::make_unique<Cell>(*this, pos);
    cells_[pos]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (pos.col < printableSize_.cols && pos.row < printableSize_.rows) {
        return cells_.at(pos).get();
    } else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (pos.col < printableSize_.cols && pos.row < printableSize_.rows) {
        return cells_[pos].get();
    } else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (const auto* cellPointer = GetCell(pos); cellPointer) {
        cells_[pos]->Clear();
        delete cells_[pos].release();
        Squeeze();
    }
}

Size Sheet::GetPrintableSize() const {
    return printableSize_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < printableSize_.rows; ++i) {
        bool isFirstCell = true;
        for (int j = 0; j < printableSize_.cols; ++j) {
            if (!isFirstCell) {
                output << '\t';
            }
            if (cells_.count({i,j}) != 0) {
                PrintCellValue(cells_.at({i,j}).get(), output);
            }
            isFirstCell = false;
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
     for (int i = 0; i < printableSize_.rows; ++i) {
        bool isFirstCell = true;
        for (int j = 0; j < printableSize_.cols; ++j) {
            if (!isFirstCell) {
                output << '\t';
            }
            if (cells_.count({i,j}) != 0) {
                output<<cells_.at({i,j})->GetText();
            }
            isFirstCell = false;
        }
        output << '\n';
    }
}

void Sheet::Resize(Size newSize) {
    printableSize_ = newSize;
}

void Sheet::Squeeze() {
    Size newPrintableSize = {0, 0};
    for (int i = 0; i < printableSize_.rows; ++i) {
        for (int j = 0; j < printableSize_.cols; ++j) {
            if (cells_[{i,j}]) {
                if (i >= newPrintableSize.rows) {
                    newPrintableSize.rows = i+1;
                }
                if (j >= newPrintableSize.cols) {
                    newPrintableSize.cols = j+1;
                }
            }
        }
    }
    Resize(newPrintableSize);
}

void Sheet::PrintCellValue(const CellInterface *cell, std::ostream &out) const {
    CellInterface::Value value = cell->GetValue();
    if (std::holds_alternative<double>(value)) {
        out << std::get<double>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        out << std::get<std::string>(value);
    } else if (std::holds_alternative<FormulaError>(value)) {
        out << std::get<FormulaError>(value);
    } else {
        throw std::runtime_error("Print cell value: something wrong"s);
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
