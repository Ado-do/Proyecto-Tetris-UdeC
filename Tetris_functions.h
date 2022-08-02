#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#include "Tetris_static.h"

//! Declarar funciones

//* Inicializar
bool initSDL(SDL_Window **ptrWindow, SDL_Renderer **ptrRenderer);
void initPlayfield(Playfield *playfield);
Text* initText(const char *str, Font *font, const SDL_Color color, const int x, const int y, const float size);

//* Cargar texturas
void loadTetrominoesTexture(SDL_Renderer *renderer);
void loadBackgroundsTexture(SDL_Renderer *renderer, SDL_Texture **backgrounds);
void loadGameOverTexture(SDL_Renderer *renderer, SDL_Texture **gameover);
void loadTextTexture(SDL_Renderer *renderer, Text *text);

//* Funciones tablero
void printPlayfield(Playfield *playfield);
void updatePlayfield(Playfield *playfield, Tetromino *curr, Tetromino *next);
void newTetromino(Tetromino *curr, Tetromino *next);

//* Funciones game loop
void tetrisLoop(Playfield *playfield, Tetromino *curr, Tetromino *next, Tetromino *holder);

void gameInput(Tetromino *curr, Tetromino *next, Playfield *playfield);
void gameUpdate(Playfield *playfield, Tetromino *curr, Tetromino *next, Tetromino *holder);
bool gameOverCheck(Playfield *playfield, Tetromino *curr);

//* Loyica de tetris
int checkLineState(Playfield *playfield, Uint8 row);
int deleteLines(Playfield *playfield);
bool collision(Playfield *playfield, Tetromino *curr);
void countStackHeight(Playfield *playfield);
void wallKickFRUNA(Playfield *playfield, Tetromino *curr, Sint8 rotation);
void rotateTetromino(Tetromino *tetro, const Sint8 sense);
void hardDropTetromino(Playfield *playfield, Tetromino *curr, Tetromino *next);
void softDropTetromino(Playfield *playfield, Tetromino *curr, Tetromino *next);
bool checkFallTime(Uint64 countFrames);
void calculateScore(int linesCleared);
void calculateDifficulty(int level) ;

//* Renderizado
void renderText(SDL_Renderer *renderer, Text *text);
void renderBackground(SDL_Renderer *renderer, SDL_Texture *background, SDL_Texture *gameboardInt);
void renderGameOver(SDL_Renderer *renderer, SDL_Texture **gameOverTextures, Text *textRecord);
void renderNextHold(SDL_Renderer *renderer, Tetromino *next, Tetromino *holder);
void renderTetromino(SDL_Renderer *renderer, Tetromino *tetro);
void renderGhostTetromino(SDL_Renderer *renderer, Playfield *playfield, Tetromino *curr);
void renderPlayfield(SDL_Renderer *renderer, Playfield *playfield);

//* Liberar y cerrar
void freeText(Text *text);
void QuitSDL(SDL_Window *window, SDL_Renderer *renderer);


//! Definir funciones principales

