//========================================================================//
// Copyright (C) 2016 Evan Bowman                                         //
//                                                                        //
// This program is free software: you can redistribute it and/or modify   //
// it under the terms of the GNU General Public License as published by   //
// the Free Software Foundation, either version 3 of the License, or      //
// (at your option) any later version.                                    //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.  //
//========================================================================//

#include "tileController.hpp"
#include "mappingFunctions.hpp"

void floodFillspread(uint8_t screen[61][61], uint8_t x, uint8_t y, uint8_t prevC, uint8_t newC) {
	// Base cases
	if (x <= 2 || x >= 59 || y <= 2 || y >= 59) return;
	if (screen[x][y] != prevC) return;
	
	// Replace replace the old element with the desired new one
	screen[x][y] = newC;
	
	// Continue in four directions
	floodFillspread(screen, x + 1, y, prevC, newC);
	floodFillspread(screen, x - 1, y, prevC, newC);
	floodFillspread(screen, x, y + 1, prevC, newC);
	floodFillspread(screen, x, y - 1, prevC, newC);
}

void floodFill(uint8_t screen[61][61], uint8_t x, uint8_t y, uint8_t newC) {
	int prevC = screen[x][y];
	floodFillspread(screen, x, y, prevC, newC);
}

inline void cleanUp(uint8_t map[61][61]) {
	for (int i = 1; i < 60; i++) {
		for (int j = 1; j < 60; j++) {
			if (map[i][j] == 1 && map[i - 1][j] < 2 && map[i + 1][j] < 2 && map[i][j - 1] < 2 && map[i][j + 1] < 2) {
				map[i][j] = 0;
			}
		}
	}
}

inline void renumber(uint8_t map[61][61]) {
	for (int i = 0; i < 61; i++) {
		for (int j = 0; j < 61; j++) {
			if (map[i][j] == 1) {
				map[i][j] = 7;
			} else if (map[i][j] == 0) {
				map[i][j] = 1;
			}
		}
	}
}

inline void addEdges (uint8_t map[61][61]) {
	for (int i = 0; i < 61; i++) {
		for (int j = 0; j < 61; j++) {
			if (map[i][j] == 1) {
				if (map[i][j - 1] == 5) {
					if (map[i][j + 1] != 5) {
						map[i][j] = 2;
					} else {
						map[i][j] = 5;
					}
				} else if (map[i][j + 1] == 5) {
					map[i][j] = 6;
				}
			}
		}
	}
}

inline void addCenterTiles (uint8_t map[61][61]) {
	for (int i = 0; i < 61; i++) {
		for (int j = 0; j < 61; j++) {
			if ((map[i - 1][j] == 5 || map[i - 1][j] == 3 || map[i - 1][j] == 4) && (map[i + 1][j] == 5 || map[i + 1][j] == 3 || map[i + 1][j] == 4) && (map[i][j - 1] == 5 || map[i][j - 1] == 3 || map[i][j - 1] == 4) && (map[i][j + 1] == 5 || map[i][j + 1] == 3 || map[i][j + 1] == 4)) {
				if ((std::abs(static_cast<int>(globalRNG())) % 12) > 2) {
					map[i][j] = 3;
				} else {
					map[i][j] = 4;
				}
			}
		}
	}
}

void condense (uint8_t map[61][61], uint8_t maptemp[61][61], char rep) {
	int count = 0;
	// Heh it's not copy-paste it's *unrolled*
	for (int i = 2; i < 59; i++) {
		for (int j = 2; j < 59; j++) {
			count = 0;
			if (map[i - 1][j - 1] == 1) {
				count += 1;
			}
			if (map[i + 1][j - 1] == 1) {
				count += 1;
			}
			if (map[i - 1][j + 1] == 1) {
				count += 1;
			}
			if (map[i + 1][j + 1] == 1) {
				count += 1;
			}
			if (map[i - 1][j] == 1) {
				count += 1;
			}
			if (map[i + 1][j] == 1) {
				count += 1;
			}
			if (map[i][j - 1] == 1) {
				count += 1;
			}
			if (map[i][j + 1] == 1) {
				count += 1;
			}
			if (map[i][j]) {
				if (count < 2) {
					maptemp[i][j] = 0;
				} else {
					maptemp[i][j] = 1;
				}
			} else {
				if (count > 5) {
					maptemp[i][j] = 1;
				} else {
					maptemp[i][j] = 0;
				}
			}
		}
	}
	
	for (int i = 2; i < 59; i++) {
		for (int j = 2; j < 59; j++) {
			map[i][j] = maptemp[i][j];
		}
	}
	
	if (rep > 0) {
		condense(map, maptemp, rep - 1);
	}
}

