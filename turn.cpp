#include "turn.h"
#include <iostream>
#include <limits>
//나는 이 프로그래밍 과제를 다른 사람의 부적절한 도움 없이 완수하였습니다.
using namespace std;

namespace {
constexpr int kNoTileAvailable = 10000;

void clearInputLine()
{
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
}

int pcTurn(tile*& trunk, tile*& pc, tile*& usr, int& win)
{
	tile* recent = nullptr; //나중에 공개예정
	int rtvalue = 0;
	cout << "컴퓨터의 차례입니다\n" << endl;

	rtvalue = getTile(trunk, pc, 3, recent);
	if (rtvalue == kNoTileAvailable) { //정상, no tile to get
		recent = getFirstHiddenTile(pc);
		if (recent == nullptr)
			return 301;
	}
	else if (rtvalue) return rtvalue; //비정상 rtvalue
	else {
		cout << "컴퓨터가 랜덤 타일을 고르고 있습니다..." << endl;
		cout << "컴퓨터가 " << (recent->white ? "백색" : "검은색") << "타일을 골랐습니다.\n" << endl;
	}
	rtvalue = prtTile(trunk, pc, usr);
	if (rtvalue) return rtvalue;
	win = 0; //승부x
	do {
		rtvalue = pcGuess(trunk, usr, recent); //추리
		prtTile(trunk, pc, usr);
		if (!countTile(pc, 2, 0)) {
			win = 2; //usr승
			break;
		}
		if (!countTile(usr, 2, 0)) {
			win = 1; //pc승
			break;
		}
	} while (rtvalue == -1); //rtvalue=-1 추리성공, 계속진행
	if (rtvalue != 0 && rtvalue != -1)
		return rtvalue;

	return 0;
}

int pcGuess(tile*& trunk, tile*& usr, tile*& recent) {
	tile* position = nullptr;
	cout << "컴퓨터가 추리를 시작합니다" << endl;

	int hiddenCount = countTile(usr, 2, 0);
	int pick = 0;

	if (hiddenCount <= 0)
		return 403;

	pick = rand() % hiddenCount;
	int hiddenIndex = 0;
	for (position = usr; position != nullptr; position = position->next) {
		if (!position->visibility) {
			if (hiddenIndex == pick) {
				break;
			}
			hiddenIndex++;
		}
	}

	if (position == nullptr)
		return 402; //exception, should not happen

	tile* obj = position; //추측대상
	if (obj == nullptr)
		return 407; //exception, should not happen

	bool iswhite = obj->white; //대상 색 기억

	int guesslist[24] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }; //추측해도되는 숫자
	int j = 0;
	for (position = trunk; position != nullptr; position = position->next) {
		if (position->white == iswhite)
			guesslist[j++] = position->number;
	} //안보이는 iswhite색

	for (position = usr; position != nullptr; position = position->next) {
		if (position->white == iswhite && position->visibility == 0)
			guesslist[j++] = position->number;
	} //안보이는 iswhite색

	if (j == 0)
		return 404;

	int guessnum = rand() % j; //숫자 선택
	cout << "컴퓨터가 " << pick + 1 << "번째 위치의 숫자가 " <<
		guesslist[guessnum] << "라고 추리합니다." << endl;

	if (guesslist[guessnum] == obj->number) {
		cout << "정답입니다!" << endl;
		cout << "플레이어의 " << (obj->white ? "백색 타일 " : "흑색 타일 ") <<
			obj->number << "번이 공개됩니다.\n" << endl;
		obj->visibility = 1;
		return -1; //정답
	}
	else {
		cout << "틀렸습니다!" << endl;
		cout << "컴퓨터의 " << (recent->white ? "백색 타일 " : "흑색 타일 ") <<
			recent->number << "번이 공개됩니다.\n" << endl;
		recent->visibility = 1;
		return 0; //오답
	}
}

int usrTurn(tile*& trunk, tile*& pc, tile*& usr, int& win)
{
	tile* recent = nullptr; //나중에 공개예정

	int rtvalue = 0;
	cout << "플레이어의 차례입니다\n" << endl;

	rtvalue = getTile(trunk, usr, 2, recent);
	if (rtvalue == kNoTileAvailable) { //빈 것
		recent = getFirstHiddenTile(usr);
		if (recent == nullptr)
			return 501;
	}
	else if (rtvalue) return rtvalue; //exception, should not happen
	else
		cout << "가져온 타일은 " << (recent->white ? "백색" : "검은색") << "타일" << recent->number << "번입니다.\n" << endl;

	rtvalue = prtTile(trunk, pc, usr);
	if (rtvalue) return rtvalue;
	win = 0;
	do {
		rtvalue = usrGuess(pc, recent); //추측
		prtTile(trunk, pc, usr);
		if (!countTile(pc, 2, 0)) {
			win = 2; //usr 승
			break;
		}
		if (!countTile(usr, 2, 0)) {
			win = 1; //pc승
			break;
		}
	} while (rtvalue == -1); //맞으면 다시추측

	if (rtvalue != 0 && rtvalue != -1)
		return rtvalue; //exception, should not happen

	return 0;
}

int usrGuess(tile*& pc, tile*& recent) {
	cout << "플레이어가 추리를 시작합니다.\n" << "추리하고 싶은 위치와 숫자를 입력해주세요." << endl;

	int guess; //추측대상
	int guessval; //추측값
	const int pcDeckCount = countTile(pc, 0, 0);
	tile* obj = nullptr;

	while (true) {
		if (!(cin >> guess >> guessval)) {
			clearInputLine();
			cout << "위치와 숫자를 정수로 다시 입력해주세요." << endl;
			continue;
		}

		if (guess < 1 || guess > pcDeckCount || guessval < 0 || guessval > 11) {
			cout << "위치는 1부터 " << pcDeckCount << " 사이, 숫자는 0부터 11 사이로 입력해주세요." << endl;
			continue;
		}

		obj = pc;
		for (int i = 1; i < guess; i++) {
			if (obj == nullptr)
				return 603; //invalid pc deck, should not happen
			obj = obj->next;
		}

		if (obj == nullptr)
			return 604;
		if (obj->visibility) {
			cout << "이미 공개된 타일입니다. 다른 위치를 선택해주세요." << endl;
			continue;
		}

		break;
	}

	guess--; //시작을 0으로,....

	cout << "플레이어가 " << guess + 1 << "번째 위치의 숫자가 " <<
		guessval << "라고 추리합니다." << endl;

	if (guessval == obj->number) {
		cout << "정답입니다!" << endl;
		cout << "컴퓨터의 " << (obj->white ? "백색 타일 " : "흑색 타일 ") <<
			obj->number << "번이 공개됩니다.\n" << endl;
		obj->visibility = 1;
		return -1; //정답
	}
	else {
		cout << "틀렸습니다!" << endl;
		cout << "플레이어의 " << (recent->white ? "백색 타일 " : "흑색 타일 ") <<
			recent->number << "번이 공개됩니다.\n" << endl;
		recent->visibility = 1;
		return 0; //오답
	}
}
