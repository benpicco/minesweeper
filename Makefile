mines: mines.c
	gcc -Os -Wall mines.c -lrt -lncurses -o mines
	size mines
