#include <queue>
#include <raylib.h>
#include <stdio.h>

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
static const double CELL_ANIMATION_DURATION = 0.125;

static const char *VICTORY_TEXT = "Oh, that was close!";
static const int VICTORY_FONT_SIZE = 40;
static const Color VICTORY_TEXT_COLOR = {0, 128, 128, 255};

static const char *VICTORY_TEXT_2 =
    "The truth is, the game was rigged from the start";
static const int VICTORY_FONT_SIZE_2 = 30;
static const Color VICTORY_TEXT_COLOR_2 = {128, 0, 128, 255};

template <typename T> static T cell_to_screen(const T &coord) {
  return BOARD_START + CELL_OFFSET * coord;
}

template <typename T> static T screen_to_cell(const T &coord) {
  return (coord - BOARD_START) / CELL_OFFSET;
}

struct Keyframe {
  double ts_start;
  double ts_end;
  int x;
  int y;
};

struct AnimatedCell {
  queue<Keyframe> keyframes;
  int id;
  int last_x;
  int last_y;

  void draw(double ts) {
    while (!keyframes.empty() && keyframes.front().ts_end < ts) {
      auto &kf = keyframes.front();
      last_x = kf.x;
      last_y = kf.y;
      keyframes.pop();
    }

    double draw_x = last_x;
    double draw_y = last_y;

    if (!keyframes.empty()) {
      auto &kf = keyframes.front();
      double next_x = kf.x;
      double next_y = kf.y;
      double t = (ts - kf.ts_start) / (kf.ts_end - kf.ts_start);
      if (t < 0) {
        t = 0;
      }
      /*
      q = a t^3 + b t^2 + c t + d
      q(0) = 0
      q(1) = 1
      q'(0) = 0
      q'(1) = 0

      d = 0
      a + b + c + d = 1
      c = 0
      3a + 2b + c = 0

      c = d = 0
      a + b = 1
      b = -1.5 a

      -0.5 a = 1
      a = -2
      b = 3
      */
      double q = t * t * (3 - 2 * t);
      draw_x = (1 - q) * last_x + q * next_x;
      draw_y = (1 - q) * last_y + q * next_y;
    }

    draw_x = cell_to_screen(draw_x) + CELL_MARGIN;
    draw_y = cell_to_screen(draw_y) + CELL_MARGIN;

    DrawRectangleV({(float)draw_x, (float)draw_y}, {CELL_SIZE, CELL_SIZE},
                   RAYWHITE);

    const char *text = TextFormat("%d", id);
    int text_size_x = MeasureText(text, CELL_FONT_SIZE);
    int text_size_y = CELL_FONT_SIZE;
    DrawText(text, draw_x + 0.5 * (CELL_SIZE - text_size_x),
             draw_y + 0.5 * (CELL_SIZE - text_size_y), CELL_FONT_SIZE,
             CELL_TEXT_COLOR);
  }
};

struct Game {
  AnimatedCell *board[4][4];
  AnimatedCell cells[15];
  double last_kf_ts;
  int empty_x;
  int empty_y;
  bool player_won;
  double victory_ts;

  void init() {
    last_kf_ts = 0;
    player_won = false;

    for (int i = 0; i < 15; ++i) {
      int y = i / 4;
      int x = i % 4;
      board[y][x] = &cells[i];
      cells[i].id = i + 1;
      cells[i].last_x = x;
      cells[i].last_y = y;
    }

    empty_x = 3;
    empty_y = 3;
    board[empty_y][empty_x] = nullptr;
  }

  void shuffle(int steps) {
    init();
    swap(board[3][2], board[2][3]);

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
      make_move(x, y);
    }

    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        auto cell = board[i][j];
        if (!cell) {
          continue;
        }

