#include "game.h"
#include "turn.h"
#include <iostream>
#include <string>
//나는 이 프로그래밍 과제를 다른 사람의 부적절한 도움 없이 완수하였습니다.
using namespace std;

int goGame() {
	int rtvalue = 0; //프로그램에 원치않는 동작시 특정 리턴값을 리턴하여 종료
	
	tile* trunk = genTile(); //게임시작시 타일 생성
	tile* pc = nullptr; 
	tile* usr = nullptr;

	rtvalue = initTile(trunk, pc, usr); //타일을 각각 4개씩 초기세팅
	if (rtvalue) {
		plushTile(trunk);
		plushTile(pc);
		plushTile(usr);  //비정상종료
		return rtvalue;
	}

	//	cout << trunk << endl << pc << endl << usr << endl;
	rtvalue = prtTile(trunk, pc, usr);
	if (rtvalue) {
		plushTile(trunk);
		plushTile(pc);
		plushTile(usr); //비정상종료
		return rtvalue;
	}
	int win = 0; // 0=continue play, 1=pc win, 2=usr win
	while (!win) {
		rtvalue = pcTurn(trunk, pc, usr, win);
		if (rtvalue) {
			plushTile(trunk);
			plushTile(pc);
			plushTile(usr);  //비정상종료
			return rtvalue;
		}
		if (win == 1) {
			cout << "컴퓨터의 승리입니다" << endl;
			break;
		}
		if (win == 2) {
			cout << "플레이어의 승리입니다" << endl;
			break;
		}
		rtvalue = usrTurn(trunk, pc, usr, win);
		if (rtvalue) {
			plushTile(trunk);
			plushTile(pc);
			plushTile(usr);  //비정상종료
			return rtvalue;
		}
		if (win == 1) {
			cout << "컴퓨터의 승리입니다" << endl;
			break;
		}
		if (win == 2) {
			cout << "플레이어의 승리입니다" << endl;
			break;
		}
	}
	prtTile(trunk, pc, usr);
	cout << "게임이 종료되었습니다. 메뉴로 돌아갑니다\n" << endl;

	plushTile(trunk);
	plushTile(pc);
	plushTile(usr); //타일들 동적할당 해제,정상종료
	return 0;
}

int countTile(const tile* head, int mode, bool data) {  
	int i;
	const tile* position = head;
	switch (mode) {
	case 0: //all, no condition
		for (i = 0; position != nullptr; position = position->next) {
			i++;
		}
		break;
	case 1: //color data
		for (i = 0; position != nullptr; position = position->next) {
			if (position->white == data)
				i++;
		}
		break;
	case 2: //visibility data
		for (i = 0; position != nullptr; position = position->next) {
			if (position->visibility == data)
				i++;
		}
		break;
	default:
		return 199;
	}
	return i;
} //mode 0=all,1=white, 2=visibility

tile* genTile() {
	int i;

	tile* trunk = new tile; //memory alloc

	tile* position = trunk; //linked list seek point

	for (i = 0; i < 24; i++) {
		position->number = i / 2; //0,0,1,1,2,2,3,3,4,4....
		position->white = i % 2; //0,1,0,1,0,1,0,1....
		position->visibility = 0;
		if (!i)
			position->prev = nullptr; //처음거
		if (i == 23)
			position->next = nullptr; //마지막거
		else
		{
			position->next = new tile;
			position->next->prev = position;
		}
		position = position->next;
	}
	return trunk;
}