void tetrisLoop(Playfield *playfield, Tetromino *curr, Tetromino *next, Tetromino *holder) {
//! INPUT ======================================================================================
	gameInput(curr, next, playfield);

//! LOGICA Y CAMBIOS ======================================================================================

	if(checkFallTime(countFrames)) fall = true;

	gameUpdate(playfield, curr, next, holder);

	if (droped) {
		updatePlayfield(playfield, curr, next);
		droped = false;
	}

	if (clearedLines > 0) {
		calculateScore(clearedLines);
		if (levelUp) {
			calculateDifficulty(level);
		}
	}
	
//! RENDER ======================================================================================
	if (clearedLines > 0) {
		// Actualizar texto lineas 
		snprintf(textLines->string + 7, 4,"%d", totalLines);
		loadTextTexture(renderer, textLines);
		// Actualizar texto score
		snprintf(textScore->string + 7, 5,"%lld", score);
		loadTextTexture(renderer, textScore); 
		clearedLines = 0;
	}

	if (levelUp) {
		// Actualizar texto nivel
		snprintf(textLevel->string + 7, 2,"%d", level);
		loadTextTexture(renderer, textLevel);
		levelUp = false;
	}

	// Cargar texto de FPS actuales
	if (countFrames > 0 && countFrames % 20) {
		snprintf(textFPS->string + 5, 6,"%.1f", FPS);
		loadTextTexture(renderer, textFPS); // Cargar textura de string con cantidad de FPS
	}

	SDL_RenderClear(renderer);
	// Fondo ================================================================
	renderBackground(renderer, backgrounds[currBackground], gameboardInt);
	renderNextHold(renderer, next, holder);

	// Texto ================================================================
	if (countFrames > 0) renderText(renderer, textFPS);

	renderText(renderer, textScore);
	renderText(renderer, textLines);
	renderText(renderer, textLevel);
	renderText(renderer, textIntruc);

	// Playfield ============================================================
	renderPlayfield(renderer, playfield);
	renderGhostTetromino(renderer, playfield, curr);
	renderTetromino(renderer, curr);

	// Interfaz superpuesta =================================================
	SDL_RenderCopy(renderer, gameboardExt, NULL, NULL);
	SDL_RenderPresent(renderer);

}


// Funcion que recibe el input del juego //TODO: Cambiar running -> gameover
void gameInput(Tetromino *curr, Tetromino *next, Playfield *playfield) {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		if(e.type == SDL_QUIT) {
			running = false;
			break;
		}
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
				case SDLK_UP:
					if (!e.key.repeat) rotation = COUNTER_CLOCKWISE;
					break;
				case SDLK_DOWN:		softD = true; break;
				case SDLK_RIGHT:	right = true; break;
				case SDLK_LEFT:		left = true; break;
				case SDLK_z:
					if (!e.key.repeat) rotation = COUNTER_CLOCKWISE;
					break;
				case SDLK_x:
					if (!e.key.repeat) rotation = CLOCKWISE;
					break;
				case SDLK_a:
					if (!e.key.repeat) rotation = DOUBLE_CLOCKWISE;
					break;
				case SDLK_c: hold = true; break;
				case SDLK_SPACE:
					if (!e.key.repeat) hardD = true;
					break;
				case SDLK_r:
					if (!e.key.repeat) restart = true;
					break;
				case SDLK_ESCAPE:	running = false; break;
									// gameOver = true; break;
				default: break;
			}
		}
	}
}

// Game updater (revisa coliones antes de mover) //TODO: ORGANIZAR BIEN ***************************
void gameUpdate(Playfield *playfield, Tetromino *curr, Tetromino *next, Tetromino *holder) {
	if (rotation) {
		rotateTetromino(curr, rotation);
		if (collision(playfield, curr)) {
			switch (rotation) {
				case CLOCKWISE: 		rotateTetromino(curr, COUNTER_CLOCKWISE); break;
				case COUNTER_CLOCKWISE: rotateTetromino(curr, CLOCKWISE); break;
				case DOUBLE_CLOCKWISE: 	rotateTetromino(curr, DOUBLE_CLOCKWISE); break;
			}
		}
		rotation = false;
	}
	if (hardD) {
		if (holded) holded = false;
		hardDropTetromino(playfield, curr, next);
		hardD = false;
	}
	if (softD) {
		//TODO: LOCK DELAY (MEDIO SEGUNDO EN CONTACTO CON SUELO ANTES DE CAER) **************************
		softDropTetromino(playfield, curr, next);
		softD = false;
	}
	if (hold) {
		if (!firstHold) {
			*holder = tetrominoes[curr->nShape];
			newTetromino(curr, next);
			firstHold = true;
			holded = true;
		} else {
			if (!holded) {
				int aux = curr->nShape;
				*curr = *holder;
				*holder = tetrominoes[aux];
				holded = true;
			}
		}
		hold = false;
	}
	if (right) {
		curr->x++;
		if (collision(playfield, curr)) curr->x--;
		right = false;
	}
	if (left) {
		curr->x--;
		if (collision(playfield, curr)) curr->x++;
		left = false;
	}
	//TODO: CREAR FUNCION ESPECIFICA PARA FALL
	if (fall) {
		curr->y++;
		if (collision(playfield, curr)) {
			curr->y--;
			if (!lock_delayFRUNA) {
				lock_timer = countFrames + 30;
				lock_delayFRUNA = true;
			} else {
				if (countFrames > lock_timer) {
					lock_delayFRUNA = false;
					droped = true;
				}
			}
		}
		fall = false;
	}
	if (restart) {
		if (currBackground < 3) currBackground++;
		else currBackground = 0;
		initPlayfield(playfield);
		restart = false;
	}
}


