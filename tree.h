#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."

FileTree *createFileTree(char* rootFolderName) {
	FileTree *file_tree =  malloc(sizeof(*file_tree));

	if (!file_tree) {
		fprintf(stderr, "Failed tree alloc\n");
		return NULL;
	}

	file_tree->root = malloc(sizeof(TreeNode));
	if (!file_tree->root) {
		fprintf(stderr, "Failed root alloc\n");
		free(file_tree);
		return NULL;
	}
	file_tree->root->name = strdup(rootFolderName);
	file_tree->root->parent = NULL;
	file_tree->root->type = FOLDER_NODE;
	file_tree->root->content = malloc(sizeof(FolderContent));
	((FolderContent *)(file_tree->root->content))->children = ll_create();
	
	return file_tree;
}

void freeTree(FileTree fileTree) {
	TreeNode *current_node = fileTree.root;
	TreeNode *next_node;
	ListNode *current_list_node;
	ListNode *next_list_node;
	while (current_node) {
		next_node = current_node->parent;
		current_list_node = ((FolderContent *)(current_node->content))->children->head;
		while (current_list_node) {
			next_list_node = current_list_node->next;
			free(current_list_node->info);
			free(current_list_node);
			current_list_node = next_list_node;
		}
		ll_free(((FolderContent *)(current_node->content))->children);
		free(current_node->content);
		free(current_node->name);
		free(current_node);
		current_node = next_node;
	}
	free(fileTree.root);
}


void ls(TreeNode* currentNode, char* arg) {

    if (!currentNode) {
		fprintf(stderr, "No current node :(");
		return;
	}

	List *curr_list = ((FolderContent *)(currentNode->content))->children;
	if (!strcmp(arg, NO_ARG)) {
		ll_print(curr_list);
		return;
	}

	ListNode *curr = ll_search(curr_list, arg);

	if (curr->info->type == FILE_NODE) {
		printf("%s: %s\n",curr->info->name , ((FileContent *) curr->info->content)->text);
		return;
	}

	if (curr->info->type == FOLDER_NODE) {
		curr_list = ((FolderContent *)curr->info->content)->children;
		ll_print(curr_list);
		return;
	}

}


void pwd(TreeNode* treeNode) {
	char path[LINE_MAX_LEN] = "";
    TreeNode *prev_dir = treeNode;

	while (prev_dir) {
		char *aux = strdup(path);
		sprintf(path, "%s/%s", prev_dir->name, aux);
		prev_dir = prev_dir->parent;
		free(aux);
	}

	path[strlen(path) - 1] = '\0';
	printf("%s", path);
}


TreeNode* cd(TreeNode* currentNode, char* path) {

	TreeNode *next_dir = currentNode;
	List *curr_list = ((FolderContent *)(currentNode->content))->children;

	// Because the string pointer is modified by strtok
	char *aux_path = strdup(path);
	char *next_directory = strtok(aux_path, "/");

	while (next_directory) {
		if (!strcmp(next_directory, PARENT_DIR)) {
			next_dir = next_dir->parent;
			if (!next_dir) {
				free(aux_path);
				return currentNode;
			}
		} else {
			ListNode *child = ll_search(curr_list, next_directory);
			if (child) {
				next_dir = child->info;
			} else {
				printf("cd: no such file or directory: '%s'", path);
				free(aux_path);
				return currentNode;
			}
		}
		curr_list = ((FolderContent *)next_dir->content)->children;
		next_directory = strtok(NULL, "/");
	}

	free(aux_path);
	return next_dir;
}

//duplicat
TreeNode* move_to(TreeNode* currentNode, char* path) {

	TreeNode *next_dir = currentNode;
	List *curr_list = ((FolderContent *)(currentNode->content))->children;

	// Because the string pointer is modified by strtok
	char *aux_path = strdup(path);
	char *next_directory = strtok(aux_path, "/");

	while (next_directory) {
		if (!strcmp(next_directory, PARENT_DIR)) {
			next_dir = next_dir->parent;
			if (!next_dir) {
				free(aux_path);
				return currentNode;
			}
		} else {
			ListNode *child = ll_search(curr_list, next_directory);
			if (child) {
				next_dir = child->info;
			} else {
				free(aux_path);
				return NULL;
			}
		}
		curr_list = ((FolderContent *)next_dir->content)->children;
		next_directory = strtok(NULL, "/");
	}

	free(aux_path);
	return next_dir;
}


