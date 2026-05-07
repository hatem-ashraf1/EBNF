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
    Node* children[2];
    int num_children;

    Node* first()
    {
        if (num_children > 0) return children[0];
        return 0;
    }

    Node* second()
    {
        if (num_children > 1) return children[1];
        return 0;
    }

    bool isID()
    {
        return label[0] != 'e' && label[0] >= 'a' && label[0] <= 'z';
    }

    bool isInv()
    {
        return strcmp(label, "inverse") == 0;
    }

    bool isProd()
    {
        return strcmp(label, "product") == 0;
    }

    bool isE()
    {
        return label[0] == 'e';
    }

    Node* copy()
    {
        Node* n = (Node*)malloc(sizeof(Node));
        strcpy(n->label, label);
        n->num_children = num_children;
        for (int i = 0; i < num_children; ++i)
        {
            n->children[i] = children[i]->copy();
        }
        return n;
    }

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

    // Must be a var or e

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

bool AreEqualTrees(Node* t1, Node* t2)
{
    if (strcmp(t1->label, t2->label) != 0) return false;
    if (t1->num_children != t2->num_children) return false;
    for (int i = 0; i < t1->num_children; ++i)
        if (!AreEqualTrees(t1->children[i], t2->children[i])) 
            return false;
    return true;
}

Node* Reduce(Node* t)
{
    if (t->isProd())
    {
        // e . x -> x
        if (t->first()->isE())
        {
            Node *newT = t->second()->copy();
            FreeTree(t);
            return newT;
        }
        // x . e -> x
        if (t->second()->isE())
        {
            Node *newT = t->first()->copy();
            FreeTree(t);
            return newT;
        }
        
        // x^-1 . x -> e
        if (t->first()->isInv() && AreEqualTrees(t->first()->first(), t->second()))
        {
            FreeTree(t);
            return NewNode("e");
        }
        
        // x . x^-1 -> e
        if (t->second()->isInv() && AreEqualTrees(t->second()->first(), t->first()))
        {
            FreeTree(t);
            return NewNode("e");
        }

        // y^-1 . (y . z) -> z
        // y . (y^-1 . z) -> z
        if (t->second()->isProd())
        {
            if (t->first()->isInv() && AreEqualTrees(t->first()->first(), t->second()->first()) ||
                t->second()->first()->isInv() && AreEqualTrees(t->first(), t->second()->first()->first()))
            {
                Node *newT = t->second()->second()->copy();
                FreeTree(t);
                return newT;
            }
        }

        // (x . y) . z -> x . (y . z)
        if (t->first()->isProd())
        {
            Node *n = NewNode("product");
            AddChild(n, t->first()->first()->copy());
            Node *n2 = NewNode("product");
            AddChild(n2, t->first()->second()->copy());
            AddChild(n2, t->second()->copy());
            AddChild(n, n2);
            FreeTree(t);
            return n;
        }
    }

    if (t->isInv())
    {
        // e^-1 -> e
        if (t->first()->isE())
        {
            FreeTree(t);
            return NewNode("e");
        }

        // (x^-1)^-1 -> x
        if (t->first()->isInv())
        {
            Node *n = t->first()->first()->copy();
            FreeTree(t);
            return n;
        }
        
        // (x . y)^-1 -> y^-1 . x^-1
        if (t->first()->isProd())
        {
            Node *n = NewNode("product");
            Node *n1 = NewNode("inverse");
            AddChild(n1, t->first()->second()->copy());
            Node *n2 = NewNode("inverse");
            AddChild(n2, t->first()->first()->copy());
            AddChild(n, n1);
            AddChild(n, n2);
            FreeTree(t);
            return n;
        }
    }

    Node *reducedChild;
    for (int i = 0; i < t->num_children; ++i)
    {
        reducedChild = Reduce(t->children[i]);
        if (reducedChild)
        {
            t->children[i] = reducedChild;
            return t;
        }
    }
    return 0;
}

void PrintTreeRecursively(Node* n, int depth, bool last_child[])
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
        PrintTreeRecursively(n->children[i], depth + 1, last_child);
    }
}

void PrintTree(Node* n)
{
    bool last_child[128];
    int i;
    for(i = 0; i < 128; i++) last_child[i] = false;
    PrintTreeRecursively(n, 0, last_child);
}

void PrintExp(Node* n, bool needParens)
{
    if (needParens) printf("(");
    
    if (n->isProd())
    {
        PrintExp(n->first(), n->first()->isProd());
        printf(".");
        PrintExp(n->second(), n->second()->isProd());
    } 
    else if (n->isInv())
    {
        if (n->first()->isProd() || n->first()->isInv())
        {
            printf("(");
            PrintExp(n->first(), false);
            printf(")^-1");
        } 
        else
        {
            PrintExp(n->first(), false);
            printf("^-1");
        }
    }
    else
    {
        printf("%s", n->label);
    }
    
    if (needParens) printf(")");
}


//////////////////////////////////////////////////////////////////////////////////////////

// Test cases
void RunTest(const char* expr)
{
    Node* tree;
    printf("Input: %s\n", expr);
    input = expr;
    pos = 0;
    tree = ParseExpr();
    if(tree == 0) {printf("Parse error.\n\n"); return;}

    while (true)
    {
        printf("Parse tree:\n");
        PrintTree(tree);
        printf("\n");

        printf("Expression: ");
        PrintExp(tree, false);
        printf("\n\n");

        Node* reducedTree = Reduce(tree);
        if (!reducedTree) break;
        else tree = reducedTree;
    }
    printf("--------------------------------\n\n");
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
    RunTest("y^-1.(y.z)");
    RunTest("y.(y^-1.z)");

    return 0;
}