// Funcion que inicializa todo SDL y demas librerias usadas
bool initSDL(SDL_Window **ptrWindow, SDL_Renderer **ptrRenderer) { 
	// Inicializar toda la biblioteca de SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Error initializing SDL2: %s\n", SDL_GetError());
		return 0;
	}
	// Inicializar SDL_TTF
	if (TTF_Init() == -1) {
		printf("Error initializing SDL_TTF: %s\n", TTF_GetError());
		return 0;
	}
	// Inicializar SDL_image
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		printf("Error initializing SDL_Image: %s\n", IMG_GetError());
		return 0;
	}
	// Calidad de escalado
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best") == SDL_FALSE) {
		printf("Error assigning scaling hint: %s\n", SDL_GetError());
	}
	// Crear ventana
	*ptrWindow = SDL_CreateWindow("Intento de tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (*ptrWindow == NULL) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 0;
	}
	SDL_Surface* icon = IMG_Load("assets/udec_icon.webp");
	if (icon == NULL) {
		printf("Error assigning window icon: %s\n", IMG_GetError());
	}
	SDL_SetWindowIcon(*ptrWindow, icon); // Poner icono a la ventana
	SDL_FreeSurface(icon);
	// Crear renderer
	*ptrRenderer = SDL_CreateRenderer(*ptrWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (*ptrRenderer == NULL) {
		printf("Error creating renderer: %s\n", SDL_GetError());
		return 0;
	}
	SDL_SetRenderDrawColor(*ptrRenderer, 0, 0, 0, 255); // Color fondo (Negro)

	return 1;
}

// Inicializar/Reiniciar tablero
void initPlayfield(Playfield *playfield) {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			if (j == 0 || j == BOARD_WIDTH - 1) {
				playfield->matrix[i][j] = '#';
			} else if (i == BOARD_HEIGHT - 1) {
				playfield->matrix[i][j] = '#';
			} else {
				playfield->matrix[i][j] = ' ';
			}
		}
	}
	printPlayfield(playfield);
}

// Funcion que inicializa objeto de la estructura Text
Text* initText(const char *str, Font *font, const SDL_Color color, const int x, const int y, const float size) {
	Text* text = malloc(sizeof(Text));
	*text = (Text) {	// Se castea a un dato tipo "Text", ya que como estamos inicializandolo desde un puntero tenemos que usar un literal compuesto (googlea "Compound literal")
		.string		= "", 									// String del texto (vacio por ahora)
		.font 		= TTF_OpenFont(font->path, font->size),	// Fuente (Cargada con ayuda de TTF_OpenFont("path del font", tamaño letra))
		.color 		= color,								// Color del texto
		.texture	= NULL,									// Textura (NULL ya que se crea y asigna posteriormente)
		.rect 		= {x, y, 0, 0},							// Rect del texto (Posicion/Tamaño), el tamaño se asigna posteriormente al crear la textura
		.size		= size
	};
	strcpy(text->string, str);	// Copiar parametro string en la string de estructura
	if (text->font == NULL) printf("ERROR INICIALIZANDO FONT: %s\n", TTF_GetError());
	return text;
}

