/* 
Features of thisproject 
two difficulty modes 
colored board display 
match detection (3, 4, 5 in a row, and L shapes) 
gravity and cascading effects
high score system 
save and load game functionality 
sound effects
*/
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <conio.h>  
#include <windows.h>
#include <fstream>
#include <iomanip>

using namespace std;

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

const int maxrows = 10;
const int maxcols = 10;

int rows = 8;
int cols = 8;
int score = 0;
int timelimit = 60;
int movesleft = 0;
time_t starttime;
time_t pausetime = 0;
string name;
bool ishardmode = false;

char candies[] = { '@', '#', '&', '$', '%', '!', '*' };
int candycount = 5;

char board[maxrows][maxcols];

//function declaratiom
void mainmenu();
void gameloop();
void initboard();
void displayBoard();
bool valid(int r, int c);
void swapcandy(int r1, int c1, int r2, int c2);
bool matches(bool animate);
void gravity(bool animate);
void savegame();
bool loadgame();
void updatescore();
void showscore();
void setcolor(int color);
void gotoxy(int x, int y);

// this sets the console text color
void setcolor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

//moving thr cursor to x,y
void gotoxy(int x, int y) {
	COORD coordinates;
	coordinates.X = x;
	coordinates.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordinates);
}

//maping candies to colors
int getcandycolor(char c) {
	switch (c) {
	case '@': return 12; //red
	case '#': return 10; //green
	case '&': return 9;  //blue
	case '$': return 14; //yellow
	case '%': return 13; //magenta
	case '!': return 11; //cyan
	case '*': return 15; //white
	default: return 7;   //gray
	}
}

//initializinf board with random candies
void initboard() {
	bool stable = false;

	while (!stable) {
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				board[i][j] = candies[rand() % candycount];
			}
		}

		stable = true;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (j < cols - 2 && board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2])
					stable = false;
				if (i < rows - 2 && board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j])
					stable = false;
			}
		}
	}
}

//displaying the board with colors
void displayBoard() { 
	system("cls");
	setcolor(15);

	cout << "CANDY CRUSH SAGA\n\n";
	cout << "Player: " << name << endl;
	cout << "Score : " << score << endl;

	time_t now = time(0);
	int elapsed = (int)difftime(now, starttime) - pausetime;
	int remaining = timelimit - elapsed;
	if (remaining < 0) remaining = 0;
	cout << "Time  : " << remaining / 60 << ":" << setw(2) << setfill('0')
		<< remaining % 60 << setfill(' ') << "\n\n";

	// column nunbers
	cout << "    ";
	for (int j = 0; j < cols; j++) cout << j << "  ";
	cout << "\n";

	//board rows
	for (int i = 0; i < rows; i++) {
		cout << setw(2) << i << "  ";
		for (int j = 0; j < cols; j++) {
			setcolor(getcandycolor(board[i][j]));
			cout << board[i][j] << "  ";
		}
		setcolor(15);
		cout << "\n";
	}

	cout << "\nControls:\n";
	cout << "- Enter row col for example 3 4 \n";
	cout << "- Arrow Keys to swap\n";
	cout << "- Enter 'P' to pause\n\n";
}

//position valid
bool valid(int r, int c) {
	return (r >= 0 && r < rows && c >= 0 && c < cols);
}

//swap two candies
void swapcandy(int r1, int c1, int r2, int c2) {
	char temp = board[r1][c1];
	board[r1][c1] = board[r2][c2];
	board[r2][c2] = temp;
}

//process matches 
bool matches(bool animate) {
	bool foundMatch = false;
	bool todelete[maxrows][maxcols] = { false };

	// horizontal 
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols - 2; j++) {
			char c = board[i][j];
			if (c == ' ') continue;
			int k = j + 1;
			while (k < cols && board[i][k] == c) k++;
			if (k - j >= 3) {
				for (int l = j; l < k; l++) todelete[i][l] = true;
				foundMatch = true;
			}
			j = k - 1;
		}
	}

	// vertical matches
	for (int j = 0; j < cols; j++) {
		for (int i = 0; i < rows - 2; i++) {
			char c = board[i][j];
			if (c == ' ') continue;
			int k = i + 1;
			while (k < rows && board[k][j] == c) k++;
			if (k - i >= 3) {
				for (int l = i; l < k; l++) todelete[l][j] = true;
				foundMatch = true;
			}
			i = k - 1;
		}
	}

	int removedcount = 0;
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			if (todelete[i][j]) {
				board[i][j] = ' ';
				removedcount++;
			}

	if (removedcount == 3) score += 10;
	else if (removedcount == 4) score += 15;
	else if (removedcount >= 5) score += 20;

	if (foundMatch) {
		if (animate) {
			Beep(523, 100);
			displayBoard();
			Sleep(200);
		}
		gravity(animate);
	}

	return foundMatch;
}

// gravity nd refill candies
void gravity(bool animate) {
	for (int j = 0; j < cols; j++) {
		int empty = rows - 1;
		for (int i = rows - 1; i >= 0; i--) {
			if (board[i][j] != ' ') {
				board[empty][j] = board[i][j];
				if (empty != i) board[i][j] = ' ';
				empty--;
			}
		}
		for (int i = empty; i >= 0; i--) {
			board[i][j] = candies[rand() % candycount];
		}
	}

	if (animate) {
		displayBoard();
		Sleep(200);
	}

	while (matches(animate));
}

