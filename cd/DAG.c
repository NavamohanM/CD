#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NODES 100
#define MAX_EXPR 100

typedef enum { NODE_VAR, NODE_OP } NodeType;
typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } OpType;

struct DAGNode;

typedef union {
    char var_name[2];
    struct {
        OpType op_type;
        struct DAGNode *left;
        struct DAGNode *right;
    } op;
} NodeData;

struct DAGNode {
    int id;
    NodeType type;
    NodeData data;
};

struct DAGNode *dag_nodes[MAX_NODES];
int next_node_id = 0;

struct DAGNode* create_node(NodeType type) {
    if (next_node_id >= MAX_NODES) {
        fprintf(stderr, "Too many nodes\n");
        exit(1);
    }
    struct DAGNode *node = (struct DAGNode*)malloc(sizeof(struct DAGNode));
    node->id = next_node_id;
    node->type = type;
    dag_nodes[next_node_id++] = node;
    return node;
}

struct DAGNode* get_var_node(char var) {
    for (int i = 0; i < next_node_id; i++) {
        if (dag_nodes[i]->type == NODE_VAR && dag_nodes[i]->data.var_name[0] == var) {
            return dag_nodes[i];
        }
    }
    struct DAGNode *node = create_node(NODE_VAR);
    node->data.var_name[0] = var;
    node->data.var_name[1] = '\0';
    return node;
}

struct DAGNode* get_op_node(OpType op_type, struct DAGNode *left, struct DAGNode *right) {
    for (int i = 0; i < next_node_id; i++) {
        if (dag_nodes[i]->type == NODE_OP &&
            dag_nodes[i]->data.op.op_type == op_type &&
            dag_nodes[i]->data.op.left == left &&
            dag_nodes[i]->data.op.right == right) {
            return dag_nodes[i];
        }
    }
    struct DAGNode *node = create_node(NODE_OP);
    node->data.op.op_type = op_type;
    node->data.op.left = left;
    node->data.op.right = right;
    return node;
}

void print_dag_node(struct DAGNode *node) {
    if (!node) return;
    printf("Node %d: ", node->id);
    if (node->type == NODE_VAR) {
        printf("VAR('%s')\n", node->data.var_name);
    } else {
        char op_char = '?';
        switch (node->data.op.op_type) {
            case OP_ADD: op_char = '+'; break;
            case OP_SUB: op_char = '-'; break;
            case OP_MUL: op_char = '*'; break;
            case OP_DIV: op_char = '/'; break;
        }
        printf("OP('%c', Left: Node %d, Right: Node %d)\n", op_char, node->data.op.left->id, node->data.op.right->id);
    }
}

void print_dag_summary() {
    printf("\n--- DAG Nodes Created ---\n");
    for (int i = 0; i < next_node_id; i++) {
        print_dag_node(dag_nodes[i]);
    }
    printf("---------------------------\n");
}

int precedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        default: return 0;
    }
}

int is_operator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

void to_postfix(char* infix, char postfix[][10], int* postfix_len) {
    char stack[MAX_EXPR];
    int top = -1, k = 0;
    for (int i = 0; infix[i]; i++) {
        if (isspace(infix[i])) continue;
        if (isalnum(infix[i])) {
            sprintf(postfix[k++], "%c", infix[i]);
        } else if (infix[i] == '(') {
            stack[++top] = infix[i];
        } else if (infix[i] == ')') {
            while (top >= 0 && stack[top] != '(')
                sprintf(postfix[k++], "%c", stack[top--]);
            top--; // Remove '('
        } else if (is_operator(infix[i])) {
            while (top >= 0 && precedence(stack[top]) >= precedence(infix[i]))
                sprintf(postfix[k++], "%c", stack[top--]);
            stack[++top] = infix[i];
        }
    }
    while (top >= 0)
        sprintf(postfix[k++], "%c", stack[top--]);
    *postfix_len = k;
}

struct DAGNode* build_dag_from_postfix(char postfix[][10], int len) {
    struct DAGNode* stack[MAX_EXPR];
    int top = -1;
    for (int i = 0; i < len; i++) {
        if (isalpha(postfix[i][0])) {
            stack[++top] = get_var_node(postfix[i][0]);
        } else {
            struct DAGNode *right = stack[top--];
            struct DAGNode *left = stack[top--];
            OpType op_type;
            switch (postfix[i][0]) {
                case '+': op_type = OP_ADD; break;
                case '-': op_type = OP_SUB; break;
                case '*': op_type = OP_MUL; break;
                case '/': op_type = OP_DIV; break;
                default: op_type = OP_ADD;
            }
            stack[++top] = get_op_node(op_type, left, right);
        }
    }
    return stack[top];
}

int main() {
    char expr[MAX_EXPR];
    char postfix[MAX_EXPR][10];
    int postfix_len;

    printf("Enter an arithmetic expression (e.g., a+a*(b-c)):\n");
    fgets(expr, sizeof(expr), stdin);
    expr[strcspn(expr, "\n")] = 0;

    to_postfix(expr, postfix, &postfix_len);
    struct DAGNode* root = build_dag_from_postfix(postfix, postfix_len);

    print_dag_summary();
    return 0;
}

