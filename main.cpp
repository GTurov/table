#include "common.h"
#include "test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

namespace {

void TestEmpty() {
    auto sheet = CreateSheet();
    ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{0, 0}));
}

void TestInvalidPosition() {
    auto sheet = CreateSheet();
    try {
        sheet->SetCell(Position{-1, 0}, "");
    } catch (const InvalidPositionException&) {
    }
    try {
        sheet->GetCell(Position{0, -2});
    } catch (const InvalidPositionException&) {
    }
    try {
        sheet->ClearCell(Position{Position::MAX_ROWS, 0});
    } catch (const InvalidPositionException&) {
    }
}

void TestSetCellPlainText() {
    auto sheet = CreateSheet();

    auto checkCell = [&](Position pos, std::string text) {
        sheet->SetCell(pos, text);
        CellInterface* cell = sheet->GetCell(pos);
        ASSERT(cell != nullptr);
        ASSERT_EQUAL(cell->GetText(), text);
        ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
    };

    checkCell("A1"_pos, "Hello");
    checkCell("A1"_pos, "World");
    checkCell("B2"_pos, "Purr");
    checkCell("A3"_pos, "Meow");

    const SheetInterface& constSheet = *sheet;
    ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

    sheet->SetCell("A3"_pos, "'=escaped");
    CellInterface* cell = sheet->GetCell("A3"_pos);
    ASSERT_EQUAL(cell->GetText(), "'=escaped");
    ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
}

void TestClearCell() {

    auto sheet = CreateSheet();

    sheet->SetCell("C2"_pos, "Me gusta");
    sheet->ClearCell("C2"_pos);
    ASSERT(sheet->GetCell("C2"_pos) == nullptr);

    sheet->ClearCell("A1"_pos);
    sheet->ClearCell("J10"_pos);
}

void TestPrint() {
    auto sheet = CreateSheet();
    sheet->SetCell("A2"_pos, "meow");
    sheet->SetCell("B2"_pos, "=1+2");
    sheet->SetCell("A1"_pos, "=1/0");

    ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 2}));

    std::ostringstream texts;
    sheet->PrintTexts(texts);
    ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

    std::ostringstream values;
    sheet->PrintValues(values);
    ASSERT_EQUAL(values.str(), "#DIV0!\t\nmeow\t3\n");

    sheet->ClearCell("B2"_pos);
    ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 1}));
}

void TestExpressions() {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("B1"_pos, "2");
    sheet->SetCell("C1"_pos, "=A1+B1");
    sheet->SetCell("D1"_pos, "=C1+1");
    sheet->SetCell("A2"_pos, "5");
    sheet->SetCell("B2"_pos, "=B1/0");
    sheet->SetCell("C2"_pos, "meow");
    sheet->SetCell("D2"_pos, "=A2+1");
    sheet->SetCell("A3"_pos, "'5");
    sheet->SetCell("B3"_pos, "=B1+B2");
    sheet->SetCell("C3"_pos, "=C1+C2");
    sheet->SetCell("D3"_pos, "=A3+1");
    sheet->SetCell("A4"_pos, "=C1-A2");
    sheet->SetCell("B4"_pos, "=B3+1");
    sheet->SetCell("C4"_pos, "=C3+1");
    sheet->SetCell("D4"_pos, "=A2*B2");

    std::ostringstream values;
    sheet->PrintValues(values);

    ASSERT_EQUAL(values.str(),
                 "1\t2\t3\t4\n"
                 "5\t#DIV0!\tmeow\t6\n"
                 "5\t#DIV0!\t#VALUE!\t#VALUE!\n"
                 "-2\t#DIV0!\t#VALUE!\t#DIV0!\n"
                 );
}

void TestErrors() {
    {
        auto sheet = CreateSheet();
        sheet->SetCell("A1"_pos, "1");
        try {
            sheet->SetCell("A2"_pos, "=A1+*");
            ASSERT(false);
        } catch (FormulaException& e) {

        }
    }

    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=ZZZ1+*");
            ASSERT(false);
        } catch (FormulaException& e) {

        }
    }

    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=A1");
            ASSERT(false);
        } catch (CircularDependencyException& e) {

        }
    }

    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=A1+100500");
            ASSERT(false);
        } catch (CircularDependencyException& e) {

        }
    }

    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=A2+1");
            sheet->SetCell("A2"_pos, "=A1+1");
            ASSERT(false);
        } catch (CircularDependencyException& e) {

        }
    }

    {
        auto sheet = CreateSheet();
        try {
            sheet->SetCell("A1"_pos, "=B1+B2");
            sheet->SetCell("B1"_pos, "=C1+C2+C3");
            sheet->SetCell("B2"_pos, "=C3+C4+C5");
            sheet->SetCell("C3"_pos, "=A1");
            ASSERT(false);
        } catch (CircularDependencyException& e) {

        }
    }
}



}  // namespace

int main() {
    auto sheet = CreateSheet();
//    sheet->SetCell("A1"_pos, "1");
//    sheet->SetCell("B1"_pos, "2");
//    sheet->SetCell("C1"_pos, "=A1+*");
//    sheet->SetCell("A1"_pos, "=ZZ1+1");
//    sheet->SetCell("A1"_pos, "=B1+B2");
//    sheet->SetCell("B1"_pos, "=C1+C2+C3");
//    sheet->SetCell("B2"_pos, "=C3+C4+C5");
//    sheet->SetCell("C3"_pos, "=A1");

    TestRunner tr;
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestInvalidPosition);
    RUN_TEST(tr, TestSetCellPlainText);
    RUN_TEST(tr, TestClearCell);
    RUN_TEST(tr, TestPrint);
    RUN_TEST(tr, TestExpressions);
    RUN_TEST(tr, TestErrors);
    return 0;
}
