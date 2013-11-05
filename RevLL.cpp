#include <iostream>

using namespace std;

struct node
{
  int data;
  node* left;
  node* right;
};

void insert(int val, node*& head);
void printTree(node* head, int type);

int main(int argc, char* argv[])
{
  node *head = NULL;
  
  srand(time(NULL));

  for (int i = 0; i < 10; i++)
  {
    int x = rand() % 100;
    cout << x << " ";
    insert(x, head);
  }

  cout << endl << "PreOrder" << endl;
  printTree(head, 1);
  cout << endl << "InOrder" << endl;
  printTree(head, 2);
  cout << endl << "PostOrder" << endl;
  printTree(head, 3);
}

void insert(int val, node*& head)
{
  if (head == NULL)
  {
    head = new node;
    head->data = val;
  }
  else
  {
    if (val <= head->data)
    {
      insert(val, head->left);
    }
    else
    {
      insert(val, head->right);
    }
  }
}

void printTree(node* head, int type)
{
  if (head == NULL)
    return;
  
  if (type == 1)
  {
    printf("%d " , head->data);
    printTree(head->left, type);
    printTree(head->right, type);
  }
  else if (type == 2)
  {
    printTree(head->left, type);
    printf("%d " , head->data);
    printTree(head->right, type);
  }
  else
  {
    printTree(head->left, type);
    printTree(head->right, type);  
    printf("%d " , head->data);
  }
}
