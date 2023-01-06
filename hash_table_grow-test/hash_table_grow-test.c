//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (speed run)
//
// Test program for the hash_table_grow function.
//

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h>


//
// static configuration
//

#define _max_word_size_  32
#define _hash_table_init_size_ 1000

//
// data structures (SUGGESTION --- you may do it in a different way)
//

typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s      hash_table_t;

struct hash_table_node_s
{
	// the hash table data
	char word[_max_word_size_];        // the word
	hash_table_node_t *next;           // next hash table linked list node
									   // the vertex data
	int visited;                       // visited status (while not in use, keep it at 0)
	hash_table_node_t *previous;       // breadth-first search parent
									   // the union find data
	hash_table_node_t *representative; // the representative of the connected component this vertex belongs to
	int number_of_vertices;            // number of vertices of the conected component (only correct for the representative of each connected component)
	int number_of_edges;               // number of edges of the conected component (only correct for the representative of each connected component)
	int component_diameter;			   // only valid for the representative node
};

struct hash_table_s
{
	unsigned int hash_table_size;            // the size of the hash table array
	unsigned int largest_component_size;	 //size of the biggest component (passed on to breadh_first as max list size)
	unsigned int number_of_entries;    // the number of entries in the hash table
	unsigned int number_of_collisions; // the total of entries inserted on an occupied index
	unsigned int number_of_edges;      // number of edges (for information purposes only)
	unsigned int number_of_edge_nodes; // number of edges (for information purposes only)
	unsigned int number_of_components; // number of connected components
	hash_table_node_t **heads;         // the heads of the linked lists
};

//
// allocation and deallocation of linked list nodes (done)
//

static hash_table_node_t *allocate_hash_table_node(void)
{
	hash_table_node_t *node;

	node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
	if(node == NULL)
	{
		fprintf(stderr,"allocate_hash_table_node: out of memory\n");
		exit(1);
	}
	return node;
}

static void free_hash_llist(hash_table_node_t *head)
{
	hash_table_node_t *next;
	for (; head; head = next)
	{
		next = head->next;
		free(head);
	}
}


//
// hash table stuff (mostly to be done)
//

unsigned int crc32(const char *str)
{
	static unsigned int table[256];
	unsigned int crc;

	if(table[1] == 0u) // do we need to initialize the table[] array?
	{
		unsigned int i,j;

		for(i = 0u;i < 256u;i++)
			for(table[i] = i,j = 0u;j < 8u;j++)
				if(table[i] & 1u)
					table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
				else
					table[i] >>= 1;
	}
	crc = 0xAED02022u; // initial value (chosen arbitrarily)
	while(*str != '\0')
		crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
	return crc;
}

static hash_table_t *hash_table_create(void)
{
	hash_table_t *hash_table;

	hash_table = (hash_table_t *)calloc(1, sizeof(hash_table_t));
	if(hash_table == NULL)
	{
		fprintf(stderr,"create_hash_table: out of memory\n");
		exit(1);
	}
	hash_table->heads = (hash_table_node_t **)calloc(_hash_table_init_size_, sizeof(hash_table_node_t *));
	hash_table->hash_table_size = _hash_table_init_size_;
	return hash_table;
}

