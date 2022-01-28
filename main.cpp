#include <bits/stdc++.h>
#include <raylib.h>

using namespace std;

const int SCREEN_SIZE = 900;
const int CELL_SIZE = 160;
const int BORDER_WIDTH = 6;
const int CELL_OFFSET = CELL_SIZE + BORDER_WIDTH;
const int BOARD_SIZE = 4 * CELL_OFFSET + BORDER_WIDTH;
const int BOARD_START = (SCREEN_SIZE - BOARD_SIZE) / 2;
const int CELL_FONT_SIZE = 40;
const Color BOARD_BACKGROUND_COLOR = {16, 16, 16, 255};
const Color CELL_TEXT_COLOR = BOARD_BACKGROUND_COLOR;

struct GameState {
  int empty_x;
  int empty_y;
  int board[4][4];
};

int main() {
  InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Almost 15 Game");

  struct GameState state;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      state.board[i][j] = 4 * i + j + 1;
    }
  }
  state.empty_x = 3;
  state.empty_y = 3;
  state.board[state.empty_y][state.empty_x] = 0;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(BOARD_START, BOARD_START, BOARD_SIZE, BOARD_SIZE,
                  BOARD_BACKGROUND_COLOR);
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        if (i == state.empty_y && j == state.empty_x) {
          continue;
        }
        DrawRectangle(BOARD_START + CELL_OFFSET * j + BORDER_WIDTH,
                      BOARD_START + CELL_OFFSET * i + BORDER_WIDTH, CELL_SIZE,
                      CELL_SIZE, RAYWHITE);
        const char *text = TextFormat("%d", state.board[i][j]);
        int width = MeasureText(text, CELL_FONT_SIZE);
        DrawText(text,
                 BOARD_START + CELL_OFFSET * j + BORDER_WIDTH +
                     (CELL_SIZE - width) / 2,
                 BOARD_START + CELL_OFFSET * i + BORDER_WIDTH +
                     (CELL_SIZE - CELL_FONT_SIZE) / 2,
                 CELL_FONT_SIZE, CELL_TEXT_COLOR);
      }
    }
    EndDrawing();
  }
}
