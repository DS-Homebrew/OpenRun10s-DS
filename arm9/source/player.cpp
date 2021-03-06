#include <nds.h>
#include "gl2d.h"
#include "sound.h"

#include "graphics/fontHandler.h"
#include "graphics/graphics.h"

#include "spr_player.h"

extern u8 mapData[];

extern int cameraXpos;
//extern int cameraYpos;

static bool destroyed = false;
static bool playerJump = false;
static bool allowPlayerJump = false;
static bool jumpFallFrame = true;
static bool playerDirection = true;
static bool moveDirection = true;
static bool animateLegs = false;
static bool moveMore = false;
static int yMoveDelay = 0;
static int hudTexID, playerTexID;
static glImage hudImage[64 * 32];
static glImage playerImage[(128 / 16) * (64 / 16)];

int playerX, playerY, playerStartX, playerStartY;

static int playerYmoveSpeed=1, legAniFrame, legAniDelay;

void initPlayers(void) {
	destroyed = false;
	playerJump = false;
	allowPlayerJump = false;
	jumpFallFrame = true;
	playerDirection = true;
	moveDirection = true;
	animateLegs = false;
	moveMore = false;
	yMoveDelay = false;
}

void setPlayerPosition(int x, int y) {
	playerStartX = x;
	playerStartY = y;
	playerX = playerStartX;
	playerY = playerStartY;
}

void resetPlayerPosition(void) {
	playerX = playerStartX;
	playerY = playerStartY;
}

void playerGraphicLoad(void) {
	playerTexID = glLoadTileSet(playerImage, // pointer to glImage array
							16, // sprite width
							16, // sprite height
							128, // bitmap width
							64, // bitmap height
							GL_RGB16, // texture type for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
							TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
							TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
							5, // Length of the palette to use (5 colors)
							(u16*) spr_playerPal, // Load our 16 color tiles palette
							(u8*) spr_playerBitmap // image data generated by GRIT
							);
}

void playerLoop(int pressed, int held) {
	extern bool isSolidTile(u8 tile);

	if (destroyed) return;

	if (playerY > 184) {
		// Make the player fall
		if (playerYmoveSpeed == 2) {
			jumpFallFrame = true;
		}
		yMoveDelay++;
		playerY += playerYmoveSpeed;
		if (yMoveDelay==4) {
			playerYmoveSpeed++;
			yMoveDelay = 0;
		}
		if (playerYmoveSpeed > 8) playerYmoveSpeed = 8;
	} else if (playerJump) {
		allowPlayerJump = false;
		jumpFallFrame = true;
		if ((isSolidTile(mapData[((playerY/12)*64)+((playerX-6)/12)]))
		 || (isSolidTile(mapData[((playerY/12)*64)+((playerX+6)/12)]))
		 || playerY<0) {
			// Touched solid tile
			playerY++;
			playerJump = false;
			playerYmoveSpeed = 1;
		}
		if ((mapData[((playerY/12)*64)+((playerX-6)/12)] == 23)
		 || (mapData[((playerY/12)*64)+((playerX+6)/12)] == 23)) {
			// Make the player jump
			yMoveDelay++;
			playerY -= playerYmoveSpeed;
			if (yMoveDelay==4) {
				playerYmoveSpeed--;
				yMoveDelay = 0;
			}
			if (playerYmoveSpeed < 1) {
				playerJump = false;
				playerYmoveSpeed = 1;
			}
		}
	} else {
		if ((mapData[(((playerY+23)/12)*64)+((playerX-6)/12)] == 23)
		 || (mapData[(((playerY+23)/12)*64)+((playerX+6)/12)] == 23)) {
			// Make the player fall
			if (playerYmoveSpeed == 2) {
				jumpFallFrame = true;
			}
			yMoveDelay++;
			playerY += playerYmoveSpeed;
			if (yMoveDelay==4) {
				playerYmoveSpeed++;
				yMoveDelay = 0;
			}
			if (playerYmoveSpeed > 8) playerYmoveSpeed = 8;
		}
		if ((isSolidTile(mapData[(((playerY+23)/12)*64)+((playerX-6)/12)]))
		 || (isSolidTile(mapData[(((playerY+23)/12)*64)+((playerX+6)/12)]))) {
			// On solid tile
			playerY -= (playerY % 12);
			playerYmoveSpeed = 1;
			allowPlayerJump = true;
			jumpFallFrame = false;
			yMoveDelay = 0;
		}
	}
	if (playerY > 192*2) {
		// Player is destroyed
		destroyed = true;
		snd().loadStream("nitro:/GameOver_BGM.raw", 16364, false);
		snd().beginStream();
	}

	if (held & KEY_LEFT) {
		moveMore = !moveMore;
		moveDirection = false;
		playerDirection = false;
		playerX -= 1+moveMore;
	} else if (held & KEY_RIGHT) {
		moveMore = !moveMore;
		moveDirection = true;
		playerDirection = true;
		playerX += 1+moveMore;
	}
	
	if ((pressed & KEY_A) && allowPlayerJump) {
		playerJump = true;
		playerYmoveSpeed = 5;
		snd().playJump();
	}
	
	if (moveDirection) {
		if (isSolidTile(mapData[(((playerY+23)/12)*64)+((playerX+6)/12)])) {
			playerX -= 1+moveMore;
		}
	} else {
		if (isSolidTile(mapData[(((playerY+23)/12)*64)+((playerX-6)/12)])) {
			playerX += 1+moveMore;
		}
	}

	animateLegs = ((held & KEY_LEFT) || (held & KEY_RIGHT));
}

void renderPlayer(bool top) {
	if (destroyed) return;

	glSprite((playerX-8)-cameraXpos, (playerY+8)-(top ? 0 : 192), playerDirection ? GL_FLIP_NONE : GL_FLIP_H, &playerImage[animateLegs ? legAniFrame : 1]);
	glColor(RGB15(31, 31, 31));

	if (!top) return;

	if (animateLegs) {
		legAniDelay++;
		if (legAniDelay==2) {
			legAniFrame++;
			if (legAniFrame > 7) {
				legAniFrame = 2;
			}
			legAniDelay = 0;
		}
	} else {
		legAniFrame = 2;
		legAniDelay = 1;
	}
}
