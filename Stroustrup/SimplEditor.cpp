/*
	Name:
	Copyright:
	Author:
	Date: 14/01/24 11:39
	Description:	Exercise X.18 in Stroustrup 4th.
		A very limited editor with five commands.
		These commands are 'Enter a line,' 'Delete a line,'
		'Undo last,' 'Scroll,' and 'Quit.' The editor prompts
		the user for the name of an existing file which, if exists,
		read into a vector of strings. Likewise, at the
		conclusion of the program the program saves the edited file
		to disk.
*/
#include <iostream>
#include <iomanip>

using std::ios;
using std::setw;
using std::setiosflags;
using std::setprecision;

#include <vector>
#include <string>
#include <fstream>
#include "windows.h"
using namespace std;

// Our newly defined 'Editor' class
class Editor {

		// Static controlling integer values.
		static const int MAXMNU;
		static const int XCOORD;
		static const int BEGIN_X;
		static const int BEGIN_Y;
		static const int MAXSCR;
		static const int MAXFIL;
		static const int PRMPT_X;
		static const int PRMPT_Y;
		static const int PRMPTY0;

		// Member data such as menu vector, working string vectors etc.
		vector<string> vecmenu{"Enter string", "Delete string",
			"Undo last operation", "Scroll", "Quit"};
		vector<string> inputvec{static_cast<vector<std::__cxx11::basic_string<char>>::size_type>(MAXFIL)},
		       savevec{static_cast<vector<std::__cxx11::basic_string<char>>::size_type>(MAXFIL)},
		       wrkvec{static_cast<vector<std::__cxx11::basic_string<char>>::size_type>(MAXFIL)};
		vector<string>::iterator x;
		ifstream inFile;
		ofstream outFile;
		string filename;
		int headerLine;
		enum mEntry { ADD=48, DEL, UNDO, SCROLL, QUIT };

		// Placeholer for string and location of last operation
		struct undo {
			int lineno;
			string linestr;
		} udStruct;

		/* Supporting member functions */

		/******** loadFile ***********************
		This member function loads an existing file if the name
		supplied as such by user exists in the working directory.
		It is called by the constructor at commencement of
		exection.
		Input/Output: None
		*****************************************/
		void loadFile() {
			if (inFile.fail()) return;
			string wstr;
			while (!inFile.eof()) {
				getline(inFile, wstr);
				inputvec.push_back(wstr);
			}
			wrkvec = inputvec;
			displayContent(wrkvec, 1);
			inFile.close();
		}

		/******** WriteDocument ************************
		This member function saves the working vector of strings
		to disk onto a file whose name if supplied by the
		user at program commencement.
		Input/Output: None
		***********************************************/
		void writeLines() {
			outFile.open(filename, ios::out);
			for (auto x: wrkvec) outFile << x << "\n";
		}

		/*********** displayContent ***************************
		This member function displays one screenful of contents of the working
		vector of strings.
		Paramaters in: a vector of strings and an integer signifying
			the beginning line number, i.e. the line at which to start the
			screen of lines.
		Return values: None.
		******************************************************/
		void displayContent(vector<string>& invec, int beg) {
			int lineno = beg, headerLine = 1, i;
			vector<string>::iterator x;

			// Loop through working line vector
			for (x = invec.begin() + beg - 1; x < invec.end(); x++, headerLine++) {
				if (lineno > MAXFIL) break;

				// Exit if end of page reached
				if (lineno > beg && (lineno - beg) % MAXSCR == 0) break;
				gotoxy(1, BEGIN_Y + (headerLine % MAXSCR == 0? MAXSCR: headerLine % MAXSCR));
				cout << setw(3) << right << lineno++ << " " << *x;
			}
		}

		/*********** displayMenu ********************************
		This function displays the menu to the user. It is called
		repeatedly throughout program execution.
		Parameter in: A boolean value indicating whether to clear
			the screen prior to display of menu.
		Return value: None.
		********************************************************/
		void displayMenu(bool clsc) {
			if (clsc) system("cls");
			gotoxy(PRMPT_X, PRMPT_Y - 1);
			int m;
			for (m = 0; m < MAXMNU; m++) {
				cout << setw(2) << m+1 << " - " << vecmenu[m] << " ";
			}
		}

