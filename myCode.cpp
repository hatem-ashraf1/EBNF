// IDs: 20220425, 20220426, 20220419
// Names: Hatem Ashraf Elsayed, Ziad Mohamed Ali, Mohamed Mahmoud Farag

// Extended BNF Grammar (EBNF):
// <expr>   ::= <factor> { '.' <factor> }
// <factor> ::= <base> ['^' '-' '1']
// <base>   ::= '(' <expr> ')' | <var>
// <var>    ::= any lowercase letter or 'e'

#include <cstdio>
#include <cstdlib>
#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////

// Parse tree node
struct Node
{
    char label[64];
    Node* children[64];
    int num_children;
};

//////////////////////////////////////////////////////////////////////////////////////////

Node* NewNode(const char* label)
{
    Node* n = (Node*)malloc(sizeof(Node));
    strcpy(n->label, label);
    n->num_children = 0;
    return n;
}

void AddChild(Node* parent, Node* child)
{
    parent->children[parent->num_children] = child;
    parent->num_children++;
}

void FreeTree(Node* n)
{
    if(n == 0) return;
    int i;
    for(i = 0; i < n->num_children; i++) FreeTree(n->children[i]);
    free(n);
}

//////////////////////////////////////////////////////////////////////////////////////////

const char* input;
int pos;

Node* ParseExpr();
Node* ParseFactor();
Node* ParseBase();

//////////////////////////////////////////////////////////////////////////////////////////

Node* ParseBase()
{
    if(input[pos] == '(')
    {
        pos++; // consume '('
        Node* inner = ParseExpr();
        if(input[pos] == ')') pos++; // consume ')'
        return inner;
    }

    // Must be a variable (single letter)
    if((input[pos] >= 'a' && input[pos] <= 'z'))
    {
        char label[2]; label[0] = input[pos]; label[1] = 0;
        pos++;
        return NewNode(label);
    }

    return 0; // error
}

Node* ParseFactor()
{
    Node* base = ParseBase();
    if(base == 0) return 0;

    // Check for '^-1' (inverse)
    if(input[pos] == '^' && input[pos+1] == '-' && input[pos+2] == '1')
    {
        pos += 3;
        Node* inv = NewNode("inverse");
        AddChild(inv, base);
        return inv;
    }

    return base;
}

Node* ParseExpr()
{
    Node* left = ParseFactor();
    if(left == 0) return 0;

    while(input[pos] == '.')
    {
        pos++; // consume '.'
        Node* right = ParseFactor();
        if(right == 0) break;

        Node* prod = NewNode("product");
        AddChild(prod, left);
        AddChild(prod, right);
        left = prod;
    }

    return left;
}

//////////////////////////////////////////////////////////////////////////////////////////

void PrintTree(Node* n, int depth, bool last_child[])
{
    int i;
    // Print prefix
    for(i = 0; i < depth; i++)
    {
        if(i == depth - 1)
        {
            printf("|--");
        }
        else
        {
            if(!last_child[i]) printf("|  ");
            else printf("   ");
        }
    }
    printf("%s\n", n->label);

    for(i = 0; i < n->num_children; i++)
    {
        last_child[depth] = (i == n->num_children - 1);
        PrintTree(n->children[i], depth + 1, last_child);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

// Test cases
void RunTest(const char* expr)
{
    printf("Input: %s\n", expr);
    input = expr;
    pos = 0;

    Node* tree = ParseExpr();
    if(tree == 0) {printf("Parse error.\n\n"); return;}

    bool last_child[128];
    PrintTree(tree, 0, last_child, 0);
    printf("\n");

    FreeTree(tree);
}

//////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // Test cases
    RunTest("((x.y^-1).z)^-1");
    RunTest("x.y");
    RunTest("x^-1");
    RunTest("e.x");
    RunTest("x.e");
    RunTest("(x.y).z");
    RunTest("x.(y.z)");
    RunTest("(x.y^-1)^-1");
    RunTest("x^-1.x");
    RunTest("x.x^-1");
    RunTest("e^-1");
    RunTest("(x.y)^-1");
    RunTest("x^-1.y^-1");
    RunTest("(x^-1)^-1");
    RunTest("x.y.z");
    RunTest("(x.y).z^-1");
    RunTest("x^-1.(y.z)");
    RunTest("(e.x)^-1");
    RunTest("x.(y^-1.z)");
    RunTest("((x.y)^-1.z)^-1");
    RunTest("(x^-1.y^-1)^-1");
    RunTest("e^-1.x");

    return 0;
}