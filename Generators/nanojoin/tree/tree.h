#ifndef TREE_H
#define TREE_H

struct stopnode {
	int minpent;
	int minhex;
	int minhept;
	int mininternal;
};

struct node {
	struct node* children[2];
	struct stopnode *stop;
};

struct node *treeroot;

void initialize_tree();

#endif /* TREE_H */