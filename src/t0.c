#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef void (*function_ptr)();

typedef struct tree_node {
    char* key;
    function_ptr value;
    struct tree_node* left;
    struct tree_node* right;
} TreeNode;


TreeNode* create_node(char* key, function_ptr value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->key = (char*)malloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->value = value;
    node->left  = NULL;
    node->right = NULL;
    return node;
}


void insert_node(TreeNode** root, char* key, function_ptr value) {
    if ( *root == NULL)  {
      *root = create_node(key, value);
      return;
    }
    if ( strcmp(key, (*root)->key) < 0 ) {
      insert_node(&(*root)->left, key, value);
    } else {
      insert_node(&(*root)->right, key, value);
    }
}


function_ptr search_node(TreeNode* root, char* key) {
  if ( root == NULL ) return NULL;
  if ( strcmp(key, root->key) == 0 )
    return root->value;
  if ( strcmp(key, root->key) < 0 )
    return search_node(root->left, key);
  return search_node(root->right, key);
}


void free_tree(TreeNode* root) {
  if ( root == NULL ) return;
  free_tree(root->left);
  free_tree(root->right);
  free(root->key);
  free(root);
}
/*
int main() {
  TreeNode* root = NULL;
  insert_node(&root, "hello", &printf);
  insert_node(&root, "world", &puts);
  function_ptr func = search_node(root, "hello");
  func("Hello, world!\n");
  free_tree(root);
  return 0;
}
*/

bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}


bool readSym(char* buffer, int buffer_size) {
  char c;
  int size = 0;

  while ( isSpace(c = getchar()) );
  if ( c == EOF ) return false;
  buffer[size++] = c;

  while ( (c = getchar()) != EOF && ! isSpace(c) && size < buffer_size - 1 ) {
    buffer[size++] = c;
  }
  buffer[size] = '\0';
  return size > 0;
}


void foo() {
  printf("foo\n");
}


void bar() {
  printf("bar\n");
}

TreeNode* scope = NULL;

int main() {
  char c;
  char buf[256];

  insert_node(&scope, "foo", &foo);
  insert_node(&scope, "bar", &bar);

  while ( readSym(buf, sizeof(buf)) ) {
    function_ptr func = search_node(scope, buf);
    if ( func == NULL ) {
      printf("Unknown word: %s\n", buf);
    } else {
      func();
    }
  }

  return 0;
}

/*
char* read_until_space() {
  char c;
  int size = 0;
  char* str = NULL;

  while ((c = getchar()) != ' ' && c != '\n') {
    str = (char*)realloc(str, sizeof(char) * (size + 1));
    str[size++] = c;
  }
  str = (char*)realloc(str, sizeof(char) * (size + 1));
  str[size] = '\0';
  return str;
}
*/
