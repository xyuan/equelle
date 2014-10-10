/*
  Copyright 2013 SINTEF ICT, Applied Mathematics.
*/

#include "PrintCPUBackendASTVisitor.hpp"
#include "ASTNodes.hpp"
#include "SymbolTable.hpp"
#include <iostream>
#include <cctype>
#include <sstream>
#include <stdexcept>


namespace
{
    const char* impl_cppStartString();
    const char* impl_cppEndString();
}

PrintCPUBackendASTVisitor::PrintCPUBackendASTVisitor()
    : suppressed_(false),
      indent_(1),
      sequence_depth_(0)
{
}

PrintCPUBackendASTVisitor::~PrintCPUBackendASTVisitor()
{
}

void PrintCPUBackendASTVisitor::visit(SequenceNode&)
{
    if (sequence_depth_ == 0) {
        // This is the root node of the program.
        // std::cout << cppStartString();
        endl();
    }
    ++sequence_depth_;
}

void PrintCPUBackendASTVisitor::midVisit(SequenceNode&)
{
}

void PrintCPUBackendASTVisitor::postVisit(SequenceNode&)
{
    --sequence_depth_;
    if (sequence_depth_ == 0) {
        // We are back at the root node. Finish main() function.
        std::cout << cppEndString();
        // Emit ensureRequirements() function.
        std::cout <<
            "\n"
            "void ensureRequirements(const equelle::EquelleRuntimeCPU& er)\n"
            "{\n";
        if (requirement_strings_.empty()) {
            std::cout << "    (void)er;\n";
        }
        for (const std::string& req : requirement_strings_) {
            std::cout << "    " << req;
        }
        std::cout <<
            "}\n";
    }
}

void PrintCPUBackendASTVisitor::visit(NumberNode& node)
{
    std::cout.precision(16);
    std::cout << "double(" << node.number() << ")";
}

void PrintCPUBackendASTVisitor::visit(QuantityNode& node)
{
    const double cf = node.conversionFactorSI();
    if (cf != 1.0) {
        std::cout.precision(16);
        std::cout << "(" << cf << "*";
    }
}

void PrintCPUBackendASTVisitor::postVisit(QuantityNode& node)
{
    if (node.conversionFactorSI() != 1.0) {
        std::cout << ")";
    }
}

void PrintCPUBackendASTVisitor::visit(UnitNode& node)
{
}

void PrintCPUBackendASTVisitor::visit(StringNode& node)
{
    std::cout << node.content();
}

void PrintCPUBackendASTVisitor::visit(TypeNode&)
{
    // std::cout << SymbolTable::equelleString(node.type());
}

void PrintCPUBackendASTVisitor::visit(FuncTypeNode&)
{
    // std::cout << node.funcType().equelleString();
}

void PrintCPUBackendASTVisitor::visit(BinaryOpNode&)
{
    std::cout << '(';
}

void PrintCPUBackendASTVisitor::midVisit(BinaryOpNode& node)
{
    char op = ' ';
    switch (node.op()) {
    case Add:
        op = '+';
        break;
    case Subtract:
        op = '-';
        break;
    case Multiply:
        op = '*';
        break;
    case Divide:
        op = '/';
        break;
    default:
        break;
    }
    std::cout << ' ' << op << ' ';
}

