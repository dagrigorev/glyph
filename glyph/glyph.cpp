#include "glyph.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <stdexcept>
#include <cctype>

// ============================================================================
// AST Node Types
// ============================================================================

enum class NodeType {
    VALUE,      // The underscore literal _
    BINARY_OP,  // +, -, *, ^, %
    LET,        // : binding
    COND,       // % conditional
    VAR         // Variable reference
};

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
    virtual int evaluate(std::map<int, int>& env) const = 0;
};

// Value node - represents underscore (_) which equals 1
class ValueNode : public ASTNode {
public:
    ValueNode() { type = NodeType::VALUE; }
    int evaluate(std::map<int, int>& env) const override {
        return 1;
    }
};

// Variable node - represents a bound name (by index)
class VarNode : public ASTNode {
public:
    int varIndex;
    VarNode(int idx) : varIndex(idx) { type = NodeType::VAR; }
    int evaluate(std::map<int, int>& env) const override {
        if (env.find(varIndex) == env.end()) {
            throw std::runtime_error("Unbound variable: " + std::to_string(varIndex));
        }
        return env[varIndex];
    }
};

// Binary operation node
class BinaryOpNode : public ASTNode {
public:
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(char operation, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(operation), left(std::move(l)), right(std::move(r)) {
        type = NodeType::BINARY_OP;
    }

    int evaluate(std::map<int, int>& env) const override {
        int leftVal = left->evaluate(env);
        int rightVal = right->evaluate(env);

        switch (op) {
        case '+': return leftVal + rightVal;
        case '-': return leftVal - rightVal;
        case '*': return leftVal * rightVal;
        case '^': {
            int result = 1;
            for (int i = 0; i < rightVal; i++) {
                result *= leftVal;
            }
            return result;
        }
        case '%': return leftVal % rightVal;
        default:
            throw std::runtime_error("Unknown operator");
        }
    }
};

// Let binding node: :(name)(value)(body)
class LetNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> name;
    std::unique_ptr<ASTNode> value;
    std::unique_ptr<ASTNode> body;

    LetNode(std::unique_ptr<ASTNode> n, std::unique_ptr<ASTNode> v, std::unique_ptr<ASTNode> b)
        : name(std::move(n)), value(std::move(v)), body(std::move(b)) {
        type = NodeType::LET;
    }

    int evaluate(std::map<int, int>& env) const override {
        // Evaluate the bound value
        int val = value->evaluate(env);

        // Create new environment with binding
        std::map<int, int> newEnv = env;
        int varIndex = name->evaluate(env); // Use evaluated name as index
        newEnv[varIndex] = val;

        // Evaluate body in new environment
        return body->evaluate(newEnv);
    }
};

// Conditional node: %(condition)(then)(else)
class CondNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;

    CondNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> t, std::unique_ptr<ASTNode> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {
        type = NodeType::COND;
    }

    int evaluate(std::map<int, int>& env) const override {
        int condVal = condition->evaluate(env);

        // If condition % 1 == 0 (always true for integers), it's false -> else
        // Otherwise -> then
        // Better interpretation: if condVal == 0 -> else, otherwise -> then
        if (condVal == 0) {
            return elseBranch->evaluate(env);
        }
        else {
            return thenBranch->evaluate(env);
        }
    }
};

// ============================================================================
// Lexer / Tokenizer
// ============================================================================

class Lexer {
private:
    std::string input;
    size_t pos;

public:
    Lexer(const std::string& src) : input(src), pos(0) {}

    char peek() const {
        if (pos >= input.length()) return '\0';
        return input[pos];
    }

    char consume() {
        if (pos >= input.length()) return '\0';
        return input[pos++];
    }

    void expect(char c) {
        char ch = consume();
        if (ch != c) {
            throw std::runtime_error(std::string("Expected '") + c + "' but got '" + ch + "'");
        }
    }

    bool isOperator(char c) const {
        return c == '+' || c == '-' || c == '*' || c == '^' || c == '%';
    }

    bool isValidChar(char c) const {
        return c == '*' || c == '(' || c == ')' || c == '+' ||
            c == '-' || c == '^' || c == '%' || c == '_' || c == ':';
    }
};

// ============================================================================
// Parser
// ============================================================================