static void hash_table_grow(hash_table_t *hash_table)
{
	unsigned int		new_size;
	unsigned int		new_key;
	unsigned int		i;
	hash_table_node_t	**new_table;
	hash_table_node_t	*next;
	hash_table_node_t	*node;

	unsigned int		test_new_size;
	unsigned int		test_new_key;
	double				j;
	hash_table_node_t	**test_new_table;
	unsigned int		colnum;

	// Determine size_inc based on collision count
	if (hash_table->number_of_collisions > 0 && (hash_table->hash_table_size / hash_table->number_of_collisions) < 5)
	{
		// Find the best j
		printf("\nFinding best j. Current hash_table_size is %u.\n", hash_table->hash_table_size);
		printf("  j   | new size | memory | colnum\n");
		for (j = 1.1; j < 3; j += 0.005)
		{
			colnum = 0u;
			test_new_size = (double)hash_table->hash_table_size * j;
			test_new_table = (hash_table_node_t **)calloc(test_new_size, sizeof(hash_table_node_t *));

			for (i=0; i < hash_table->hash_table_size; i++)
			{
				for (node = hash_table->heads[i]; node; node = next)
				{
					test_new_key = crc32(node->word) % test_new_size;
					next = node->next;
					if (test_new_table[test_new_key])
					{
						colnum++;
					}
					test_new_table[test_new_key] = node;
				}
			}
			printf("%3.3f | %8u | %6lu | %6u\n", j, test_new_size, test_new_size * sizeof(hash_table_node_t *), colnum);
		}

		char chosen_j_char[10];
		printf("Choose j: ");
		scanf("%99s", chosen_j_char);

		new_size = hash_table->hash_table_size * atof(chosen_j_char);
		new_table = (hash_table_node_t **)calloc(new_size, sizeof(hash_table_node_t *));
		if (!new_table)
		{
			fprintf(stderr,"hash_table_grow: out of memory\n");	
			exit(1);
		}
		hash_table->number_of_collisions = 0u;
		for (i=0; i < hash_table->hash_table_size; i++)
			for (node = hash_table->heads[i]; node; node = next)
			{
				new_key = crc32(node->word) % new_size;
				next = node->next;
				node->next = new_table[new_key];
				if (node->next)
					hash_table->number_of_collisions++;
				new_table[new_key] = node;
			}
		printf("New number of collisions, with increment 2: %u\n", hash_table->number_of_collisions);
		free(hash_table->heads);
		hash_table->heads = new_table;
		hash_table->hash_table_size = new_size;
	}
}

static void hash_table_free(hash_table_t *hash_table)
{
	for (unsigned int i = 0; i < hash_table->hash_table_size; i++)
		if (hash_table->heads[i])
			free_hash_llist(hash_table->heads[i]); 
	free(hash_table->heads);
	free(hash_table);
}

static hash_table_node_t *create_word_node(const char *word)
{
	hash_table_node_t *node = allocate_hash_table_node();
	node->representative = node;
	node->visited = -1;
	node->number_of_vertices = 1;
	node->number_of_edges = 0;
	node->previous = NULL;
	node->next = NULL;
	strcpy(node->word, word);
	return node;
}

static hash_table_node_t *find_word(hash_table_t *hash_table,const char *word,int insert_if_not_found)
{
	hash_table_node_t *node;
	unsigned int i;

	i = crc32(word) % hash_table->hash_table_size;
	node = hash_table->heads[i];
	while (node)
	{
		if (strcmp(node->word, word) == 0)
			return node;
		node = node->next;
	}
	if (insert_if_not_found)
	{
		node = create_word_node(word);
		if (hash_table->heads[i])
			hash_table->number_of_collisions++;
		node->next = hash_table->heads[i];
		hash_table->heads[i] = node;
		hash_table->number_of_components++;
		hash_table->number_of_entries++;
		hash_table_grow(hash_table);
	}
	return node;
}

//
// main program
//

int main(int argc,char **argv)
{
	char word[100];
	hash_table_t *hash_table;
	FILE *fp;

	// initialize hash table
	hash_table = hash_table_create();
	
	// read words
	printf("Reading words from %s\n",(argc < 2) ? "wordlist-big-latest.txt" : argv[1]);
	fp = fopen((argc < 2) ? "wordlist-big-latest.txt" : argv[1],"rb");
	if(fp == NULL)
	{
		fprintf(stderr,"main: unable to open the words file\n");
		exit(1);
	}
	while(fscanf(fp,"%99s",word) == 1)
		(void)find_word(hash_table,word,1);
	fclose(fp);

	// clean up
	hash_table_free(hash_table);
	return 0;
}
