// This code is intended to be used in the LaTeX report, so don't compile it.
// It doesn't include the normal flow of the program (default 2x increment).

static void hash_table_grow(hash_table_t *hash_table)
{
	unsigned int		i;
	double				j;
	unsigned int		k;
	unsigned int		test_new_size;
	unsigned int		test_new_key;
	hash_table_node_t	*next;
	hash_table_node_t	*node;
	hash_table_node_t	**test_new_table;
	unsigned int		colnum;
	unsigned int		free_entries;

	// Determine size_inc based on collision count
	if (hash_table->number_of_collisions > 0 && (hash_table->hash_table_size / hash_table->number_of_collisions) < 5)
	{
		// Find the best j
		printf("\nFinding best j. Current hash_table_size is %u.\n", hash_table->hash_table_size);
		printf("  j   | new size | memory | free m | colnum\n");
		for (j = 1.1; j < 3; j += 0.005)
		{
			colnum = 0u;
			free_entries = 0u;
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
			for (k=0; k < test_new_size; k++) {
				if (!test_new_table[k]) {
					free_entries++;
				}
			}
			printf("%3.3f | %8u | %6lu | %6lu | %6u\n", j, test_new_size, test_new_size * sizeof(hash_table_node_t *), free_entries * sizeof(hash_table_node_t *), colnum);
		}
	}
}