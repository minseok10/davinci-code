#pragma once
struct tile {
	bool visibility;
	bool white;
	int number;
	tile* next;
	tile* prev;
}; //linked list struct
int goGame();
int countTile(const tile* head, int mode, bool data); //mode 0=all,1=white, 2=visibility
tile* genTile();
int getTile(tile*& trunk, tile*& tgt, int control, tile*& recent);  //control 0=black,1=white,2=usr choice,3=random color
int initTile(tile*& trunk, tile*& pc, tile*& usr);
void plushTile(tile* head);
int prtTile(tile*, tile*, tile*);