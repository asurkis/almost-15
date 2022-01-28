#include <algorithm>
#include <raylib.h>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

using namespace std;

static const int SCREEN_SIZE = 900;
static const int CELL_SIZE_HALF = 80;
static const int CELL_SIZE = 2 * CELL_SIZE_HALF;
static const int CELL_MARGIN = 3;
static const int CELL_OFFSET = CELL_SIZE + 2 * CELL_MARGIN;
static const int BOARD_SIZE = 4 * CELL_OFFSET;
static const int BOARD_START = (SCREEN_SIZE - BOARD_SIZE) / 2;
static const int BOARD_OUTER_SIZE = BOARD_SIZE + 2 * CELL_MARGIN;
static const int BOARD_OUTER_START = BOARD_START - CELL_MARGIN;
static const int CELL_FONT_SIZE = 40;
static const Color BOARD_BACKGROUND_COLOR = {16, 16, 16, 255};
static const Color CELL_TEXT_COLOR = BOARD_BACKGROUND_COLOR;
static const Color HIGHLIGHT_COLOR = {0, 0, 255, 64};

struct Game {
  int empty_x;
  int empty_y;
  int board[4][4];

  Game() {
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        board[i][j] = 4 * i + j + 1;
      }
    }
    empty_x = 3;
    empty_y = 3;
    board[empty_y][empty_x] = 0;
  }

  void shuffle(int steps) {
    for (int i = 0; i < steps; ++i) {
      int move = GetRandomValue(0, 3);
      int dx = 0, dy = 0;
      (move > 1 ? dx : dy) = 2 * (move % 2) - 1;
      int x = empty_x + dx;
      int y = empty_y + dy;
      if (x < 0 || y < 0 || x > 3 || y > 3) {
        x = empty_x - dx;
        y = empty_y - dy;
      }
      // printf("move=%d, dx=%d, dy=%d, x=%d y=%d\n", move, dx, dy, x, y);
      make_move(x, y);
    }
  }

  bool make_move(int x, int y) {
    int dx = x - empty_x;
    int dy = y - empty_y;
    if (abs(dx) + abs(dy) != 1) {
      return false;
    }

    swap(board[y][x], board[empty_y][empty_x]);
    empty_x = x;
    empty_y = y;
    return true;
  }
};

static int cell_to_screen(int coord) {
  return BOARD_START + CELL_OFFSET * coord;
}

static int screen_to_cell(int coord) {
  return (coord - BOARD_START) / CELL_OFFSET;
}

struct Game game;

void iter_main_loop() {
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
    game.make_move(selected_cell_x, selected_cell_y);
  }

  BeginDrawing();
  ClearBackground(RAYWHITE);
  DrawRectangle(BOARD_OUTER_START, BOARD_OUTER_START, BOARD_OUTER_SIZE,
                BOARD_OUTER_SIZE, BOARD_BACKGROUND_COLOR);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (i == game.empty_y && j == game.empty_x) {
        continue;
      }
      DrawRectangle(cell_to_screen(j) + CELL_MARGIN,
                    cell_to_screen(i) + CELL_MARGIN, CELL_SIZE, CELL_SIZE,
                    RAYWHITE);
      const char *text = TextFormat("%d", game.board[i][j]);
      int width = MeasureText(text, CELL_FONT_SIZE);
      DrawText(text, cell_to_screen(j) + CELL_MARGIN + (CELL_SIZE - width) / 2,
               cell_to_screen(i) + CELL_MARGIN +
                   (CELL_SIZE - CELL_FONT_SIZE) / 2,
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

int main() {
  InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Almost 15 Game");

  game.shuffle(10000);

#ifdef PLATFORM_WEB
  emscripten_set_main_loop(iter_main_loop, 0, 1);
#else
  while (!WindowShouldClose()) {
    iter_main_loop();
  }
#endif

  CloseWindow();
  return 0;
}
