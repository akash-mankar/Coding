//#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <stack>


using namespace std;
//
////BST *t = new BST();
////t->insert(10);
////t->insert(20);
////t->insert(5);
////t->insert(30);
////bool b = t->isEmpty();
/////
/////
////10
////  / \
//// 5   20
////      \
////       30
////
////
#define SIZE 5
struct sNode
{
	int Data;
	sNode* left;
	sNode* right;

	//sNode(int value):Data(value),left(0),right(0){}
};

class BST
{
private:
	//sNode* node;
	sNode* root;

	void inorder(sNode* root);
	void preorder(sNode* root);
	void postorder(sNode* root);

	int sum(sNode* root);
	void printlevelbylevel(sNode* root);
	void printzigzag(sNode* root);
	int getHeight(sNode* root);
	void leastCommonAncestor(sNode* root, int first, int second);
	void  printlevelwoqueue(sNode* root);
public:
	BST():root(NULL)
	{
	}

	~BST();
	bool isEmpty()const
	{
		return (root == NULL);
	}
	void insert(int Data);

	void print_inorder();
	void print_preorder();
	void print_postorder();
	void print_sum();
	void search(int num);
	void printlevelbylevel();
	void printzigzag();
	int getHeight();
	int leastCommonAncestor(int first, int second);
	void delet(int);
	void printlevelwoqueue();
};
void BST::printlevelwoqueue(sNode* root)
{
	if (root == NULL)
		return;
	else
	{
		cout << root->Data;
	}
	sNode* curr = root;

	if(curr)
	{
		printlevelwoqueue(curr->left);
		printlevelwoqueue(curr->right);
	}

	getchar();
}

void BST::printlevelwoqueue()
{
	printlevelwoqueue(root);

}

void BST::delet(int Data)
{
	//if(isEmpty())
	//	cout << "node to Delete not found" << endl;

	//sNode* curr;
	//sNode* parent = NULL;
	//
	//curr = root;
	//parent = curr;

	////while(curr)
	//{
	//	//if (curr->Data == Data)

	//}

}

void BST::leastCommonAncestor(sNode* root, int first, int second)
{
	if(root == NULL || root->Data == first || root->Data == second)
	{
		cout <<"no LCA" << endl;  return ;
	}

	sNode* curr;
	curr = root;
	if(curr->right!=NULL 
		&& (curr->right->Data == first || curr->right->Data == second))
	{
		cout << "LCA is  " << curr->Data << endl;
	}
	if(curr->left!=NULL 
		&& (curr->left->Data == first || curr->left->Data == second))
	{
		cout << "LCA is  " << curr->Data << endl;
		return;
	}

	if(curr->Data > first && curr->Data < second )
	{
		cout << "LCA is " << curr->Data << endl;
		return;
		//return curr->Data;
	}
	else if(curr->Data > first && curr->Data > second)
	{
		curr =  curr->left;
		leastCommonAncestor(curr, first,second);
	}
	else if(curr->Data <first && curr->Data < second)
	{
		curr = curr->right;
		leastCommonAncestor(curr,first,second);
	}
}

int BST::leastCommonAncestor(int first,int second)
{
	leastCommonAncestor(root,first,second);
	return 1;
}

int BST::getHeight(sNode* root)
{
	if(root == NULL)
		return -1;
	
	int leftHeight = getHeight(root->left);
	int rightHeight =  getHeight(root->right);

	if(leftHeight >rightHeight)
	   return leftHeight+1;
	else
		return rightHeight+1;

}
int BST::getHeight()
{
	return getHeight(root);
}
void BST::printzigzag(sNode* root)
{
	if(isEmpty())
	return;

	stack<sNode*> currentlevel, nextlevel;
	//bool orderType; // true = left->right; 
	int level =0; 

	currentlevel.push(root);

	while(!currentlevel.empty())
	{
		sNode* temp = currentlevel.top();
		currentlevel.pop();
		if(temp)
		{
			cout << temp->Data << " " ;
			if(level%2)
			{
				nextlevel.push(temp->left);
				nextlevel.push(temp->right);
			}
			else
			{
				nextlevel.push(temp->right);
				nextlevel.push(temp->left);
			}
		}
		if(currentlevel.empty())
		{
			cout << endl;
			swap(currentlevel, nextlevel);
			level++;
		}

	}
}
void BST::printzigzag()
{
	printzigzag(root);
}
void BST::printlevelbylevel(sNode* p)
{
	queue<sNode*> currentlevel, nextlevel;
	//sNode* temp;

	if(isEmpty())
	{
		cout << "tree empty nothing to print " << endl;
		return;
	}

    currentlevel.push(p);
	//nextlevel.push(p->left);
	//nextlevel.push(p->right);

	while(!currentlevel.empty())
	{
		sNode* temp = currentlevel.front();

		if(temp)
		{
			cout << temp->Data << " ";
			nextlevel.push(temp->left);
			nextlevel.push(temp->right);
		}
		currentlevel.pop();
		if(currentlevel.empty())
		{
			cout << endl;
			swap(currentlevel, nextlevel);
		}
	}

}
void BST::printlevelbylevel(){

	printlevelbylevel(root);
}

