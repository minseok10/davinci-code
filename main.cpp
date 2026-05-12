#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>

#include "game.h"
#include "menus.h"

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

void printEnginePreview()
{
	DavinciGame game(static_cast<unsigned int>(time(nullptr)));
	game.start(2, 2);

	cout << "게임 엔진이 초기화되었습니다.\n";
	cout << "웹 앱에서 재사용할 수 있도록 콘솔 입출력과 게임 로직을 분리했습니다.\n";
	cout << "현재 콘솔 플레이 루프는 제거되었고, 엔진 API는 game.h의 DavinciGame을 사용합니다.\n\n";
	cout << "남은 타일: " << game.stock().size() << "\n";
	cout << "컴퓨터 타일 수: " << game.hand(Player::Computer).size() << "\n";
	cout << "플레이어 타일 수: " << game.hand(Player::User).size() << "\n\n";
}
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	while (true) {
		prtMenu();
		const int choice = readMenuChoice();
		switch (choice) {
		case 1:
			prtManual();
			break;
		case 2:
			printEnginePreview();
			break;
		case 3:
			return 0;
		default:
			cout << "다시 선택하세요\n" << endl;
			break;
		}
	}
}
