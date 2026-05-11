#include "turn.h"
#include <iostream>
//나는 이 프로그래밍 과제를 다른 사람의 부적절한 도움 없이 완수하였습니다.
using namespace std;

int pcTurn(tile*& trunk, tile*& pc, tile*& usr, int& win)
{
	tile* recent = nullptr; //나중에 공개예정
	int rtvalue = 0;
	srand(time(NULL));
	cout << "컴퓨터의 차례입니다\n" << endl;

	rtvalue = getTile(trunk, pc, 3, recent);
	if (rtvalue == 10000) { //정상, no tile to get
		recent = pc;
		while (recent->visibility)
			recent = recent->next; //안보이는 가장왼쪽
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
		rtvalue = pcGuess(trunk, pc, usr, recent); //추리
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

int pcGuess(tile*& trunk, tile*& pc, tile*& usr, tile*& recent) {
	tile* position = nullptr;
	cout << "컴퓨터가 추리를 시작합니다" << endl;

	int usrcnt = countTile(usr, 0, 0);
	int pick = 0;

	while (true) {
		pick = rand() % usrcnt;
		position = usr;
		for (int i = 0; i < pick; i++) {
			if (position == nullptr)
				return 401; //exception, should not happen
			else
				position = position->next;
		}
		if (position == nullptr)
			return 402; //exception, should not happen
		else if (position->visibility == 0)  //안보이는것을 뽑을때까지 다시뽑기
			break;
	}

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
	srand(time(NULL));
	cout << "플레이어의 차례입니다\n" << endl;

	rtvalue = getTile(trunk, usr, 2, recent);
	if (rtvalue == 100) { //빈 것
		recent = usr;
		while (recent->visibility)
			recent = recent->next; //가장왼쪽 안보이는거
	}
	else if (rtvalue) return rtvalue; //exception, should not happen
	else
		cout << "가져온 타일은 " << (recent->white ? "백색" : "검은색") << "타일" << recent->number << "번입니다.\n" << endl;

	rtvalue = prtTile(trunk, pc, usr);
	if (rtvalue) return rtvalue;
	win = 0;
	do {
		rtvalue = usrGuess(trunk, pc, usr, recent); //추측
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

int usrGuess(tile*& trunk, tile*& pc, tile*& usr, tile*& recent) {
	tile* position = nullptr;
	cout << "플레이어가 추리를 시작합니다.\n" << "추리하고 싶은 위치와 숫자를 입력해주세요." << endl;

	int guess; //추측대상
	int guessval; //추측값

	cin >> guess >> guessval;
	guess--; //시작을 0으로,....

	position = pc;
	for (int i = 0; i < guess; i++) {
		if (position == nullptr)
			return 603; //invalid pc deck, should not happen
		else
			position = position->next;
	}

	tile* obj = position;//추측대상

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