class Parser {
private:
    Lexer& lexer;

public:
    Parser(Lexer& lex) : lexer(lex) {}

    std::unique_ptr<ASTNode> parseExpression() {
        char ch = lexer.peek();

        if (ch == '\0') {
            throw std::runtime_error("Unexpected end of input");
        }

        if (ch == '_') {
            lexer.consume();
            return std::make_unique<ValueNode>();
        }

        if (ch == '(') {
            lexer.consume(); // eat '('

            char next = lexer.peek();

            // Check for operators
            if (lexer.isOperator(next)) {
                char op = lexer.consume();

                // Special case: % can be conditional
                if (op == '%' && lexer.peek() == '(') {
                    // It's a conditional: %(cond)(then)(else)
                    auto cond = parseExpression();
                    auto thenBranch = parseExpression();
                    auto elseBranch = parseExpression();
                    lexer.expect(')');
                    return std::make_unique<CondNode>(
                        std::move(cond),
                        std::move(thenBranch),
                        std::move(elseBranch)
                    );
                }
                else {
                    // Regular binary operation
                    auto left = parseExpression();
                    auto right = parseExpression();
                    lexer.expect(')');
                    return std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
                }
            }

            // Check for let binding
            if (next == ':') {
                lexer.consume(); // eat ':'
                auto name = parseExpression();
                auto value = parseExpression();
                auto body = parseExpression();
                lexer.expect(')');
                return std::make_unique<LetNode>(
                    std::move(name),
                    std::move(value),
                    std::move(body)
                );
            }

            throw std::runtime_error("Invalid expression starting with '('");
        }

        throw std::runtime_error(std::string("Unexpected character: ") + ch);
    }
};

// ============================================================================
// Interpreter
// ============================================================================

class GlyphInterpreter {
public:
    int run(const std::string& source) {
        // Validate input - only allowed characters
        for (char c : source) {
            if (c != '*' && c != '(' && c != ')' && c != '+' &&
                c != '-' && c != '^' && c != '%' && c != '_' && c != ':') {
                throw std::runtime_error(std::string("Invalid character: ") + c);
            }
        }

        Lexer lexer(source);
        Parser parser(lexer);

        auto ast = parser.parseExpression();

        std::map<int, int> env;
        return ast->evaluate(env);
    }
};

// ============================================================================
// Main Program
// ============================================================================

int main() {
    GlyphInterpreter interpreter;

    std::cout << "=== Glyph Programming Language Interpreter ===" << std::endl;
    std::cout << "Valid characters: * ( ) + - ^ % _ :" << std::endl << std::endl;

    // Test programs from the specification
    std::vector<std::pair<std::string, std::string>> testPrograms = {
        {"_", "Unit value (1)"},
        {"(+__)", "1 + 1 = 2"},
        {"(+(+__)_)", "2 + 1 = 3"},
        {"(^(+__)_)", "2 ^ 1 = 2"},
        {"(^(+__)(+__))", "2 ^ 2 = 4"},
        {"(*(+__)(+(+__)_))", "2 * 3 = 6"},
        {"(-(+(+(+__)_)_)_)", "4 - 1 = 3"},
        {"(%(+(+__)_)__)", "3 % 1 = 0"},
        {"(:___)", "Let binding: let x=1 in 1"},
        {"(%(+__)__)", "Conditional: if 2 then 1 else 1"}
    };

    for (const auto& [program, description] : testPrograms) {
        try {
            int result = interpreter.run(program);
            std::cout << "Program: " << program << std::endl;
            std::cout << "  Desc: " << description << std::endl;
            std::cout << "  Result: " << result << std::endl;
            std::cout << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Program: " << program << std::endl;
            std::cout << "  Desc: " << description << std::endl;
            std::cout << "  ERROR: " << e.what() << std::endl;
            std::cout << std::endl;
        }
    }

    // Interactive mode
    std::cout << "=== Interactive Mode ===" << std::endl;
    std::cout << "Enter Glyph expressions (or 'quit' to exit):" << std::endl;

    std::string line;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "quit" || line == "exit" || line == "q") {
            break;
        }

        if (line.empty()) {
            continue;
        }

        try {
            int result = interpreter.run(line);
            std::cout << "=> " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}

