//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (speed run)
//
// Place your student numbers and names here
//   N.Mec. 93096  Name: João Catarino
//
// Do as much as you can
//   1) MANDATORY: complete the hash table code
//      *) hash_table_create 		DONE
//      *) hash_table_grow			WORKING	-Requires better resize condition and
//      									 increment rule.			
//      *) hash_table_free			DONE
//      *) find_word				DONE
//      +) add code to get some statistical data about the hash table
//
//   2) HIGHLY RECOMMENDED: build the graph (including union-find data) -- use the similar_words function...
//      *) find_representative		DONE
//      *) add_edge					DONE
//
//   3) RECOMMENDED: implement breadth-first search in the graph
//      *) breadh_first_search		DONE
//
//   4) RECOMMENDED: list all words belonging to a connected component
//      *) breadh_first_search		DONE
//      *) list_connected_component	DONE
//
//   5) RECOMMENDED: find the shortest path between to words
//      *) breadh_first_search		DONE
//      *) path_finder				DONE
//      *) test the smallest path from bem to mal
//         [ 0] bem
//         [ 1] tem
//         [ 2] teu
//         [ 3] meu
//         [ 4] mau
//         [ 5] mal
//      *) find other interesting word ladders
//
//   6) OPTIONAL: compute the diameter of a connected component and list the longest word chain
//      *) breadh_first_search
//      *) connected_component_diameter
//
//   7) OPTIONAL: print some statistics about the graph
//      *) graph_info
//
//   8) OPTIONAL: test for memory leaks
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

typedef struct adjacency_node_s  adjacency_node_t;
typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s      hash_table_t;
typedef struct ptr_queue_s 		 ptr_queue_t;

struct ptr_queue_s
{
	void 			**circular_array;
	unsigned int	hi;
	unsigned int	lo;
	unsigned int	size;
	unsigned int	max_size;
	int		full;
};

struct adjacency_node_s
{
	adjacency_node_t *next;            // link to th enext adjacency list node
	hash_table_node_t *vertex;         // the other vertex
};

struct hash_table_node_s
{
	// the hash table data
	char word[_max_word_size_];        // the word
	hash_table_node_t *next;           // next hash table linked list node
									   // the vertex data
	adjacency_node_t *head;            // head of the linked list of adjancency edges
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
// allocation and deallocation of queue
//

static ptr_queue_t *allocate_ptr_queue(unsigned int max_size)
{
	ptr_queue_t	*queue = (ptr_queue_t *)malloc(sizeof(ptr_queue_t));
	if(queue == NULL)
	{
		fprintf(stderr,"allocate_ptr_queue: out of memory\n");
		exit(1);
	}
	queue->circular_array = (void **)malloc(sizeof(void *) * max_size);
	if(queue->circular_array == NULL)
	{
		fprintf(stderr,"allocate_ptr_queue->circular_array: out of memory\n");
		free(queue);
		exit(1);
	}
	queue->max_size = max_size;
	queue->size = 0;
	queue->full = 0;
	queue->hi = 0;
	queue->lo = 0;
	return queue;
}

static void free_ptr_queue(ptr_queue_t *queue)
{
	free(queue->circular_array);
	free(queue);
}

//
// queue methods
//

static void queue_put_hi(ptr_queue_t *queue, void *ptr)
{
	assert(queue->size < queue->max_size);
	queue->circular_array[queue->hi] = ptr;
	queue->hi = (queue->hi + 1) % queue->max_size;
	queue->size++;
}

static void *queue_get_lo(ptr_queue_t *queue)
{
	assert(queue->size > 0);
	void *ret = queue->circular_array[queue->lo];
	queue->lo = (queue->lo + 1) % queue->max_size;
	queue->size--;
	return ret;
}

//
// allocation and deallocation of linked list nodes (done)
//

static adjacency_node_t *allocate_adjacency_node(void)
{
	adjacency_node_t *node;

	node = (adjacency_node_t *)malloc(sizeof(adjacency_node_t));
	if(node == NULL)
	{
		fprintf(stderr,"allocate_adjacency_node: out of memory\n");
		exit(1);
	}
	return node;
}

static void free_adjacency_llist(adjacency_node_t *head)
{
	adjacency_node_t *next;
	for (; head; head = next)
	{
		next = head->next;
		free(head);
	}
}

static void free_hash_table_node(hash_table_node_t *node)
{
	free_adjacency_llist(node->head);
	free(node);
}

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
		free_hash_table_node(head);
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
	if(hash_table->heads == NULL)
	{
		fprintf(stderr,"create_hash_table: out of memory for array\n");
		exit(1);
	}
	hash_table->hash_table_size = _hash_table_init_size_;
	return hash_table;
}