int getTile(tile*& trunk, tile*& tgt, int control, tile*&recent) {  //control 0=black,1=white,2=usr choice,3=random color
	
	srand(time(NULL)); //랜덤타임
	int trunkcnt; //trunk number count
	int pick; //가져올타일
	bool color; //choice 저장소
	int b = 0, w = 0; //trunk number count by color
	tile* position = nullptr;

	trunkcnt = countTile(trunk, 0, 0);
	b = countTile(trunk, 1, 0);
	w = countTile(trunk, 1, 1);
	if (b + w != trunkcnt)
		return 170; //타일 갯수 무결성, should not happen

	if (trunk == nullptr)
		return 10000; //might happen,if trunk is empty

	if (control != 3) {
		if (b == 0 || w == 0) {
			pick = rand() % trunkcnt; //색상 선택권없음
		}
		else {
			if (control == 2) {
				cout << "가져오고 싶은 타일의 색을 고르세요. (흑:0 백:1):" << endl;
				cin >> color;
			}
			else
				color = control;
			while (true) {
				pick = rand() % trunkcnt;
				position = trunk;
				for (int i = 0; i < pick; i++) {
					if (position == nullptr)
						return 101; //exception, should not happen
					else
						position = position->next;
				}
				if (position == nullptr)
					return 102;
				else if (position->white == color)
					break; //원하는 색상이 나올 때까지 다시 뽑는다

			} 
		}
	}
	else {
		pick = rand() % trunkcnt; //control=3, no color choice
	}

	position = trunk;
	for (int i = 0; i < pick; i++) {
		if (position == nullptr) //exception, should not happen
			return 103;
		else
			position = position->next;
	}
	tile* obj = position; //가져올 타일 확정

	recent = obj; //recent 정보 전달

	if (obj == nullptr)
		return 104;  //exception, should not happen
	if (obj->prev != nullptr)
		obj->prev->next = obj->next;
	else
		trunk = obj->next;

	if (obj->next != nullptr)
		obj->next->prev = obj->prev; //바닥 덱 linked list 에서 obj 타일을 꺼낸다.

	obj->prev = nullptr;
	obj->next = nullptr; //obj 타일 꺼내진 상태

	if (tgt == nullptr) {
		tgt = obj; //가져오는 곳이 blank인 경우 그냥 삽입
	}
	else{ //linked list 적정위치에 삽입
		position = tgt;
		while (position != nullptr)
		{
			if (position->number > obj->number) { //숫자 작을시에, 바로 왼쪽에 삽입
				obj->prev = position->prev;
				obj->next = position;
				position->prev = obj;
				if (obj->prev != nullptr)
					obj->prev->next = obj;
				break;
			}
			else if (position->number == obj->number) { //숫자 같을경우 색비교
				if (position->white > obj->white) { //position 왼쪽에 삽입
					obj->prev = position->prev;
					obj->next = position;
					position->prev = obj;
					if (obj->prev != nullptr)
						obj->prev->next = obj;
					break;
				}
				else if (position->white < obj->white) { //position 오른쪽에 삽입
					obj->prev = position;
					obj->next = position->next;
					position->next = obj;
					if (obj->next != nullptr)
						obj->next->prev = obj;
					break;
				}
				else
					return 201; //숫자같고 색같고, exception, should not happen
			}
			else if (position->number < obj->number) { //다음 칸으로 넘어가서 비교준비
				if (position->next != nullptr) {
					position = position->next;
					continue;
				}
				else { //position이 마지막 칸일경우 position 오른쪽에 삽입
					obj->next = nullptr;
					obj->prev = position;
					position->next = obj;
					break;
				}
			}
			else
				return 202; //exception, should not happen
		}
		while (tgt->prev != nullptr)
			tgt = tgt->prev; //tgt가 linked list의 맨 앞주소 참조가 아닐경우 맨 앞까지 당김
	}
	return 0;
}

int initTile(tile*& trunk, tile*& pc, tile*& usr) {
	int b = 0;
	int w = 0;
	int rtvalue;
	while (b + w != 4) {
		cout << "가져올 타일의 색 별 개수를 정하세요. (흑 백 순서, 흑+백=4):" << endl;
		cin >> b >> w;
	}
	tile* recent; //용도 없음
	for (int i = 0; i < b; i++) { //유저흑색
		rtvalue = getTile(trunk, usr, 0, recent);
		if (rtvalue)
			return rtvalue;
	}
	for (int i = 0; i < w; i++) { //유저백색
		rtvalue = getTile(trunk, usr, 1, recent);
		if (rtvalue)
			return rtvalue;
	}
	for (int i = 0; i < 4; i++) { //컴퓨터
		rtvalue = getTile(trunk, pc, 3, recent);
		if (rtvalue)
			return rtvalue;
	}
	return 0;
}

void plushTile(tile* head)
{
	tile* position = head;
	for (position = head; position != nullptr; ) {
		tile* nowpos = position;
		position = position->next;
		delete nowpos;
	}
}

int prtTile(tile* trunk, tile* pc, tile* usr) {
	int b = 0, w = 0;
	tile* position = nullptr;
	b = countTile(trunk, 1, 0);
	w = countTile(trunk, 1, 1);
	cout << "=====================================" << endl;
	cout << "남은 타일 || □:" << b << "  ■:" << w << endl;
	cout << "=====================================\n" << endl;

	int pdeck = 0, udeck = 0; // 덱 개수 카운트
	cout << "==컴퓨터 덱==" << endl;

	for (position = pc; position != nullptr; position = position->next) {
		cout << (position->visibility ? "<" : "[") << pdeck + 1 <<
			(position->white ? ": ■" : ": □") <<
			(position->visibility ? to_string(position->number) : "??") <<
			(position->visibility ? "> " : "] ");
		pdeck++;
	}

	/* 투시 핵
	for (position = pc; position != nullptr; position = position->next) {
		cout << (position->visibility ? "<" : "[") << pdeck + 1 <<
			(position->white ? ": ■" : ": □") <<
			position->number <<
			(position->visibility ? "> " : "] ");
		pdeck++;
	}
	*/
	cout << endl << endl;
	if (pdeck != countTile(pc, 0, 0))
		return 171; //exception, should not happen
	
	position = nullptr;

	cout << "==플레이어 덱==" << endl;
	for (position = usr; position != nullptr; position = position->next) {
		cout << (position->visibility ? "<" : "[") << udeck + 1 <<
			(position->white ? ": ■" : ": □") <<
			position->number <<
			(position->visibility ? ">" : "]") << "  ";
		udeck++;
	}
	cout << endl << endl;
	if (udeck != countTile(usr, 0, 0))
		return 172; //exception, should not happen

	return 0;
}