// Funcion que asigna texturas a tetrominos
void loadTetrominoesTexture(SDL_Renderer *renderer) {
	// Asignar texturas
	for (int nShape = 0; nShape < 9; nShape++) {
		blockColors[nShape] = IMG_LoadTexture(renderer, blockPaths[nShape]);
		if (nShape == 7) ghostBlock = IMG_LoadTexture(renderer, blockPaths[nShape]);
		if (nShape == 8) lockBlock = IMG_LoadTexture(renderer, blockPaths[nShape]);
	}
}

// Cargar fondos
void loadBackgroundsTexture(SDL_Renderer *renderer, SDL_Texture **backgrounds) {
	for (int i = 0; i < 4; i++) {
		backgrounds[i] = IMG_LoadTexture(renderer, backgroundsPath[i]);
	}
}

void loadGameOverTexture(SDL_Renderer *renderer, SDL_Texture **gameover) {
	for (int i = 0; i < 9; i++) {
		gameover[i] = IMG_LoadTexture(renderer, gameOverPath[i]);
	}
	SDL_QueryTexture(gameover[1], NULL, NULL, &gameOverRect.w, &gameOverRect.h);
	gameOverRect.x = SCREEN_WIDTH / 2 - gameOverRect.w;
	gameOverRect.y = SCREEN_HEIGHT / 2 - gameOverRect.h;
}

// Funcion que carga textura de texto
void loadTextTexture(SDL_Renderer *renderer, Text *text) {
	if (text->texture != NULL) SDL_DestroyTexture(text->texture);

	SDL_Surface* textSurface = TTF_RenderText_Solid(text->font, text->string, text->color);
	if (textSurface == NULL) {
		printf("Error al intentar crear textSurface: %s\n", TTF_GetError());
	} else {
		text->rect.w = textSurface->w * text->size;
		text->rect.h = textSurface->h * text->size;
	}
	text->texture = SDL_CreateTextureFromSurface(renderer, textSurface);
	if (text->texture == NULL) printf("Error al intentar crear textTexture: %s\n", SDL_GetError());
	SDL_FreeSurface(textSurface);
}

//? Imprimir grilla actual
void printPlayfield(Playfield *playfield) {
	int row = 0, column = 0;
	printf("La matriz de la grilla actual es:\n");
	for (int i = 0; i < BOARD_HEIGHT + 1; i++) {
		if (i == 3) printf("   ---------------------------------------\n");
		if (i == BOARD_HEIGHT) printf("   ---------------------------------------\n");
		for (int j = 0; j < BOARD_WIDTH; j++) {
			if (j == 0 && i < BOARD_HEIGHT) printf("%2d | ", row++);
			if (i < BOARD_HEIGHT) printf("%2c ", playfield->matrix[i][j]);
			if (j == BOARD_WIDTH - 1 && i < BOARD_HEIGHT) printf("|\n");
			if (i == BOARD_HEIGHT) {
				if (j != 0) printf("%2d ", column++);
				else printf("     %2d ", column++);
			}
		}
	}
	printf("\n");
}

// Funcion que actualiza tablero
void updatePlayfield(Playfield *playfield, Tetromino *curr, Tetromino *next) {
	// Guardar caracteristicas de pieza dropeada
	lastDropedRow = curr->y;
	lastDropedSize = curr->size;
	printf("lastDropedRow: %d, lastDropedSize: %d!!!!!!!!!!\n", curr->y, curr->size); //? Test de drops

	// Ingresar pieza actual al stack
	for (int i = 0; i < curr->size; i++) {
		for (int j = 0; j < curr->size; j++) {
			if (curr->matrix[i][j]) {
				playfield->matrix[i + curr->y][j + curr->x] = curr->shape;
			}
		}
	}
	printPlayfield(playfield); //? Imprimir matrix actual
	countStackHeight(playfield); // Comprobar altura del Stack


	if (firstThreeDrops >= 3) clearedLines = (deleteLines(playfield));
	if (lastStackRow <= 5) gameOver = gameOverCheck(playfield, curr);

	if (gameOver) return;

	totalLines += clearedLines;

	newTetromino(curr, next);
}