static void hash_table_info(hash_table_t *hash_table)
{
	printf("Entries: %u\nCollisions: %u\nSize: %u\n",
			hash_table->number_of_entries,
			hash_table->number_of_collisions,
			hash_table->hash_table_size);
}

static void hash_table_grow(hash_table_t *hash_table)
{
	unsigned int		new_size;
	unsigned int		new_key;
	unsigned int		i;
	hash_table_node_t	**new_table;
	hash_table_node_t	*next;
	hash_table_node_t	*node;
	// Determine size_inc based on collision count
	if (hash_table->number_of_collisions > 0 && (hash_table->hash_table_size / hash_table->number_of_collisions) < 5)
	{
		new_size = hash_table->hash_table_size * 2;
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
	node->head = NULL;
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
// add edges to the word ladder graph (mostly do be done)
//

static hash_table_node_t *find_representative(hash_table_node_t *node)
{
	hash_table_node_t *representative,*next_node;

	for(representative = node; representative != representative->representative; representative = representative->representative);

	for(next_node = node; next_node != representative; next_node = node)
	{
		node = next_node->representative;
		next_node->representative = representative;
	}
	return representative;
}

static void insert_edge(hash_table_t *hash_table, hash_table_node_t *from, hash_table_node_t *to)
{
	adjacency_node_t *link;

	link = allocate_adjacency_node();
	link->vertex = to;
	link->next = from->head;
	from->head = link;
	hash_table->number_of_edge_nodes++;
}

static void add_edge(hash_table_t *hash_table,hash_table_node_t *from,const char *word)
{
	hash_table_node_t *to,*from_representative,*to_representative;
	adjacency_node_t *link;

	to = find_word(hash_table,word,0);
	if (!to)
		return;
	for (link = from->head; link && link->vertex != to; link = link->next);
	if (link)
		return;
	hash_table->number_of_edges++;
	insert_edge(hash_table, from, to);
	insert_edge(hash_table, to, from);

	from_representative = find_representative(from);
	to_representative = find_representative(to);

	from_representative->number_of_edges++;
	if (from_representative != to_representative)
	{
		unsigned int	vert_sum = from_representative->number_of_vertices + to_representative->number_of_vertices;	
		unsigned int	edge_sum = from_representative->number_of_edges + to_representative->number_of_edges;
		int					cond = to_representative->number_of_vertices > from_representative->number_of_vertices;
		hash_table_node_t	*new_rep = cond ? from_representative : to_representative;
		new_rep->number_of_vertices = vert_sum;
		new_rep->number_of_edges = edge_sum;
		(cond ? to_representative : from_representative)->representative = new_rep;
		hash_table->number_of_components--;
		if (vert_sum > hash_table->largest_component_size)
			hash_table->largest_component_size = vert_sum;
	}
}


//
// generates a list of similar words and calls the function add_edge for each one (done)
//
// man utf8 for details on the uft8 encoding
//

static void break_utf8_string(const char *word,int *individual_characters)
{
	int byte0,byte1;

	while(*word != '\0')
	{
		byte0 = (int)(*(word++)) & 0xFF;
		if(byte0 < 0x80)
			*(individual_characters++) = byte0; // plain ASCII character
		else
		{
			byte1 = (int)(*(word++)) & 0xFF;
			if((byte0 & 0b11100000) != 0b11000000 || (byte1 & 0b11000000) != 0b10000000)
			{
				fprintf(stderr,"break_utf8_string: unexpected UFT-8 character\n");
				exit(1);
			}
			*(individual_characters++) = ((byte0 & 0b00011111) << 6) | (byte1 & 0b00111111); // utf8 -> unicode
		}
	}
	*individual_characters = 0; // mark the end!
}

static void make_utf8_string(const int *individual_characters,char word[_max_word_size_])
{
	int code;

	while(*individual_characters != 0)
	{
		code = *(individual_characters++);
		if(code < 0x80)
			*(word++) = (char)code;
		else if(code < (1 << 11))
		{ // unicode -> utf8
			*(word++) = 0b11000000 | (code >> 6);
			*(word++) = 0b10000000 | (code & 0b00111111);
		}
		else
		{
			fprintf(stderr,"make_utf8_string: unexpected UFT-8 character\n");
			exit(1);
		}
	}
	*word = '\0';  // mark the end
}

static void similar_words(hash_table_t *hash_table,hash_table_node_t *from)
{
	static const int valid_characters[] =
	{ // unicode!
		0x2D,                                                                       // -
		0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,           // A B C D E F G H I J K L M
		0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,           // N O P Q R S T U V W X Y Z
		0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,           // a b c d e f g h i j k l m
		0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,           // n o p q r s t u v w x y z
		0xC1,0xC2,0xC9,0xCD,0xD3,0xDA,                                              // Á Â É Í Ó Ú
		0xE0,0xE1,0xE2,0xE3,0xE7,0xE8,0xE9,0xEA,0xED,0xEE,0xF3,0xF4,0xF5,0xFA,0xFC, // à á â ã ç è é ê í î ó ô õ ú ü
		0
	};
	int i,j,k,individual_characters[_max_word_size_];
	char new_word[2 * _max_word_size_];

	break_utf8_string(from->word,individual_characters);
	for(i = 0;individual_characters[i] != 0;i++)
	{
		k = individual_characters[i];
		for(j = 0;valid_characters[j] != 0;j++)
		{
			individual_characters[i] = valid_characters[j];
			make_utf8_string(individual_characters,new_word);
			// avoid duplicate cases
			if(strcmp(new_word,from->word) > 0)
				add_edge(hash_table,from,new_word);
		}
		individual_characters[i] = k;
	}
}


//
// breadth-first search (to be done)
//
// returns the number of vertices visited; if the last one is goal, following the previous links gives the shortest path between goal and origin
//

static unsigned int breadh_first_search(unsigned int maximum_number_of_vertices,hash_table_node_t **list_of_vertices,hash_table_node_t *origin,hash_table_node_t *goal)
{	
	unsigned int		list_len;
	hash_table_node_t	*node;
	adjacency_node_t	*link;
	ptr_queue_t 		*queue;

	queue = allocate_ptr_queue(maximum_number_of_vertices);

	list_len = 0;
	queue_put_hi(queue, origin);
	while (queue->size > 0)
	{
		node = queue_get_lo(queue);
		node->visited++;
		if (list_of_vertices)
			list_of_vertices[list_len] = node;
		list_len++;
		if (node == goal)
			break;
		for(link = node->head; link && list_len < maximum_number_of_vertices; link = link->next)
		{
			if (link->vertex->visited == -1)
			{
				link->vertex->visited = node->visited;
				link->vertex->previous = node;
				queue_put_hi(queue, link->vertex);	
			}
			else if ((node->visited + 1) < link->vertex->visited)
			{
				link->vertex->visited = node->visited + 1;
				link->vertex->previous = node;
			}
		}
	}
	free_ptr_queue(queue);
	if (goal && goal != node)
		return 0;
	return list_len;
}


//
// list all vertices belonging to a connected component (complete this)
//

static void mark_all_vertices(hash_table_t *hash_table)
{
	hash_table_node_t *node;

	for(unsigned int i = 0;i < hash_table->hash_table_size;i++)
		for(node = hash_table->heads[i];node != NULL;node = node->next)
			node->visited = -1;
}

static void list_connected_component(hash_table_t *hash_table,const char *word)
{
	hash_table_node_t	*origin, *rep;
	hash_table_node_t	**list;
	unsigned int		list_len;

	origin = find_word(hash_table, word, 0);
	if (!origin)
	{
		printf("\nWord not found: %s\n", word);
		return;
	}

	mark_all_vertices(hash_table);
	rep = find_representative(origin);
	list = malloc(rep->number_of_vertices * sizeof(hash_table_node_t *));
	list_len = breadh_first_search(rep->number_of_vertices, list, origin, NULL);
	for (unsigned int i=0; i < list_len; i++)
		printf("%s\n", list[i]->word);
	free(list);
}


//
// compute the diameter of a connected component (optional)
//

static int largest_diameter;
static hash_table_node_t **largest_diameter_example;

static int connected_component_diameter(hash_table_node_t *node)
{
	int					diameter;
	unsigned int		j, i, comp_len, search_len;
	hash_table_node_t	**comp_nodes, **node_list, *chain_start, *chain_end;

	diameter = 0;
	chain_start = chain_end = NULL;
	comp_nodes = calloc(node->representative->number_of_vertices, sizeof(hash_table_node_t *));
	comp_len = breadh_first_search(node->representative->number_of_vertices, comp_nodes, node, NULL);
	for (i = 0; i < comp_len; i++)
	{
		for (j = 0; j < comp_len; j++)
			comp_nodes[j]->visited = -1;
		node_list = calloc(node->representative->number_of_vertices, sizeof(hash_table_node_t *));
		search_len = breadh_first_search(node->representative->number_of_vertices, node_list, comp_nodes[i], NULL);
		for (j = 0; j < search_len; j++)
			if (node_list[j]->visited >= diameter)
			{
				diameter = node_list[j]->visited;
				chain_start = comp_nodes[i];
				chain_end = node_list[j];
			}
		free(node_list);
	}
	if (diameter > largest_diameter)
	{
		largest_diameter = diameter;
		if (largest_diameter_example)
			free(largest_diameter_example);
		largest_diameter_example = calloc(diameter, sizeof(hash_table_node_t *));
		i = diameter;
		for (node = chain_end; node != chain_start; node = node->previous)
			largest_diameter_example[--i] = node;
	}
	free(comp_nodes);
	return diameter;
}


//
// find the shortest path from a given word to another given word (to be done)
//

static void path_finder(hash_table_t *hash_table,const char *from_word,const char *to_word)
{
	hash_table_node_t	*from, *to;
	size_t				i, list_len;

	// switch from with to in order to print path in correct order
	from = find_word(hash_table, from_word, 0);
	to = find_word(hash_table, to_word, 0);

	if (!from)
	{
		fprintf(stderr, "\nWord not found: %s\n", from_word);
		return ;
	}
	if (!to)
	{
		fprintf(stderr, "\nWord not found: %s\n", to_word);
		return ;
	}

	mark_all_vertices(hash_table);
	list_len = breadh_first_search(find_representative(to)->number_of_vertices, NULL, to, from);
	if (list_len == 0)
		fprintf(stderr, "Words are not connected\n");
	else
	{
		i = 0;
		for(; from && from != to; from = from->previous)
			printf("  [%zu] %s\n", i++, from->word);
		printf("  [%zu] %s\n", i++, from->word);
	}
}


static void component_info(hash_table_t *hash_table, char *word)
{
	hash_table_node_t	*origin, *rep;

	origin = find_word(hash_table, word, 0);
	if (!origin)
		return (void)fprintf(stderr, "\nWord not found.\n");
	rep = find_representative(origin);
	printf("\nRepresentative: %s\nVertices: %u\nEdges: %u\nDiameter: %u\n",
			rep->word,
			rep->number_of_vertices,
			rep->number_of_edges,
			rep->component_diameter);
}

//
// some graph information (optional)
//

static void graph_info(hash_table_t *hash_table)
{
	printf("\nNodes: %u\nEdges: %u\nEdge nodes: %u\nComponents: %u\nLargest Component: %u\n",
			hash_table->number_of_entries,
			hash_table->number_of_edges,
			hash_table->number_of_edge_nodes,
			hash_table->number_of_components,
			hash_table->largest_component_size);
}


//
// main program
//

int main(int argc,char **argv)
{
	char word[100],from[100],to[100];
	hash_table_t *hash_table;
	hash_table_node_t *node, *rep;
	unsigned int i;
	int command;
	FILE *fp;

	largest_diameter_example = NULL;
	largest_diameter = 0;
	// initialize hash table
	hash_table = hash_table_create();
	// read words
	fp = fopen((argc < 2) ? "wordlist-big-latest.txt" : argv[1],"rb");
	if(fp == NULL)
	{
		fprintf(stderr,"main: unable to open the words file\n");
		exit(1);
	}
	while(fscanf(fp,"%99s",word) == 1)
		(void)find_word(hash_table,word,1);
	fclose(fp);
	// find all similar words
	for(i = 0u;i < hash_table->hash_table_size;i++)
		for(node = hash_table->heads[i];node != NULL;node = node->next)
			similar_words(hash_table,node);
	for(i =0u;i < hash_table->hash_table_size;i++)
		for(node = hash_table->heads[i];node != NULL;node = node->next)
		{
			if (node->visited == -1)
			{
				rep = find_representative(node);
				rep->component_diameter = connected_component_diameter(node);
			}
		}
	printf("Largest diameter: %d, from component: %s\n", largest_diameter, find_representative(largest_diameter_example[0])->word);
	printf("Largest word chain:\n");
	for(i=0; i < (unsigned int)largest_diameter; i++)
		printf("	[%d] %s\n", i, largest_diameter_example[i]->word);
	// ask what to do
	for(;;)
	{
		fprintf(stderr,"\nYour wish is my command:\n");
		fprintf(stderr,"  1 WORD       (list the connected component WORD belongs to)\n");
		fprintf(stderr,"  2 FROM TO    (list the shortest path from FROM to TO)\n");
		fprintf(stderr,"  3 WORD       (list component info)\n");
		fprintf(stderr,"  4            (list hash table info)\n");
		fprintf(stderr,"  5            (list graph info)\n");
		fprintf(stderr,"  0            (terminate)\n");
		fprintf(stderr,"> ");
		if(scanf("%99s",word) != 1)
			break;
		command = atoi(word);
		if(command == 1)
		{
			if(scanf("%99s",word) != 1)
				break;
			list_connected_component(hash_table,word);
		}
		else if(command == 2)
		{
			if(scanf("%99s",from) != 1)
				break;
			if(scanf("%99s",to) != 1)
				break;
			path_finder(hash_table,from,to);
		}
		else if(command == 3)
		{
			if(scanf("%99s",word) != 1)
				break;
			component_info(hash_table, word);
		}
		else if(command == 4)
			hash_table_info(hash_table);		
		else if(command == 5)
			graph_info(hash_table);
		else if(command == 0)
			break;
	}
	// clean up
	hash_table_free(hash_table);
	if (largest_diameter_example)
		free(largest_diameter_example);
	return 0;
}
