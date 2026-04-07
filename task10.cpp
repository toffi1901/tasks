#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cctype>
#include <cmath>
#include <stdexcept>

enum class TokenType {
    VAR,              
    LPAREN,   
    RPAREN,         
    OR,         
    AND, 
    NOT,   
    END
};

struct Token{
    TokenType type;
    std::string val;
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

std::vector<Token> tokenize(const std::string &input){
    std::vector<Token> tokens;
    for (size_t i = 0; i < input.size(); ++i){
        char c = input[i];
        if (std::isspace(c)){
            continue;
        }
        if (c == '('){
            tokens.push_back({TokenType::LPAREN, "("});
            continue;
        }
        if (c == ')'){
            tokens.push_back({TokenType::RPAREN, ")"});
            continue;
        }
        if (std::isalpha(c)){
            std::string str;
            while (i < input.size() && std::isalpha(input[i])){
                str += std::toupper(input[i]);
                i++;
            }
            i--;

            if (str == "AND"){
                tokens.push_back({TokenType::AND, str});
            }
            else if (str == "OR"){
                tokens.push_back({TokenType::OR, str});
            }
            else if (str == "NOT"){
                tokens.push_back({TokenType::NOT, str});
            }
            else if (str.size() == 1){
                tokens.push_back({TokenType::VAR, str});
            }
            else{
                throw ParseError("Error: ");
            }
            continue;
        }
        else{
            throw ParseError("Error: ");
        }
    }
    tokens.push_back({TokenType::END, ""});
    return tokens;
};


struct AstNode{
    virtual ~AstNode() = default;
    virtual bool create_table(const std::map<char, bool>& vars) const = 0;
    virtual void all_operands(std::set<char>& vars) const = 0;
};

struct VarNode : AstNode {
    char name;
    VarNode(char new_var) : name(new_var) {}
    bool create_table(const std::map<char, bool>& vars) const override{
        return vars.at(name);
    }
    void all_operands(std::set<char>& vars) const override{
        vars.insert(name);
    }
};

struct NotNode : AstNode {
    AstNode* child;
    NotNode(AstNode *new_child) : child(new_child) {}
    ~NotNode() override { delete child; }
    bool create_table(const std::map<char, bool>& vars) const override{
        return !child->create_table(vars);
    }
    void all_operands(std::set<char>& vars) const override{
        child->all_operands(vars);
    }
};

struct BinaryNode : AstNode {
    enum Op { AND, OR } op;
    AstNode* left;
    AstNode* right;
    BinaryNode(Op op, AstNode* lleft, AstNode* rright) : op(op), left(lleft), right(rright) {}
    ~BinaryNode() override { delete left; delete right; }
    bool create_table(const std::map<char, bool>& vars) const override{
        if (op == AND){
            return left->create_table(vars) && right->create_table(vars);
        }
        else{
            return left->create_table(vars) || right->create_table(vars);
        }
    }
    void all_operands(std::set<char>& vars) const override{
        left->all_operands(vars);
        right->all_operands(vars);
    }

};

class Parser{
    private:
        std::vector<Token> tokens;
        size_t pos;
    public:
        Parser(const std::vector<Token> &new_tokens) : tokens(new_tokens), pos(0) {}

        Token get_argument(){
            if (pos >= tokens.size()){
                return {TokenType::END, ""};
            }
            return tokens[pos];
        }

        Token inc_pos(){
            if (pos >= tokens.size()){
                return {TokenType::END, ""};
            }
            return tokens[pos++];
        }

        AstNode *parse_arg(){
            Token tok = get_argument();
            if (tok.type == TokenType::VAR){
                inc_pos();
                return new VarNode(tok.val[0]);
            }
            else if (tok.type == TokenType::NOT){
                inc_pos();
                AstNode *right = parse_arg();
                return new NotNode(right);
            }
            else if (tok.type == TokenType::LPAREN){
                inc_pos();
                AstNode *node = parse_expr();
                if (get_argument().type != TokenType::RPAREN){
                    delete node;
                    throw ParseError("Error: ");
                }
                inc_pos();
                return node;
            }
            throw ParseError("Unexpected token");
        }

        AstNode *parse_term(){
            AstNode *left = parse_arg();
            while (get_argument().type == TokenType::AND){
                inc_pos();
                AstNode *right = parse_arg();
                left = new BinaryNode(BinaryNode::AND, left, right);
            }
            return left;
        }


        AstNode *parse_expr(){
            AstNode *left = parse_term();
            while (get_argument().type == TokenType::OR){
                inc_pos();
                AstNode *right = parse_term();
                left = new BinaryNode(BinaryNode::OR, left, right);
            }
            return left;
        };

        AstNode *parser(){
            if (tokens.empty()){
                return nullptr;
            }
            AstNode *node = parse_expr();
            if (get_argument().type != TokenType::END){
                delete node;
                throw ParseError("Error: ");
            }
            return node;
        }
};


int main() {
    std::string line;
    if (!std::getline(std::cin, line)){
        return 1;
    }
    try {
        std::vector<Token> tokens = tokenize(line);
        Parser parser(tokens);
        AstNode* ast = parser.parser();
        if (!ast){
            return 1;
        }
        std::set<char> vars;
        ast->all_operands(vars);

        for (char v : vars) std::cout << v << " ";
        std::cout << "Result" << std::endl;

        std::vector<char> varList(vars.begin(), vars.end());
        int n = varList.size();
        for (int mask = 0; mask < (1 << n); ++mask) {
            std::map<char, bool> values;
            for (int i = 0; i < n; ++i) {
                bool val = (mask >> i) & 1;
                values[varList[i]] = val;
                std::cout << val << " ";
            }
            bool res = ast->create_table(values);
            std::cout << res << std::endl;
        }

        delete ast;
    } catch (const ParseError& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}