// Generar nueva pieza pseudo aleatoria //TODO: FALTA AGREGAR 7-BAG
void newTetromino(Tetromino *curr, Tetromino *next) {
	*curr = *next;
	*next = tetrominoes[rand() % 7];
}

// Funcion que revisa si el jugador perdio //TODO: MEJORAR DETECCION DE GAMEOVER
bool gameOverCheck(Playfield *playfield, Tetromino *curr) {
	bool gameOver = false;
	// Contar bloques de la pieza encima del tablero, si los 4 estan arriba, pierdes
	int countBlocks = 0;
	for (int i = 0; i < curr->size; i++) {
		for (int j = 0; j < curr->size; j++) {
			// Coordenada Y respecto al tablero de bloque actual
			int coordY = i + curr->y;
			//TODO: Hacer mas flexible respecto a situacion del tablero y pieza actual
			if (curr->matrix[i][j] && coordY <= 2) countBlocks++;
		}
	}
	switch (countBlocks) {
		case 3: //* "Mini clutch" (Salvar juego si la ultima pieza logro limpiar al menos una linea antes de caer)
			if (clearedLines == 0) {
				printf("GAME OVER!!!!\n"); //? Avisar gameover
				restart = true; //? Quitar al implementar pantalla de gameover
				gameOver = true;
			}
			break;
		case 4: //* "Top out" (Pieza cayo totalmente de playfield)
			printf("GAME OVER!!!!\n"); //? Avisar gameover
			gameOver = true; //? Quitar al implementar pantalla de gameover
			break;
		default: break;
	}
	return gameOver;
}

// Funcion que comprueba estado de linea (-1 = borde, 0 = vacia, 1 = incompleta, 2 = completa)
int checkLineState(Playfield *playfield, Uint8 row) {
	int countBlocks = 0;
	for (int column = 1; column < BOARD_WIDTH - 1; column++) {
		if (playfield->matrix[row][column] == '#') return -1;
		if (playfield->matrix[row][column] != ' ') countBlocks++;
	}
	if (countBlocks == 10) return 2;
	else if (countBlocks == 0) return 0;
	else return 1;
}

// Funcion que elimina lineas completas por la anterior pieza dropeada
int deleteLines(Playfield *playfield) {
	Uint8 deletedLines = 0;
	
	Uint8 rowInterval = lastDropedRow + lastDropedSize;
	if (rowInterval > BOARD_HEIGHT - 2) rowInterval = BOARD_HEIGHT - 2;

	//* Llenar lineas completas de espacios vacios (Recorrer matriz hacia abajo)
	for (int row = lastDropedRow; row <= rowInterval; row++) {
		if (checkLineState(playfield, row) == 2) {
			for (int column = 1; column < BOARD_WIDTH - 1; column++) {
				playfield->matrix[row][column] = ' ';	
			}
			deletedLines++;
		}
	}

	//* Bajar piezas que quedaron flotando (Recorrer matriz hacia arriba)
	for (int mainRow = rowInterval; mainRow > lastStackRow; mainRow--) { 
		if (checkLineState(playfield, mainRow) == 0) {
			int secondRow = mainRow - 1; // Recorrer filas hasta encontrar filas incompletas
			while (checkLineState(playfield, secondRow) == 0 && secondRow > lastStackRow) secondRow--; // Saltarse lineas vacias
			// Swap linea vacia (mainRow) con siguiente linea incompleta (sencondRow)
			for (int column = 1; column < BOARD_WIDTH - 1; column++) {
				playfield->matrix[mainRow][column] = playfield->matrix[secondRow][column];
				playfield->matrix[secondRow][column] = ' ';
			}
		}
	}

	return deletedLines;
}

// Funcion que revisa colisiones de tetromino actual
bool collision(Playfield *playfield, Tetromino *curr) {
	for (int i = 0; i < curr->size; i++) {
		for (int j = 0; j < curr->size; j++) {
			if (curr->matrix[i][j] && playfield->matrix[i + curr->y][j + curr->x] != ' ') {
				return true;
			}
		}
	}
	return false;
}

