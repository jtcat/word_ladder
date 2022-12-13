#
# Tomás Oliveira e Silva, AED, November 2022
#
# makefile to compile the A.02 assignment (word ladder)
#

word_ladder:		word_ladder.c
	cc -Wall -Wextra -O2 word_ladder.c -o word_ladder -lm

solution_word_ladder:	solution_word_ladder.c
	cc -g -Wall -Wextra -O2 solution_word_ladder.c -o solution_word_ladder -lm

debug_solution_word_ladder:	solution_word_ladder.c
	cc -g -Wall -Wextra -O0 solution_word_ladder.c -o debug_solution_word_ladder -lm

clean:
	rm -rf a.out word_ladder solution_word_ladder debug_solution_word_ladder
