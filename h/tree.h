/* tree.h - declare structures used by tree.c
 * vix 27jun86 [broken out of tree.c]
 */


#ifndef	_TREE_FLAG
#define	_TREE_FLAG


typedef	struct	tree_s
	{
		struct	tree_s	*tree_l, *tree_r;
		short		tree_b;
		char		*tree_p;
	}
	tree;


void tree_init(tree **ppr_tree);

char *tree_srch(tree **ppr_tree, int (*pfi_compare)(),char * pc_user);

void tree_add(tree **ppr_tree, int (*pfi_compare)(), char *pc_user, int (*pfi_delete)());

int tree_delete(tree **ppr_p, int (*pfi_compare)(), char *pc_user, int (*pfi_uar)());

int tree_trav(tree **ppr_tree, int (*pfi_uar)());

void tree_mung(tree **ppr_tree, int (*pfi_uar)());

unsigned long tree_count(tree **ppr_tree);

#endif _TREE_FLAG