void PrintCPUBackendASTVisitor::postVisit(BinaryOpNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(ComparisonOpNode&)
{
    std::cout << '(';
}

void PrintCPUBackendASTVisitor::midVisit(ComparisonOpNode& node)
{
    std::string op(" ");
    switch (node.op()) {
    case Less:
        op = "<";
        break;
    case Greater:
        op = ">";
        break;
    case LessEqual:
        op = "<=";
        break;
    case GreaterEqual:
        op = ">=";
        break;
    case Equal:
        op = "==";
        break;
    case NotEqual:
        op = "!=";
        break;
    default:
        break;
    }
    std::cout << ' ' << op << ' ';
}

void PrintCPUBackendASTVisitor::postVisit(ComparisonOpNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(NormNode&)
{
    std::cout << "er.norm(";
}

void PrintCPUBackendASTVisitor::postVisit(NormNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(UnaryNegationNode&)
{
    std::cout << '-';
}

void PrintCPUBackendASTVisitor::postVisit(UnaryNegationNode&)
{
}

void PrintCPUBackendASTVisitor::visit(OnNode& node)
{
    if (node.isExtend()) {
        std::cout << "er.operatorExtend(";
    } else {
        std::cout << "er.operatorOn(";
    }
}

void PrintCPUBackendASTVisitor::midVisit(OnNode& node)
{
    // Backend's operatorOn/operatorExtend has three arguments when the left argument
    // is a collection, not two. The middle argument (that we will
    // write in this method) should be the set that the first argument
    // is On. Example:
    // a : Collection Of Scalar On InteriorFaces()
    // a On AllFaces() ===> er.operatorOn(a, InteriorFaces(), AllFaces()).
    std::cout << ", ";
    if (node.lefttype().isCollection()) {
        const std::string esname = SymbolTable::entitySetName(node.lefttype().gridMapping());
        // Now esname can be either a user-created named set or an Equelle built-in
        // function call such as AllCells(). If the second, we must transform to
        // proper call syntax for the C++ backend.
        const char first = esname[0];
        const std::string cppterm = std::isupper(first) ?
            std::string("er.") + char(std::tolower(first)) + esname.substr(1)
            : esname;
        std::cout << cppterm;
        std::cout << ", ";
    }
}

void PrintCPUBackendASTVisitor::postVisit(OnNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(TrinaryIfNode&)
{
    std::cout << "er.trinaryIf(";
}

void PrintCPUBackendASTVisitor::questionMarkVisit(TrinaryIfNode&)
{
    std::cout << ", ";
}

void PrintCPUBackendASTVisitor::colonVisit(TrinaryIfNode&)
{
    std::cout << ", ";
}

void PrintCPUBackendASTVisitor::postVisit(TrinaryIfNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(VarDeclNode& node)
{
    suppress();
}

void PrintCPUBackendASTVisitor::postVisit(VarDeclNode&)
{
    unsuppress();
}

void PrintCPUBackendASTVisitor::visit(VarAssignNode& node)
{
    std::cout << indent();
    if (node.type() == StencilI || node.type() == StencilJ || node.type() == StencilK) {
        //This goes into the stencil-lambda definition, and is only used during parsing.
        std::cout << "// Note: ";
    }
    if (!SymbolTable::variableType(node.name()).isMutable()) {
        std::cout << "const " << cppTypeString(node.type()) << " ";
    } else if (defined_mutables_.count(node.name()) == 0) {
        std::cout << cppTypeString(node.type()) << " ";
        defined_mutables_.insert(node.name());
    }
    std::cout << node.name() << " = ";
}

void PrintCPUBackendASTVisitor::postVisit(VarAssignNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(VarNode& node)
{
    if (!suppressed_) {
        std::cout << node.name();
    }
}

void PrintCPUBackendASTVisitor::visit(FuncRefNode& node)
{
    std::cout << node.name();
}

void PrintCPUBackendASTVisitor::visit(JustAnIdentifierNode&)
{
}

void PrintCPUBackendASTVisitor::visit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::visit()}";
}

void PrintCPUBackendASTVisitor::midVisit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::midVisit()}";
}

void PrintCPUBackendASTVisitor::postVisit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::postVisit()}";
}

void PrintCPUBackendASTVisitor::visit(FuncDeclNode&)
{
    // std::cout << node.name() << " : ";
}

void PrintCPUBackendASTVisitor::postVisit(FuncDeclNode&)
{
    // endl();
}

void PrintCPUBackendASTVisitor::visit(FuncStartNode& node)
{
    const FunctionType& ft = SymbolTable::getFunction(node.name()).functionType();
    const size_t n = ft.arguments().size();
    std::cout << indent() << "auto " << node.name() << " = [&](";
    for (int i = 0; i < n; ++i) {
        std::cout << "const auto& " << ft.arguments()[i].name();
        if (i < n - 1) {
            std::cout << ", ";
        }
    }
    suppress();
    ++indent_;
}

void PrintCPUBackendASTVisitor::postVisit(FuncStartNode& node)
{
    unsuppress();
    std::cout << ") {";
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncAssignNode&)
{
}

void PrintCPUBackendASTVisitor::postVisit(FuncAssignNode&)
{
    --indent_;
    std::cout << indent() << "};";
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncArgsNode&)
{
}