int initMapOverlay(uint8_t map[61][61]) {
	uint8_t maptemp[61][61];
	memset(map, 0, sizeof(map[0][0]) * std::pow(61, 2));
	memset(map, 0, sizeof(map[0][0]) * std::pow(61, 2));
	for (int i = 5; i < 54; i++) {
		for (int j = 5; j < 54; j++) {
			map[i][j] = (std::abs(static_cast<int>(globalRNG())) % 2);
		}
	}
	condense(map, maptemp, 3);
	uint8_t xindex;
	uint8_t yindex;
	do {
		xindex = std::abs(static_cast<int>(globalRNG())) % 61;
		yindex = std::abs(static_cast<int>(globalRNG())) % 61;
	}
	while (map[xindex][yindex] != 1);
	floodFill(map, xindex, yindex, 5);
	int count = 0;
	for (int i = 0; i < 59; i++) {
		for (int j = 0; j < 59; j++) {
			if (map[i][j] == 5) {
				count += 1;
			}
		}
	}
	return count;
}

void combine(uint8_t map[61][61], uint8_t overlay[61][61]) {
	for (int i = 0; i < 61; i++) {
		for (int j = 0; j < 61; j++) {
			if ((overlay[i][j] == 5 && map[i][j] != 0) && (overlay[i][j] == 5 && map[i][j] != 1)) {
				if (map[i][j] == 2) {
					map[i][j] = 9;
				}
				else if (map[i][j] == 6) {
					map[i][j] = 10;
				}
				else if (map[i][j] == 5) {
					map[i][j] = 8;
				}
				else {
					map[i][j] = 11;
				}
			}
		}
	}
}

void cleanEdgesPostCombine(uint8_t map[61][61]) {
	for (int i = 1; i < 60; i++) {
		for (int j = 1; j < 60; j++) {
			if (map[i][j] == 9 && map[i][j - 1] != 8) {
				map[i][j] = 2;
			}
			else if (map[i][j] == 10 && map[i][j + 1] != 8) {
				map[i][j] = 6;
			}
		}
	}
}

int generateMap(uint8_t map[61][61]) {
	uint8_t maptemp[61][61];
	memset(map, 0, sizeof(map[0][0]) * std::pow(61, 2));
	memset(maptemp, 0, sizeof(maptemp[0][0]) * std::pow(61, 2));
	for (int i = 16; i < 44; i++) {
		for (int j = 16; j < 44; j++) {
			map[i][j] = (std::abs(static_cast<int>(globalRNG())) % 2);
		}
	}
	condense(map, maptemp, 1);
	renumber(map);
	uint8_t xindex;
	uint8_t yindex;
	do {
		xindex = std::abs(static_cast<int>(globalRNG())) % 61;
		yindex = std::abs(static_cast<int>(globalRNG())) % 61;
	}
	while (map[xindex][yindex] != 7);
	floodFill(map, xindex, yindex, 5);
	for (int i = 2; i < 59; i++) {
		for (int j = 2; j < 59; j++) {
			if (map[i][j] == 7) {
				map[i][j] = 1;
			}
		}
	}
	addEdges(map);	//Adds the top and bottom edges for the platforms
	cleanUp(map);
	addCenterTiles(map);
	int count = 0;
	for (int i = 0; i < 59; i++) {
		for (int j = 0; j < 59; j++) {
			if (map[i][j] == 3 || map[i][j] == 4) {
				count += 1;
			}
			if (map[i + 1][j] == 5 && map[i - 1][j] == 5 && map[i][j + 1] == 5 && map[i][j - 1] == 5) {
				map[i][j] = 5;
			}
		}
	}	
	uint8_t mapOverlay[61][61];
	int count2;
	do {
		count2 = initMapOverlay(mapOverlay);
	} while (count2 < 300);
			
	combine(map, mapOverlay);
	cleanEdgesPostCombine(map);
	return count;
}