		/*********** getLineno ************************************
		This member function propmts the user for a line number which it returns
		as an integer. It is called whenever an input line number is needed
		for the program to carry out its operation.
		Parameter in: None
		Return value: An integer representing the line number entered by
			the user.
		**********************************************************/
		int getLineno() {
			cout << "Enter line number: ";
			char c = 0;
			string mstr;
			int lineno = 0;
			for (;;) {

				// Keep iterating until a digit is found
				while (c != '\n') {
					cin.get(c);
					if (c >= 48 && c <= 57)
						mstr += c;
				}
				if (mstr.size() >= 1) lineno = stoi(mstr);
				if (mstr.size() < 1 || lineno > MAXFIL) {

					// Give message if exceeded the permissible number
					if (lineno > MAXFIL) {
						gotoxy(PRMPT_X, PRMPT_Y);
						cout << "Cannot exceed " << MAXFIL << "lines.";
					}
					c = 0;

					// re-deisplay menu and contents before prompt.
					displayMenu(true);
					displayContent(wrkvec, 1);
					gotoxy(PRMPT_X, PRMPT_Y);
					cout << "Enter line number: ";
					continue;
				}
				break;
			}
			return lineno;
		}

		/*********** refreshLayout ************************************
		This member function refreshes the screen layout. It is called
		to refresh the screen data before processing 'Add' or 'Del'
		operations.
		Parameter In: An int representing a line no. This serves as an
		indicator wether to prompt for a line or not. Following the
		line prompt, it refreshes the data and re-displays the menu.
		**************************************************************/
		int refreshLayout(int line) {
			if (line != 0) return line;
			gotoxy(PRMPT_X, PRMPT_Y);
			int lineno = getLineno();
			system("cls");
			if (lineno < headerLine || lineno >= headerLine + MAXSCR) headerLine = lineno;
			displayContent(wrkvec, headerLine);
			displayMenu(false);
			gotoxy(BEGIN_X, BEGIN_Y + (lineno == headerLine? 1: lineno - headerLine + 1));
			return lineno;
		}
		
		/********* gotoxy *************************************
		******************************************************/
		void gotoxy(int x, int y) {
			COORD coord;
			coord.X = x;
			coord.Y = y;
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		}

        /******** processAddDel ******************************
        *****************************************************/
		void processAddDel(char choice, int lineno) {
			if (choice - 1 != ADD && choice - 1 != DELETE) return;
			if (choice - 1 == ADD) {
				getline(cin, wrkvec[lineno-1]);
			} else {
				for (int j = 0; j < 80; j++) cout << " ";
				wrkvec[lineno-1] = "";
			}
		}

	public:
		Editor () {
			gotoxy(PRMPT_X, PRMPTY0);
			cout << "Enter input file name: ";
			cin >> filename;
			cin.ignore();
			inFile.open(filename, ios::in);
			if (inFile.fail()) {
				gotoxy(PRMPT_X, PRMPTY0);
				system("cls");
				gotoxy(PRMPT_X, PRMPTY0);
				cout << "File not found.";
				sleep(2);
			} else
				system("cls");
			headerLine = 1;
		}

		void run() {
			string wstr{}, swapstr;
			char choice;
			system("cls");
			loadFile();
			int i = 0, lineno = 1;
			for (;;) {
				displayMenu(false);
				cout << ": ";
				cin.get(choice);
				if (choice - 1 == QUIT) break;
				cin.ignore();
				
				// process undo, prepare backup and undo
				if (choice - 1 == UNDO) {
					swapstr = wrkvec[udStruct.lineno - 1];
					wrkvec[udStruct.lineno - 1] = udStruct.linestr;
				}
				lineno = refreshLayout(choice - 1 == ADD || choice - 1 == DELETE || choice - 1 == SCROLL? 0: lineno);
				
				// back up data for future undo
				if (choice - 1 >= ADD && choice - 1 <= UNDO) {
					udStruct.lineno = lineno;
					udStruct.linestr = choice - 1 == UNDO? swapstr: wrkvec[lineno - 1];
				}
				processAddDel(choice, lineno);
				system("cls");
				
				// update current beginning header line upon scroll
				if (choice - 1 == SCROLL) headerLine = lineno;
				displayContent(wrkvec, headerLine);
				i++;
			}
			writeLines();
		}
};

const int Editor::MAXMNU = 5;
const int Editor::XCOORD = 39;
const int Editor::BEGIN_X = 5;
const int Editor::BEGIN_Y = 0;
const int Editor::MAXFIL = 300;
const int Editor::PRMPT_X = 1;
const int Editor::PRMPTY0 = 28;
const int Editor::PRMPT_Y = 29;
const int Editor::MAXSCR = 26;

int main() {
	Editor myEditor{};
	myEditor.run();
	return 0;
}