// Funcion que cuenta altura actual del stack
void countStackHeight(Playfield *playfield) {
	for (int row = 0; row < BOARD_HEIGHT - 1; row++) {
		if (checkLineState(playfield, row) == 1) {
			lastStackRow = row;
			printf("lastStackRow: %d (altura: %d)\n", lastStackRow, (BOARD_HEIGHT - 1) - lastStackRow);
			break;
		}
	}
}

//TODO: WALL KICK PARA CASOS BASICOS COMO POR EJEMPLO COLISION CON BORDE CUANDO "lastRowStack" ES MENOR A "posY" DE FIGURA
void wallKickFRUNA(Playfield *playfield, Tetromino *curr, Sint8 rotation);

// Funcion que rota piezas
void rotateTetromino(Tetromino *tetro, const Sint8 sense) {
	Tetromino copy = *tetro;
	for(int i = 0; i < tetro->size; i++) {
			for(int j = 0; j < tetro->size; j++) {
				tetro->matrix[i][j] = copy.matrix[j][i];
			}
		}
	copy = *tetro;
	switch (sense) {
		case COUNTER_CLOCKWISE:
			for(int i = 0; i < tetro->size/2; i++) {
				for(int j = 0; j < tetro->size; j++) {
					bool t = copy.matrix[i][j];
					tetro->matrix[i][j] = copy.matrix[tetro->size - i - 1][j];
					tetro->matrix[tetro->size - i - 1][j] = t;
				}
			}
			break;
		case CLOCKWISE:
			for(int i = 0; i < tetro->size; i++) {
				for(int j = 0; j < tetro->size/2; j++) {
					bool t = copy.matrix[i][j];
					tetro->matrix[i][j] = copy.matrix[i][tetro->size - j - 1];
					tetro->matrix[i][tetro->size - j - 1] = t;
				}
			}
			break;
		case DOUBLE_CLOCKWISE:
			for(int i = 0; i < tetro->size; i++) {
				for(int j = 0; j < tetro->size/2; j++) {
					bool t = copy.matrix[i][j];
					tetro->matrix[i][j] = copy.matrix[i][tetro->size - j - 1];
					tetro->matrix[i][tetro->size - j - 1] = t;
				}
			}
			rotateTetromino(tetro, CLOCKWISE);
			break;
		default:
			break;
	}
}

// Funcion que hacer caer pieza al stack instantaneamente
void hardDropTetromino(Playfield *playfield, Tetromino *curr, Tetromino *next) {
	if (firstThreeDrops < 3) firstThreeDrops++;
	// Bajar hasta que sea posible colision
	if (curr->y < lastStackRow - 4) {
		curr->y = lastStackRow - 4;
	}
	// Bajar pieza hasta que choque
	while (!collision(playfield, curr)) curr->y++;
	curr->y--;

	droped = true;
	// Pequeño delay antes de que la nueva pieza baje de nuevo
	dropDelay = 60;
}

// Funcion que mueve un bloque abajo la pieza //TODO: MEJORAR SOFTDROP ****************************************************
void softDropTetromino(Playfield *playfield, Tetromino *curr, Tetromino *next) {
	curr->y++;
	if (collision(playfield, curr)) {
		curr->y--;
		droped = true;
		if (firstThreeDrops < 3) firstThreeDrops++;
	}
}

// Funcion que maneja delay de fall y dificultad //TODO: HACER VELOCIDAD DE CAIDA DEPENDIENTE DE DIFICULTAD ******************
bool checkFallTime(Uint64 countFrames) {
	// Fall (Cada 48 frames baja 1 celda)
	if (countFrames % (int) 48 == 0 && dropDelay == 0 && countFrames > 48) {
		return true;
	} else if (dropDelay > 0) {
		dropDelay--;
	}
	return false;
}

