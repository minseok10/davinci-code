#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include "menus.h"
#include "game.h"
//나는 이 프로그래밍 과제를 다른 사람의 부적절한 도움 없이 완수하였습니다.
using namespace std;

namespace {
int readMenuChoice()
{
	int choice = 0;
	while (true) {
		if (cin >> choice) {
			return choice;
		}

		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "숫자를 입력하세요.\n" << endl;
	}
}
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	int rtvalue = 0; //프로그램에 원치않는 동작시 리턴값을 저장한다

	while (true) {
		prtMenu();
		int choice = readMenuChoice();
		switch (choice) {
		case 1:
			prtManual(); 
			break;
		case 2:
			rtvalue = goGame();
			if (rtvalue) return rtvalue; //프로그램에 원치않는 동작시 특정 리턴값을 리턴하여 종료
			break;
		case 3:
			return rtvalue;
		default:
			cout << "다시 선택하세요\n" << endl;
			break;
		}
	}
	return 0;
}
