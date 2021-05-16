#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
    using Line = std::vector<std::unique_ptr<Cell>>;
    using Table = std::vector<Line>;
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void Resize(Size newSize);

private:
    void Squeeze();
    void PrintCellValue(const CellInterface* cell, std::ostream& out) const;
    Table cells_;
    Size printableSize_;
};