void tree(TreeNode* currentNode, char* arg) {
    // TODO
}


void mkdir(TreeNode* currentNode, char* folderName) {

    if (!currentNode) {
		fprintf(stderr, "No current node :(");
		return;
	}

	List *curr = ((FolderContent *)(currentNode->content))->children;
	TreeNode *info = calloc(1, sizeof(*info));
	info->parent = currentNode;
	info->name = strdup(folderName);
	info->type = FOLDER_NODE;
	info->content = calloc(1, sizeof(FolderContent));
	((FolderContent *)info->content)->children = ll_create();
	

	if (!ll_add_node(curr, info)) {
		free(info->name);
		if(info->content)
			free(info->content);
		free(info);	
	}
}


void rmrec(TreeNode* currentNode, char* resourceName) {
    List *list = ((FolderContent *)(currentNode->content))->children;
    ListNode *removed_node = ll_search(list, resourceName);
	if (!removed_node) {
		printf("rmrec: failed to remove '%s': No such file or directory", resourceName);
		return;
	}
	
	if (removed_node->info->type == FOLDER_NODE) {
		if (((FolderContent *)(removed_node->info->content))->children->head) {
			TreeNode *next_node = cd(currentNode, resourceName);
			List *list = ((FolderContent *)(next_node->content))->children;
			ll_free(list);
		}

		rmdir(currentNode, resourceName);
		return;
	}

	rm(currentNode, resourceName);
}


void rm(TreeNode* currentNode, char* fileName) {
	List *list = ((FolderContent *)(currentNode->content))->children;
    ListNode *removed_node = ll_search(list, fileName);
	if (!removed_node) {
		printf("rm: failed to remove '%s': No such file or directory", fileName);
		return;
	}

	if (removed_node->info->type != FILE_NODE) {
		printf("rm: cannot remove '%s': Is a directory", fileName);
		return;
	}

	removed_node = ll_remove_node(list, fileName);
	free(removed_node->info);
	free(removed_node);
}


void rmdir(TreeNode* currentNode, char* folderName) {

    List *list = ((FolderContent *)(currentNode->content))->children;
    ListNode *removed_node = ll_search(list, folderName);
	if (!removed_node) {
		printf("rmdir: failed to remove '%s': No such file or directory", folderName);
		return;
	}

	if (removed_node->info->type != FOLDER_NODE) {
		printf("rmdir: failed to remove '%s': Not a directory", folderName);
		return;
	}

	if (((FolderContent *)(removed_node->info->content))->children->head) {
		printf("rmdir: failed to remove '%s': Directory not empty", folderName);
		return;
	}

	removed_node = ll_remove_node(list, folderName);
	free(removed_node->info);
	free(removed_node);
}


void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
	if (!currentNode) {
		fprintf(stderr, "No current node :(");
		return;
	}

	List *curr = ((FolderContent *)(currentNode->content))->children;
	TreeNode *info = calloc(1, sizeof(*info));
	info->parent = currentNode;
	info->name = strdup(fileName);
	info->type = FILE_NODE;

	if (fileContent) {
		info->content = calloc(1, sizeof(FileContent));
		((FileContent *)info->content)->text = strdup(fileContent);
	} else {
		info->content = NULL;
	}

	if (!ll_add_node(curr, info)) {
		free(info->name);
		if(info->content)
			free(info->content);
		free(info);	
	}
}


void cp(TreeNode* currentNode, char* source, char* destination) {

	TreeNode *source_node = cd(currentNode, source);
	TreeNode *destination_node = move_to(currentNode, destination);

	if (source_node->type == FOLDER_NODE) {
		printf("cp: -r not specified; omitting directory '%s'\n", source);
		return;
	}

	if(destination_node == NULL) {
		printf("cp: failed to access '%s': Not a directory", destination);
		return;
	}

	if(destination_node->type == FILE_NODE) {
		((FileContent *)destination_node->content)->text =
		((FileContent *)source_node->content)->text;
		return;
	} else {
		List *list = ((FolderContent *)(destination_node->content))->children;
		ll_add_node(list, source_node);
		return;
	}
}

void mv(TreeNode* currentNode, char* source, char* destination) {
    // TODO
}