// Funcion que calcula score
void calculateScore(int linesCleared) {
	switch(linesCleared) {
		case 1: {score += 80   * (level + combo); break;} // Single
		case 2: {score += 200  * (level + combo); break;} // Double
		case 3: {score += 600  * (level + combo); break;} // Triple
		case 4: {score += 2400 * (level + combo); break;} // Tetris
	}
	if (linesCleared > 0) combo++;
	else combo = 0;

	if (totalLines % 20 == 0) {
		level++;
		levelUp = true;
	} else {
		levelUp = false;
	}
}

void calculateDifficulty(int level) {
	difficulty = pow((0.8 - ((level - 1) *0.007)), level - 1);
	printf("Level %d: caida cada %2.f frames\n", level, difficulty);
}

// Funcion que renderiza textura de texto
void renderText(SDL_Renderer *renderer, Text *text) {
	if (SDL_RenderCopy(renderer, text->texture, NULL, &text->rect) < 0) {
		printf("Error renderizando texto: %s\n", SDL_GetError());
	}
}

// Renderizar texturas de fondo
void renderBackground(SDL_Renderer *renderer, SDL_Texture *background, SDL_Texture *gameboardInt) {
	SDL_RenderCopy(renderer, background, NULL, NULL);
	SDL_RenderCopy(renderer, gameboardInt, NULL, NULL);
}

void renderGameOver(SDL_Renderer *renderer, SDL_Texture **gameOverTextures, Text *textRecord) {
	SDL_RenderClear(renderer);

	SDL_RenderCopy(renderer, gameOverTextures[0], NULL, NULL);
	if (countFrames % 20) currGameOver++;
	if (currGameOver > 8) currGameOver = 0;
	SDL_RenderCopy(renderer, gameOverTextures[currGameOver], NULL, &gameOverRect);
	renderText(renderer, textRecord);

	SDL_RenderPresent(renderer);
}

void renderNextHold(SDL_Renderer *renderer, Tetromino *next, Tetromino *holder) {
	int currRectNext, currRectHolder, nextX, nextY, holderX, holderY;
	currRectNext = 0, currRectHolder = 0, nextX = 665, nextY = 200, holderX = 90, holderY = 200;
	switch (next->shape) {
		case 'O': nextX += 20; break;
		case 'I': nextX -= 20; nextY -= 5; break;
		default: break;
	}
	for(int i = 0; i < next->size; i++) {
		for(int j = 0; j < next->size; j++) {
			if(next->matrix[i][j]) {
				next->rects[currRectNext].w = next->rects[currRectNext].h = TILE_SIZE + 1;
				next->rects[currRectNext].x = (j * (TILE_SIZE + 1)) + nextX;
				next->rects[currRectNext].y = (i * (TILE_SIZE + 1)) + nextY;
				SDL_RenderCopy(renderer, blockColors[next->nShape], NULL, &next->rects[currRectNext]);
				currRectNext++;
			}
		}
	}
	if (firstHold) {
		switch (holder->shape) {
			case 'O': holderX += 20; break;
			case 'I': holderX -= 20; holderY -= 5; break;
			default: break;
		}
		for(int i = 0; i < holder->size; i++) {
			for(int j = 0; j < holder->size; j++) {
				if(holder->matrix[i][j]) {
					holder->rects[currRectHolder].w = holder->rects[currRectHolder].h = TILE_SIZE + 1;
					holder->rects[currRectHolder].x = (j * (TILE_SIZE + 1)) + holderX;
					holder->rects[currRectHolder].y = (i * (TILE_SIZE + 1)) + holderY;
					if (holded) SDL_RenderCopy(renderer, lockBlock, NULL, &holder->rects[currRectHolder]);
					else SDL_RenderCopy(renderer, blockColors[holder->nShape], NULL, &holder->rects[currRectHolder]);
					currRectHolder++;
				}
			}
		}
	}
}