void PrintCPUBackendASTVisitor::midVisit(FuncArgsNode&)
{
    if (!suppressed_) {
        std::cout << ", ";
    }
}

void PrintCPUBackendASTVisitor::postVisit(FuncArgsNode&)
{
}

void PrintCPUBackendASTVisitor::visit(ReturnStatementNode&)
{
    std::cout << indent() << "return ";
}

void PrintCPUBackendASTVisitor::postVisit(ReturnStatementNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncCallNode& node)
{
    if (SymbolTable::isFunctionDeclared(node.name())) {
        const std::string fname = node.name();
        const char first = fname[0];
        std::string cppname;
        if (std::isupper(first)) {
            bool is_stencil = false;

            is_stencil = is_stencil | node.type().isStencil();

            const std::vector<EquelleType>& types = node.args()->argumentTypes();

            for (int i=0; i<types.size(); ++i) {
                is_stencil = is_stencil | types[i].isStencil();
            }

            if (is_stencil) {
                cppname += std::string("er_cart.");
            }
            else {
                cppname += std::string("er.");
            }
            cppname += char(std::tolower(first)) + fname.substr(1);
        } else {
            cppname += fname;
        }
        std::cout << cppname << '(';
    }
    else if (SymbolTable::isVariableDeclared(node.name()) && node.type().isStencil()) {
        std::cout << "grid.cellAt( " << node.name() << ", ";
    }
    else {
        //Error here?
    }
}