        while (!cell->keyframes.empty()) {
          cell->keyframes.pop();
        }
        cell->last_x = j;
        cell->last_y = i;
      }
    }

    player_won = false;
  }

  bool make_move(int x, int y) {
    int dx = x - empty_x;
    int dy = y - empty_y;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    if (adx + ady != 1) {
      return false;
    }

    swap(board[y][x], board[empty_y][empty_x]);
    empty_x = x;
    empty_y = y;
    return true;
  }

  bool make_animated_move(double ts, int x, int y) {
    auto cell = board[y][x];
    Keyframe kf;
    kf.ts_start = max(ts, last_kf_ts);
    kf.ts_end = kf.ts_start + CELL_ANIMATION_DURATION;
    kf.x = empty_x;
    kf.y = empty_y;

    if (!make_move(x, y)) {
      return false;
    }

    last_kf_ts = kf.ts_end;
    cell->keyframes.push(kf);

    if (!player_won) {
      if (check_victory()) {
        player_won = true;
        victory_ts = ts;
      }
    }

    return true;
  }

  void draw(double ts) {
    ClearBackground(RAYWHITE);
    DrawRectangle(BOARD_OUTER_START, BOARD_OUTER_START, BOARD_OUTER_SIZE,
                  BOARD_OUTER_SIZE, BOARD_BACKGROUND_COLOR);
    for (auto &cell : cells) {
      cell.draw(ts);
    }

    if (player_won) {
      int size_x = MeasureText(VICTORY_TEXT, VICTORY_FONT_SIZE);
      int size_y = VICTORY_FONT_SIZE;
      DrawText(VICTORY_TEXT, (SCREEN_SIZE - size_x) / 2,
               (BOARD_OUTER_START - size_y) / 2, VICTORY_FONT_SIZE,
               VICTORY_TEXT_COLOR);

      double faded_in = (ts - (victory_ts + 3)) / 5;
      if (faded_in < 0) {
        faded_in = 0;
      }
      if (faded_in > 1) {
        faded_in = 1;
      }
      size_x = MeasureText(VICTORY_TEXT_2, VICTORY_FONT_SIZE_2);
      size_y = VICTORY_FONT_SIZE_2;
      Color c = VICTORY_TEXT_COLOR_2;
      c.a = 255 * faded_in;
      DrawText(VICTORY_TEXT_2, (SCREEN_SIZE - size_x) / 2,
               (BOARD_OUTER_START + BOARD_OUTER_SIZE + SCREEN_SIZE - size_y) /
                   2,
               VICTORY_FONT_SIZE_2, c);
    }
  }

  bool check_victory() {
    int right_place = 0;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        auto cell = board[i][j];
        int real_id = cell ? cell->id - 1 : 15;
        int expected_i = real_id / 4;
        int expected_j = real_id % 4;
        if (i == expected_i && j == expected_j) {
          ++right_place;
        }
      }
    }
    return right_place >= 14;
  }
};

Game game;
queue<pair<int, int>> animations;
int prev_touch_count = 0;

void iter_main_loop() {
  double ts = GetTime();

  bool is_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  int mouse_x = GetMouseX();
  int mouse_y = GetMouseY();
  int selected_cell_x = -1;
  int selected_cell_y = -1;

  int touch_count = GetTouchPointCount();
  if (prev_touch_count == 0 && touch_count > 0) {
    is_mouse_pressed = true;
    mouse_x = GetTouchX();
    mouse_y = GetTouchY();
  }
  prev_touch_count = touch_count;

  bool mouse_over_cell =
      BOARD_START <= mouse_x && mouse_x < BOARD_START + BOARD_SIZE &&
      BOARD_START <= mouse_y && mouse_y < BOARD_START + BOARD_SIZE;
  if (mouse_over_cell) {
    selected_cell_x = screen_to_cell(mouse_x);
    selected_cell_y = screen_to_cell(mouse_y);
  }

  if (is_mouse_pressed && mouse_over_cell) {
    game.make_animated_move(ts, selected_cell_x, selected_cell_y);
  }

  BeginDrawing();
  game.draw(ts);
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
