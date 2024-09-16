#include <iostream>
#include <map>
#include <cctype>

std::map<std::string, double> table;

enum class Kind : char
{
    name, number, end,
    plus = '+', minus = '-', mul = '*', div = '/', print = ';', assign = '=', 
    lp = '(', rp =')'
};

struct Token
{
    Kind kind;
    std::string string_value;
    double number_value;
};

class Token_stream
{
    public:
    Token_stream(std::istream& s) : ip(&s), owns{false} {}
    Token_stream(std::istream* p) : ip(p), owns{true} {}

    ~Token_stream() {close();}

    Token get(); // read and return next token
    const Token& current() {return ct;}; // most recently read token

    private:
    void close() {if (owns) delete ip;}

    std::istream* ip; // pointer to an input stream
    bool owns; // does the token stream own the istream?
    Token ct {Kind::end}; // current token
};

int no_of_errors;

double error(const std::string& s)
{
    no_of_errors++;
    std::cerr << "error: " << s << '\n';
    return 1;
}

// INPUT

Token Token_stream::get()
{
    char ch = 0;
    
    do
    { // skip whitespace except '\n'
        if (!ip->get(ch)) return ct = {Kind::end};
    } while (ch != '\n' && isspace(ch));

    switch (ch)
    {
        case ';':
        case '\n':
            return ct = {Kind::print};

        case 0:
            return ct = {Kind::end}; // assign and return

        case '*':
        case '/':
        case '+':
        case '-':
        case '(':
        case ')':
        case '=':
            return ct = {static_cast<Kind>(ch)};

        // NUMBER HANDLING

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            ip->putback(ch); // put the first digit (or .) back into input stream
            *ip >> ct.number_value; // read the number into ct
            ct.kind = Kind::number;
            return ct;

        default:
            if (isalpha(ch))
            {
                ct.string_value = ch;
                
                while (ip->get(ch))
                {
                    if (isalnum(ch)) 
                        ct.string_value += ch; // append ch to end of string_value

                    else
                    {
                        ip->putback(ch);
                        break;
                    }
                    ct.kind = Kind::name;
                    return ct;
                }
            }

            error("bad token");
            return ct = {Kind::print};
    }
}

static Token_stream ts{std::cin};

double expr(bool); 

/*
Breaks up the loop: expr(), which calls term(), which call prim(), which in trun
calls expr().
*/ 

double prim(bool get) // handles primaries
{
    if (get) ts.get(); // read next token

    for(;;) // forever
    {
        switch (ts.current().kind)
        {
            case Kind::number: // floating-point constant

            {
                double v = ts.current().number_value;
                ts.get();
                return v;
            }

            case Kind::name:
            
            {
                double& v = table[ts.current().string_value]; // find the coresponding

                if (ts.get().kind == Kind::assign) v = expr(true); // '=' seen: assignment
                return v;
            }

            case Kind::minus: // unary minus
                return -prim(true);

            case Kind::lp:

            {
                auto e = expr(true);

                if (ts.current().kind != Kind::rp) return error(" ')' expected");
                ts.get();        // eat ')'
                return e;
            }

            default:
                return error("primary expected");
        }
    }
}

double term(bool get) // multiply and divide
{
    double left = prim(get);

    for(;;) // forever
    {
        switch (ts.current().kind)
        {
            case Kind::mul:
                left *= prim(true);
                break;

            case Kind::div:
                if (auto d = prim(true))
                {
                    left /= d;
                    break;
                }
                return error("divide by 0");
            default:
                return left;
        }
    }
}

double expr(bool get) // add and subtract
{
    double left = term(get);

    for(;;) // forever
    {
        switch (ts.current().kind)
        {
            case Kind::plus:
                left += term(true);
                break;

            case Kind::minus:
                left -= term(true);
                break;
            default:
                return left;
        }
    }
}

void calculate()
{
    for(;;)
    {
        ts.get();
        
        if (ts.current().kind == Kind::end) break;

        if (ts.current().kind == Kind::print) continue;
        
        std::cout << expr(false) << '\n';
    }
}

int main()
{
    table["pi"] = 3.14159265359; // insert predefined names
    table["ee"] = 2.71828182846;

    calculate();

    return no_of_errors;
}