void PrintCPUBackendASTVisitor::postVisit(FuncCallNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(FuncCallStatementNode&)
{
    std::cout << indent();
}

void PrintCPUBackendASTVisitor::postVisit(FuncCallStatementNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(LoopNode& node)
{
    BasicType loopvartype = SymbolTable::variableType(node.loopSet()).basicType();
    std::cout << indent() << "for (const " << cppTypeString(loopvartype) << "& "
              << node.loopVariable() << " : " << node.loopSet() << ") {";
    ++indent_;
    endl();
}

void PrintCPUBackendASTVisitor::postVisit(LoopNode&)
{
    --indent_;
    std::cout << indent() << "}";
    endl();
}

void PrintCPUBackendASTVisitor::visit(ArrayNode&)
{
    // std::cout << cppTypeString(node.type()) << "({{";
    std::cout << "makeArray(";
}

void PrintCPUBackendASTVisitor::postVisit(ArrayNode&)
{
    // std::cout << "}})";
    std::cout << ")";
}

void PrintCPUBackendASTVisitor::visit(RandomAccessNode& node)
{
    if (node.arrayAccess()) {
        // This is Array access.
        std::cout << "std::get<" << node.index() << ">(";
    } else {
        // This is Vector access.
        std::cout << "CollOfScalar(";
    }
}

void PrintCPUBackendASTVisitor::postVisit(RandomAccessNode& node)
{
    if (node.arrayAccess()) {
        // This is Array access.
        std::cout << ")";
    } else {
        // This is Vector access.
        // Add a grid dimension requirement.
        std::ostringstream os;
        os << "er.ensureGridDimensionMin(" << node.index() + 1 << ");\n";
        addRequirementString(os.str());
        // Random access op is taking the column of the underlying Eigen array.
        std::cout << ".col(" << node.index() << "))";
    }
}

const char *PrintCPUBackendASTVisitor::cppStartString() const
{
    return ::impl_cppStartString();
}

const char *PrintCPUBackendASTVisitor::cppEndString() const
{
    return ::impl_cppEndString();
}


void PrintCPUBackendASTVisitor::endl() const
{
    std::cout << '\n';
}

std::string PrintCPUBackendASTVisitor::indent() const
{
    return std::string(indent_*4, ' ');
}

void PrintCPUBackendASTVisitor::suppress()
{
    suppressed_ = true;
}

void PrintCPUBackendASTVisitor::unsuppress()
{
    suppressed_ = false;
}

std::string PrintCPUBackendASTVisitor::cppTypeString(const EquelleType& et) const
{
    std::string cppstring;
    if (et.isArray()) {
        return "auto";
    }
    if (et.isStencil()) {
        cppstring += "Stencil";
    }
    if (et.isCollection()) {
        cppstring += "CollOf";
    } else if (et.isSequence()) {
        cppstring += "SeqOf";
    }
    cppstring += basicTypeString(et.basicType());
    return cppstring;
}

void PrintCPUBackendASTVisitor::addRequirementString(const std::string& req)
{
    requirement_strings_.insert(req);
}


void PrintCPUBackendASTVisitor::visit(StencilAssignmentNode &node)
{
    std::cout << indent() << "{ //Start of stencil-lambda" << std::endl;
    indent_++;
    //FIXME: Make dimension independent
    std::cout << indent() << "auto cell_stencil = [&]( int i, int j ) {" << std::endl;
    indent_++;
    std::cout << indent();
}

void PrintCPUBackendASTVisitor::midVisit(StencilAssignmentNode &node)
{
    std::cout << " = " << std::endl;
    indent_++;
    std::cout << indent();
    indent_--;
}

void PrintCPUBackendASTVisitor::postVisit(StencilAssignmentNode &node)
{
    std::string gridMapping = SymbolTable::entitySetName(node.type().gridMapping());
    gridMapping[0] = tolower(gridMapping[0]);
    indent_--;
    std::cout << ";" << std::endl;
    std::cout << indent() << "};" << std::endl;
    std::cout << indent() << node.name() << ".grid." << gridMapping << ".execute( cell_stencil );" << std::endl;
    indent_--;
    std::cout << indent() << "} // End of stencil-lambda" << std::endl;
}

void PrintCPUBackendASTVisitor::visit(StencilNode& node)
{
    //FIXME If using half indices, should then use faceAt, not cellAt
    std::cout << node.name() << ".grid.cellAt(" << node.name() << ", ";
}

void PrintCPUBackendASTVisitor::postVisit(StencilNode& node)
{
    std::cout << ")";
}


namespace
{
    const char* impl_cppStartString()
    {
        return
"\n"
"// This program was created by the Equelle compiler from SINTEF.\n"
"\n"
"#include <opm/core/utility/parameters/ParameterGroup.hpp>\n"
"#include <opm/core/linalg/LinearSolverFactory.hpp>\n"
"#include <opm/core/utility/ErrorMacros.hpp>\n"
"#include <opm/autodiff/AutoDiffBlock.hpp>\n"
"#include <opm/autodiff/AutoDiffHelpers.hpp>\n"
"#include <opm/core/grid.h>\n"
"#include <opm/core/grid/GridManager.hpp>\n"
"#include <algorithm>\n"
"#include <iterator>\n"
"#include <iostream>\n"
"#include <cmath>\n"
"#include <array>\n"
"\n"
"#include \"equelle/EquelleRuntimeCPU.hpp\"\n"
"#include \"equelle/CartesianGrid.hpp\"//Should be renamed EquelleCartesianRuntimeCPU\n"
"\n"
"void ensureRequirements(const equelle::EquelleRuntimeCPU& er);\n"
"void equelleGeneratedCode(equelle::EquelleRuntimeCPU& er, equelle::CartesianEquelleRuntime& er_cart);\n"
"\n"
 "#ifndef EQUELLE_NO_MAIN\n"
"int main(int argc, char** argv)\n"
"{\n"
"    // Get user parameters.\n"
"    Opm::parameter::ParameterGroup param(argc, argv, false);\n"
"\n"
"    // Create the Equelle runtime.\n"
"    equelle::CartesianEquelleRuntime er_cart(param);\n"
"    equelle::EquelleRuntimeCPU er(param);\n"
"    equelleGeneratedCode(er, er_cart);\n"
"    return 0;\n"
"}\n"
"#endif // EQUELLE_NO_MAIN\n"
"\n"
"void equelleGeneratedCode(equelle::EquelleRuntimeCPU& er,\n"
"                          equelle::CartesianEquelleRuntime& er_cart) {\n"
"    using namespace equelle;\n"
"    ensureRequirements(er);\n"
"    (void)er_cart; // To suppress compile warnings if not used below.\n"
"\n"
"    // ============= Generated code starts here ================\n";
    }

    const char* impl_cppEndString()
    {
        return "\n"
"    // ============= Generated code ends here ================\n"
"\n"
"}\n";
    }
}
