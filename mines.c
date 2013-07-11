#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

enum state { RUNNING, LOST, WON, QUIT };

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

bool cleared(char* field, int size_x, int size_y) {
	int i;

	for (i=0; i < size_x * size_y; ++i) {
		if (abs(field[i]) == '.' || field[i] == 'f')
			return false;
	}

	return true;
}

int main() {
	initscr();
	raw();
	noecho();
	curs_set(2);
	keypad(stdscr, TRUE);

	int state = RUNNING;
	int size_x = 12;
	int size_y = 8;

	int mines = 0;
	int i, x, y;
	x=y=0;

	char* field = calloc(size_x * size_y, sizeof(char));

	for (i=0; i < size_x * size_y; ++i) {
		field[i] = '.';
		if (rand() % 6 == 0) {
			field[i] *= -1;
			++mines;
		}
	}

	while (state == RUNNING) {
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
		case '\n':
			if (field[i] < 0) {
				for (i = 0; i < size_x * size_y; ++i)
					if (field[i] < 0)
						field[i] = 'X';
				state = LOST;
			} else {
				search(field, x, y, size_x, size_y);
				if (mines == 0 && cleared(field, size_x, size_y))
					state = WON;
			}
			break;
		case ' ':
			neg = field[i] < 0 ? -1 : 1;
			if (abs(field[i]) == '.') {
				field[i] = 'f' * neg;
				if (field[i] < 0)
					--mines;
				if (mines == 0 && cleared(field, size_x, size_y))
					state = WON;
			} else if (abs(field[i]) == 'f') {
				field[i] = '.' * neg;
				if (field[i] < 0)
					++mines;
			}
			break;
		case 'q':
			state = QUIT;
			break;
		}

		refresh();
	}

	switch (state) {
	case WON:
		mvprintw(3, 14, ":-)");
		refresh();
		getch();

		break;
	case LOST:
		mvprintw(3, 14, ":-(");
		refresh();
		getch();

		break;
	case QUIT:
		break;
	}

	endwin();
	return 0;
}