void renderTetromino(SDL_Renderer *renderer, Tetromino *tetro) {
	int currRect = 0;
	for(int i = 0; i < tetro->size; i++) {
		for(int j = 0; j < tetro->size; j++) {
			if(tetro->matrix[i][j]) {
				tetro->rects[currRect].w = tetro->rects[currRect].h = TILE_SIZE;
				// tetro->rects[currRect].w = tetro->rects[currRect].h = TILE_SIZE + 1;
				tetro->rects[currRect].x = ((tetro->x - 1 + j) * (TILE_SIZE + 1)) + BOARD_X_ORIGIN;
				tetro->rects[currRect].y = ((tetro->y + i) * (TILE_SIZE + 1)) + BOARD_Y_ORIGIN;
				SDL_RenderCopy(renderer, blockColors[tetro->nShape], NULL, &tetro->rects[currRect]);
				currRect++;
			}
		}
	}
}

void renderGhostTetromino(SDL_Renderer *renderer, Playfield *playfield, Tetromino *curr) {
	bool noFitting = false;
	int currRect = 0;
	Tetromino currGhost = *curr;
	while (!noFitting) {
		if (collision(playfield, &currGhost)) {
			currGhost.y--;
			for(int i = 0; i < currGhost.size; i++) {
				for(int j = 0; j < currGhost.size; j++) {
					if(currGhost.matrix[i][j]) {
						currGhost.rects[currRect].w = currGhost.rects[currRect].h = TILE_SIZE;
						currGhost.rects[currRect].x = ((currGhost.x - 1 + j) * (TILE_SIZE + 1)) + BOARD_X_ORIGIN;
						currGhost.rects[currRect].y = ((currGhost.y + i) * (TILE_SIZE + 1)) + BOARD_Y_ORIGIN;
						SDL_RenderCopy(renderer, ghostBlock, NULL, &currGhost.rects[currRect]);
						currRect++;
					}
				}
			}
			noFitting = true;
		} else currGhost.y++;
	}
}

void renderPlayfield(SDL_Renderer *renderer, Playfield *playfield) {
	for (int y = BOARD_HEIGHT - 2; y >= 0; y--) {
		for (int x = 1; x < BOARD_WIDTH - 1; x++) {
			if (playfield->matrix[y][x] != ' ') {
				SDL_Rect tmpRect;
				int nShape;
				tmpRect.w = tmpRect.h = TILE_SIZE;
				tmpRect.x = ((x - 1) * (TILE_SIZE + 1)) + BOARD_X_ORIGIN;
				tmpRect.y = (y * (TILE_SIZE + 1)) + BOARD_Y_ORIGIN;
				switch (playfield->matrix[y][x]) {
					case 'L': nShape = 0; break;
					case 'Z': nShape = 1; break;
					case 'I': nShape = 2; break;
					case 'J': nShape = 3; break;
					case 'O': nShape = 4; break;
					case 'S': nShape = 5; break;
					case 'T': nShape = 6; break;
					default: break;
				}
				if (SDL_RenderCopy(renderer, blockColors[nShape], NULL, &tmpRect) < 0) {
					printf("ERROR RENDER PLAYFIELD: %s\n", SDL_GetError());
				}
			}
		}
	}
}

// Funcion que libera texto junto a su textura y fuente cargada
void freeText(Text *text) {
	SDL_DestroyTexture(text->texture);
	TTF_CloseFont(text->font);
	free(text);
}

// Funcion que apaga y limpia todos los subsystemas de SDL usados.
void QuitSDL(SDL_Window *window, SDL_Renderer *renderer) {
	// bool error = false;
	// if (*SDL_GetError() != '\0') {
	// 	printf("LAST SDL ERROR: %s\n", SDL_GetError());
	// 	error = true;
	// }
	// if (*TTF_GetError() != '\0') {
	// 	printf("LAST SDL_TTF ERROR: %s\n", TTF_GetError());
	// 	error = true;
	// }
	// if (*IMG_GetError() != '\0') {
	// 	printf("LAST SDL_IMG ERROR: %s\n", IMG_GetError());
	// 	error = true;
	// }
	// if (error) SDL_Delay(3000);

	TTF_Quit();
	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
