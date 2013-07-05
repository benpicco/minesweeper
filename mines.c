#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int pos(int x, int y, int size_y) {
	return size_y*x + y;
}

void print_field(char* field, int size_x, int size_y) {
	int x, y;

	for (x = 0; x < size_x; ++x) {
		for (y=0; y < size_y; ++y) {
			mvprintw(y, x, "%c", abs(field[pos(y,x, size_x)]));
		}
	}
}

void search(char* field, int x, int y, int size_x, int size_y) {
	int i = pos(y, x, size_x);
	if (abs(field[i]) != '.')
		return;

	int count = 0;

	if (x > 0 && y > 0 && field[pos(y-1, x-1, size_x)] < 0)
		++count;
	if (x > 0 && field[pos(y, x-1, size_x)] < 0)
		++count;
	if (x > 0 && y < size_y - 1 && field[pos(y+1, x-1, size_x)] < 0)
		++count;
	if (y > 0 && field[pos(y-1, x, size_x)] < 0)
		++count;
	if (y < size_y - 1 && field[pos(y+1, x, size_x)] < 0)
		++count;
	if (x < size_x -1 && y > 0 && field[pos(y-1, x+1, size_x)] < 0)
		++count;
	if (x < size_x - 1 && field[pos(y, x+1, size_x)] < 0)
		++count;
	if (x < size_x -1 && y < size_y -1 && field[pos(y+1, x+1, size_x)] < 0)
		++count;

	if (count > 0) {
		field[i] = '0' + count;
	} else {
		field[i] = ' ';
		if (x > 0)
			search(field, x-1, y, size_x, size_y);
		if (x < size_x - 1)
			search(field, x+1, y, size_x, size_y);
		if (y > 0)
			search(field, x, y-1, size_x, size_y);
		if (y < size_y - 1)
			search(field, x, y+1, size_x, size_y);
	}
}

int main() {
	initscr();
	raw();
	noecho();
	curs_set(2);
	keypad(stdscr, TRUE);

	int running = TRUE;
	int size_x = 12;
	int size_y = 8;

	int i, x, y;
	x=y=0;

	char* field = calloc(size_x * size_y, sizeof(char));

	for (i=0; i < size_x * size_y; ++i) {
		field[i] = '.';
		if (rand() % 13 == 0)
			field[i] *= -1;
	}

	while (running) {
		print_field(field, size_x, size_y);
		mvprintw(1, 14, "%d, %d", x, y);
		mvprintw(2, 14, "%c", field[pos(y, x, size_x)] < 0 ? 'B' : ' ');

		move(y,x);

		i=pos(y, x, size_x);
		int c = getch();
		int neg;
		switch (c) {
		case KEY_UP:
			if (y > 0)
				--y;
			break;
		case KEY_DOWN:
			if (y < size_y - 1)
				++y;
			break;
		case KEY_LEFT:
			if (x > 0)
				--x;
			break;
		case KEY_RIGHT:
			if (x < size_x - 1)
				++x;
			break;
		case 'b':
			if (field[i] < 0) {
				for (i = 0; i < size_x * size_y; ++i)
					if (field[i] < 0)
						field[i] = 'X';
			} else {
				search(field, x, y, size_x, size_y);
			}
			break;
		case ' ':
			neg = field[i] < 0 ? -1 : 1;
			if (abs(field[i]) == '.')
				field[i] = 'f' * neg;
			else if (abs(field[i]) == 'f')
				field[i] = '.' * neg;
			break;
		case 'q':
			running = FALSE;
			break;
		}

		refresh();
	}

	endwin();
	return 0;
}
