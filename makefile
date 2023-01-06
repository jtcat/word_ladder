#
# Tomás Oliveira e Silva, AED, November 2022
#
# makefile to compile the A.02 assignment (word ladder)
#

all:	solution_word_ladder debug_solution_word_ladder 

word_ladder:		word_ladder.c
	cc -Wall -Wextra -O2 word_ladder.c -o word_ladder -lm

solution_word_ladder:	solution_word_ladder.c
	cc -Wall -Wextra -O2 solution_word_ladder.c -o solution_word_ladder -lm

debug_solution_word_ladder:	solution_word_ladder.c
	cc -g -Wall -Wextra -O0 solution_word_ladder.c -o debug_solution_word_ladder -lm

# ---------------------------------------------------------
# Program fork to test the hash_table_grow resize condition
# ---------------------------------------------------------

hash_table_grow_test: ./hash_table_grow-test/hash_table_grow-test.c
	cc -Wall -Wextra -O2 ./hash_table_grow-test/hash_table_grow-test.c -o ./hash_table_grow-test/hash_table_grow_test -lm

debug_hash_table_grow_test: ./hash_table_grow-test/hash_table_grow-test.c
	cc -g -Wall -Wextra -O0 ./hash_table_grow-test/hash_table_grow-test.c -o ./hash_table_grow-test/debug_hash_table_grow_test -lm

# ---------------------------------------------------------
# Clean & redo scripts
# ---------------------------------------------------------

clean:
	rm -rf a.out word_ladder solution_word_ladder debug_solution_word_ladder

re:	clean all
