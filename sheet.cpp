#include "sheet.h"

#include "cell.h"
#include "common.h"

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
    cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    cells_[pos.row][pos.col]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (pos.col < printableSize_.cols && pos.row < printableSize_.rows) {
        return cells_[pos.row][pos.col].get();
    } else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (pos.col < printableSize_.cols && pos.row < printableSize_.rows) {
        return cells_[pos.row][pos.col].get();
    } else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position"s);
    }
    if (const auto* cellPointer = GetCell(pos); cellPointer) {
        cells_[pos.row][pos.col]->Clear();
        delete cells_[pos.row][pos.col].release();
        Squeeze();
    }
}

Size Sheet::GetPrintableSize() const {
    return printableSize_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (auto& line: cells_) {
        bool isFirstCell = true;
        for (auto& cell: line) {
            if (!isFirstCell) {
                output << '\t';
            }
            if (cell) {
                PrintCellValue(cell.get(), output);
            }
            isFirstCell = false;
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (auto& line: cells_) {
        bool isFirstCell = true;
        for (auto& cell: line) {
            if (!isFirstCell) {
                output << '\t';
            }
            if (cell) {
                output<<cell->GetText();
            }
            isFirstCell = false;
        }
        output << '\n';
    }
}

void Sheet::Resize(Size newSize) {
    cells_.resize(newSize.rows);
    for (auto& line: cells_) {
        line.resize(newSize.cols);
    }
    printableSize_ = newSize;
}

void Sheet::Squeeze() {
    Size newPrintableSize = {0, 0};
    for (int i = 0; i < printableSize_.rows; ++i) {
        for (int j = 0; j < printableSize_.cols; ++j) {
            if (cells_[i][j]) {
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