void BST::search(int num)
{
	if(isEmpty())
	{
		cout<<"Tree is empty number not found" << endl;
		return;
	} 

	sNode* curr;
	curr = root;

	while(curr)
	{
		if(curr->Data == num)
		{
		 cout<< "Number found" << endl;
		return;
		}
		else if (curr->Data > num)
			curr = curr->left;
		else
			curr = curr->right;
	}
	
	cout << "Number not found" << endl;
}
int BST::sum(sNode* p)
{
	if(p==NULL)
		return 0;
	else
	   return ( p->Data + sum(p->left) + sum(p->right));
}

void BST::print_sum()
{
	int sumint;

	sumint = sum(root);

	cout << sumint << endl;
}
void BST::inorder(sNode * p)
{
	if(p!=NULL)
	{
	   inorder(p->left);
	   cout << p->Data << " " ;
	   inorder(p->right);
	}
}
void BST::print_inorder()
{
	inorder(root);
}

void BST::preorder(sNode* p)
{
	if(p!=NULL)
	{
		preorder(p->left);
		preorder(p->right);
		cout<< p->Data << " " ;
	}
}
void BST::print_preorder()
{
	preorder(root);
}

void BST::postorder(sNode* p)
{
	if( p!=NULL)
	{
		postorder(p->right);
		postorder(p->left);
		cout << p->Data << " " ;
	}

}

void BST::print_postorder()
{
	postorder(root);
}

void BST::insert(int Data)
{
	if(root == NULL)
	{
		root = new sNode();
		root->Data = Data;
		root->left = NULL;
		root->right =  NULL;
	}
	else
	{
		sNode* newNode = new sNode();
		newNode->Data = Data;
		newNode->left = NULL;
		newNode->right = NULL;

		sNode* parent = NULL;
		sNode* curr;

		curr = root;
		
		while(curr)
		{
			parent = curr;
			if (curr->Data > newNode->Data)
			 curr = curr->left;
			else if (curr->Data <= newNode->Data)
			 curr = curr->right;
		}

		if(newNode->Data <parent->Data)
		{
			parent->left = newNode;
		}
		else
			parent->right = newNode;
	}

	cout<< "Element " << Data <<"inserted into the tree" << endl;
}


int main()
{
	

	BST * t = new BST();

	if(t->isEmpty())
		cout << "tree is empty" << endl;

	//cout<<"enter the elements to be inserted" << endl;
	srand(time(NULL));
	
	int element;
	for (int i=0; i< SIZE ; i++)
	{
		// cin >> element;
		element = rand()% 100;
		t->insert(element);
	}

	cout<< "in order Traversal" << endl;

	t->print_inorder();
	
	cout << endl << "Preorder Traversal" << endl;
	t->print_preorder();

	cout << "Postorder Traversal " << endl;
	t->print_postorder();

	// print level by level check
	// print zigzag order  chek
	// find least common ancestor check
	// find sum of all nodes check
	// Search for a number  check
	// height of a given tree check
	// delete an element in the tree
	// iterative traversals

	cout << "sum of all nodes in the tree is" ;
	t->print_sum();

	int searchnum = rand()%100;
	cout << "Searching for a number" << searchnum << endl;
	t->search(searchnum);

	cout << "level by level printing of the tree is" << endl;
	t->printlevelbylevel();

	cout << "print the tree in zigzag order" << endl;
	t->printzigzag();

	cout << "height of the tree is" <<t->getHeight()<< endl;
	
	cout << "least common ancestor . Enter two numbers:" << endl;
	int first , second;

	cin >> first >> second;

	t->leastCommonAncestor(first,second);

	t->printlevelwoqueue();
	//t->delet(searchnum);

	getchar();
	getchar();
	return 0;
}