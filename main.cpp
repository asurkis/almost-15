#include <bits/stdc++.h>
#include <raylib.h>

using namespace std;

const int SCREEN_SIZE = 900;
const int CELL_SIZE_HALF = 80;
const int CELL_SIZE = 2 * CELL_SIZE_HALF;
const int CELL_MARGIN = 3;
const int BORDER_WIDTH = 2 * CELL_MARGIN;
const int CELL_OFFSET = CELL_SIZE + 2 * CELL_MARGIN;
const int BOARD_SIZE = 4 * CELL_OFFSET;
const int BOARD_START = (SCREEN_SIZE - BOARD_SIZE) / 2;
const int BOARD_OUTER_SIZE = BOARD_SIZE + 2 * CELL_MARGIN;
const int BOARD_OUTER_START = BOARD_START - CELL_MARGIN;
const int CELL_FONT_SIZE = 40;
const Color BOARD_BACKGROUND_COLOR = {16, 16, 16, 255};
const Color CELL_TEXT_COLOR = BOARD_BACKGROUND_COLOR;
const Color HIGHLIGHT_COLOR = {0, 0, 255, 64};

struct GameState {
  int empty_x;
  int empty_y;
  int board[4][4];
};

int cell_to_screen(int coord) { return BOARD_START + CELL_OFFSET * coord; }

int screen_to_cell(int coord) { return (coord - BOARD_START) / CELL_OFFSET; }

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
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    int selected_cell_x = -1;
    int selected_cell_y = -1;

    bool mouse_over_cell =
        BOARD_START <= mouse_x && mouse_x < BOARD_START + BOARD_SIZE &&
        BOARD_START <= mouse_y && mouse_y < BOARD_START + BOARD_SIZE;
    if (mouse_over_cell) {
      selected_cell_x = screen_to_cell(mouse_x);
      selected_cell_y = screen_to_cell(mouse_y);
    }

    bool is_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (is_mouse_pressed && mouse_over_cell) {
      int dx = selected_cell_x - state.empty_x;
      int dy = selected_cell_y - state.empty_y;
      if (abs(dx) + abs(dy) == 1) {
        swap(state.board[selected_cell_y][selected_cell_x],
             state.board[state.empty_y][state.empty_x]);
        state.empty_x = selected_cell_x;
        state.empty_y = selected_cell_y;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(BOARD_OUTER_START, BOARD_OUTER_START, BOARD_OUTER_SIZE,
                  BOARD_OUTER_SIZE, BOARD_BACKGROUND_COLOR);
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        if (i == state.empty_y && j == state.empty_x) {
          continue;
        }
        DrawRectangle(cell_to_screen(j) + CELL_MARGIN,
                      cell_to_screen(i) + CELL_MARGIN, CELL_SIZE, CELL_SIZE,
                      RAYWHITE);
        const char *text = TextFormat("%d", state.board[i][j]);
        int width = MeasureText(text, CELL_FONT_SIZE);
        DrawText(
            text, cell_to_screen(j) + CELL_MARGIN + (CELL_SIZE - width) / 2,
            cell_to_screen(i) + CELL_MARGIN + (CELL_SIZE - CELL_FONT_SIZE) / 2,
            CELL_FONT_SIZE, CELL_TEXT_COLOR);
      }
    }

    if (mouse_over_cell) {
      DrawRectangle(cell_to_screen(selected_cell_x),
                    cell_to_screen(selected_cell_y), CELL_OFFSET, CELL_OFFSET,
                    HIGHLIGHT_COLOR);
    }
    EndDrawing();
  }
}
