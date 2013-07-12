#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <sys/time.h>

enum state { RUNNING, LOST, WON, QUIT };

#define BTN_MINE	'\n'
#define BTN_FLAG	' '

int pos(int x, int y, int size_y) {
	return size_y*x + y;
}

void print_field(int current_pos, char* field, int size_x, int size_y) {
	int i, x, y;

	for (x = 0; x < size_x; ++x)
		for (y=0; y < size_y; ++y) {
			i = pos(y, x, size_x);
			mvaddch(y, x, abs(field[i]) | (i == current_pos ? A_STANDOUT : 0));
		}
}

void search(char* field, int x, int y, int size_x, int size_y) {
	int i = pos(y, x, size_x);
	if (field[i] != '.')
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

	for (i=0; i < size_x * size_y; ++i)
		if (abs(field[i]) == '.' || field[i] == 'f')
			return false;
	return true;
}

static int elapsed_time = 0;
void timer_tick(int x) {
	static int size_x = -1;

	if (size_x < 0)
		size_x = x;

	struct itimerval tout_val = {.it_value.tv_sec = 1};
	setitimer(ITIMER_REAL, &tout_val, 0);

	mvprintw(2, size_x + 2, "%d sec", elapsed_time++);
	refresh();
}

int main(int argc, char** argv) {
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	mousemask(BUTTON1_RELEASED | BUTTON3_RELEASED, 0);
	MEVENT event;

	int state = RUNNING;
	int size_x = 12;
	int size_y = 8;

	if (argc > 1)
		size_x = atoi(argv[1]);
	if (argc > 2)
		size_y = atoi(argv[2]);

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

	signal(SIGALRM, timer_tick);
	timer_tick(size_x);

	while (state == RUNNING) {
		mvprintw(1, size_x + 2, "%d, %d", x, y);

		print_field(pos(y, x, size_x), field, size_x, size_y);
		move(y,x);

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
		case KEY_MOUSE:
			if(getmouse(&event) == OK && event.x < size_x && event.y < size_y) {
				x = event.x;
				y = event.y;

				if (event.bstate & BUTTON1_RELEASED)
					goto mine;
				if (event.bstate & BUTTON3_RELEASED)
					goto flag;
			}
			break;
		case BTN_MINE:
		mine:
			i = pos(y, x, size_x);
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
		case BTN_FLAG:
		flag:
			i = pos(y, x, size_x);
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

	print_field(-1, field, size_x, size_y);

	struct itimerval tout_val = {{0}};	// stop timer
	setitimer(ITIMER_REAL, &tout_val, 0);

	switch (state) {
	case WON:
		mvprintw(3, size_x + 2, ":-)");
		refresh();
		getch();

		break;
	case LOST:
		mvprintw(3, size_x + 2, ":-(");
		refresh();
		getch();

		break;
	case QUIT:
		break;
	}

	endwin();
	return 0;
}