// main game loop
void gameloop() {
	initboard();
	starttime = time(0);
	pausetime = 0;

	while (true) {
		displayBoard();

		time_t now = time(0);
		int elapsed = (int)difftime(now, starttime) - pausetime;
		if (elapsed >= timelimit) {
			system("cls");
			cout << "\nGAME OVER TIME UP\n";
			cout << "Final score: " << score << "\n\n";
			updatescore();
			system("pause");
			return;
		}

		cout << "enter row and column or -1 to pause: ";
		int r, c;

		if (!(cin >> r)) {
			cin.clear(); cin.ignore(1000, '\n');
			continue;
		}

		if (r == -1) {
			cout << "enter (S for save & exit) or (C to continue): ";
			char ch; cin >> ch;
			if (ch == 'S' || ch == 's') { savegame(); return; }
			continue;
		}

		cin >> c;
		if (!valid(r, c)) continue;

		cout << "Use arrow key to swap:  ";
		int key = _getch();
		if (key == 0 || key == 224) {
			int r2 = r, c2 = c;
			switch (_getch()) {
			case KEY_UP: r2--; break;
			case KEY_DOWN: r2++; break;
			case KEY_LEFT: c2--; break;
			case KEY_RIGHT: c2++; break;
			}
			if (!valid(r2, c2)) continue;

			swapcandy(r, c, r2, c2);
			displayBoard();
			Sleep(150);

			if (!matches(true))
				swapcandy(r, c, r2, c2);
		}
	}
}

// save top scores to highscore.txt wali file
void updatescore() {
	ifstream in("highscore.txt"); // read existing high scores
	string names[100];
	int scores[100];
	int count = 0;

	while (in >> names[count] >> scores[count]) count++;
	in.close();

	names[count] = name;
	scores[count] = score;
	count++;

	// sort descending
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			if (scores[j] < scores[j + 1]) {
				swap(scores[j], scores[j + 1]);
				swap(names[j], names[j + 1]);
			}
		}
	}

	ofstream out("highscore.txt");
	for (int i = 0; i < min(10, count); i++)
		out << names[i] << " " << scores[i] << endl;
	out.close();

	cout << "Score saved to highscore.txt.\n";
}

// Display high scores
void showscore() {
	system("cls");
	cout << "Top 10 highscores \n\n";

	ifstream in("highscore.txt");
	string n; int s; int rank = 1;
	while (in >> n >> s) {
		cout << rank++ << ". " << n << " — " << s << endl;
	}
	in.close();

	cout << "\nPress any key to return...";
	_getch();
}

// Save the current gamew
void savegame() {
	ofstream out("savegame.txt");
	out << name << endl;
	out << score << endl;
	out << (time(0) - starttime) << endl;
	out << ishardmode << endl;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++)
			out << board[i][j];
		out << endl;
	}
	out.close();

	cout << "Game saved\n";
	Sleep(800);
}

// Load saved game
bool loadgame() {
	ifstream in("savegame.txt");
	if (!in.is_open()) return false;

	long elapsed;
	in >> name >> score >> elapsed >> ishardmode;

	if (ishardmode) { rows = 10; cols = 10; candycount = 7; timelimit = 40; }
	else { rows = 8; cols = 8; candycount = 5; timelimit = 60; }

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			in >> board[i][j];
	in.close();

	starttime = time(0) - elapsed;
	pausetime = 0;
	return true;
}

// Main menu
void mainmenu() {
	while (true) {
		system("cls");
		setcolor(15);

		cout << "****CANDY CRUSH GAME****\n\n";
		cout << "1. New Game\n";
		cout << "2. Load Game\n";
		cout << "3. High Scores\n";
		cout << "4. Instructions\n";
		cout << "5. Exit\n\n";
		cout << "Enter choice: ";

		char c;
		cin >> c;

		if (c == '1') {
			cout << "Enter Player Name: ";
			cin >> name;

			cout << "Difficulty:\n1. Easy\n2. Hard\nChoice: ";
			char d; cin >> d;
			ishardmode = (d == '2');

			if (ishardmode) { rows = 10; cols = 10; candycount = 7; timelimit = 40; }
			else { rows = 8; cols = 8; candycount = 5; timelimit = 60; }

			score = 0;
			gameloop();
		}
		else if (c == '2') {
			if (loadgame()) gameloop();
		}
		else if (c == '3') {
			showscore();
		}
		else if (c == '4') {
			system("cls");
			cout << "INSTRUCTIONS:\n";
			cout << "- Match 3 or more candies.\n";
			cout << "- L shaped matches give bonus.\n";
			cout << "- Enter Row Col, then use arrow keys.\n";
			cout << "- Watch the timer \n\n";
			cout << "Press any key...";
			_getch();
		}
		else if (c == '5') {
			exit(0);
		}
	}
}


int main() {
	srand((unsigned)time(0));
	SetConsoleTitle(TEXT("Candy Crush Project"));
	mainmenu();

	system("pause");
	return 0;
}
