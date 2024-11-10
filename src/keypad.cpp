#include "../include/keypad.h"


// // KEYPAD
// 1 	2 	3 	4
// Q 	W 	E 	R
// A 	S 	D 	F
// Z 	X 	C 	V
// 0x0, 0x1, 0x2, 0x3,
// 0x4, 0x5, 0x6, 0x7,
// 0x8, 0x9, 0xA, 0xB,
// 0xC, 0xD, 0xE, 0xF,
bool checkKeypress(uint8_t key_idx) {
	switch (key_idx) {
		case 0x0: {
			return IsKeyPressed(KEY_ONE);
		}
		case 0x1: {
			return IsKeyPressed(KEY_TWO);
		}
		case 0x2: {
			return IsKeyPressed(KEY_THREE);
		}
		case 0x3: {
			return IsKeyPressed(KEY_FOUR);
		}
		case 0x4: {
			return IsKeyPressed(KEY_Q);
		}
		case 0x5: {
			return IsKeyPressed(KEY_W);
		}
		case 0x6: {
			return IsKeyPressed(KEY_E);
		}
		case 0x7: {
			return IsKeyPressed(KEY_R);
		}
		case 0x8: {
			return IsKeyPressed(KEY_A);
		}
		case 0x9: {
			return IsKeyPressed(KEY_S);
		}
		case 0xA: {
			return IsKeyPressed(KEY_D);
		}
		case 0xB: {
			return IsKeyPressed(KEY_F);
		}
		case 0xC: {
			return IsKeyPressed(KEY_Z);
		}
		case 0xD: {
			return IsKeyPressed(KEY_X);
		}
		case 0xE: {
			return IsKeyPressed(KEY_C);
		}
		case 0xF: {
			return IsKeyPressed(KEY_V);
		}
		default: //WARN: should be unreachable
			return false;
	}
}
