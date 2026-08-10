#include "raylib.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

static constexpr int SCROLLBAR_WIDTH{17};
static constexpr int MAXIMUM_ALBUM_WIDTH{255};

struct Album {
  std::filesystem::path path;
  Texture2D cover;
};

std::ostream& operator<<(std::ostream& os, const Album& a) {
  return os << a.path;
}

void playAlbum(Album a) {
  if (fork() != 0) return;
  if (setsid() < 0) std::exit(EXIT_FAILURE);
  pid_t pid = fork();
  if (pid < 0) std::exit(EXIT_FAILURE);
  if (pid > 0) std::exit(EXIT_SUCCESS);

  std::vector<char*> argv;
  argv.reserve(3);
  argv.push_back(const_cast<char*>("mpv"));

  auto pathStr{a.path.string()};
  argv.push_back(const_cast<char*>(pathStr.c_str()));
  argv.push_back(nullptr);
  execvp(argv[0], argv.data());
  std::exit(EXIT_FAILURE);
}

int main() {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1, 1, "fucture");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  SetExitKey(KEY_NULL);

  const std::filesystem::path root{"/home/asday/bombs"};
  std::vector<Album> albums{};
  using directory_iterator = std::filesystem::directory_iterator;
  for (const auto& de : directory_iterator(root)) {
    Album album{};
    album.path = de.path();
    for (const auto& maybeCover : directory_iterator(de.path())) {
      if (maybeCover.path().stem() == "cover") {
        Image cover{LoadImage(maybeCover.path().c_str())};
        album.cover = LoadTextureFromImage(cover);
        UnloadImage(cover);
        break;
      }
    }
    albums.push_back(album);
  }

  int scrollPos{0};
  int screenWidth{-1};
  int screenHeight{-1};
  int albumSize{-1};
  int abreast{0};
  int rows{0};
  BeginDrawing(); EndDrawing();  // Make first `GetScreenWidth()` not `1`.
  while (!WindowShouldClose()) {
    {
      auto newWidth{GetScreenWidth()};
      if (screenWidth != newWidth) {
        screenWidth = newWidth;

        abreast = (screenWidth - SCROLLBAR_WIDTH) / MAXIMUM_ALBUM_WIDTH;
        if (abreast <= 0) abreast = 1;
        rows = ((static_cast<int>(albums.size()) + abreast - 1) / abreast);
        albumSize = (screenWidth - SCROLLBAR_WIDTH) / abreast;
        if (albumSize > MAXIMUM_ALBUM_WIDTH) albumSize = MAXIMUM_ALBUM_WIDTH;
      }
    }
    screenHeight = GetScreenHeight();

    if (IsKeyPressed(KEY_DOWN)) scrollPos += albumSize;
    if (IsKeyPressed(KEY_UP)) scrollPos -= albumSize;
    if (IsKeyPressed(KEY_PAGE_DOWN)) scrollPos += screenHeight;
    if (IsKeyPressed(KEY_PAGE_UP)) scrollPos -= screenHeight;

    if (scrollPos < 0) scrollPos = 0;
    if (scrollPos > (rows * albumSize) - screenHeight) {
      scrollPos = (rows * albumSize) - screenHeight;
    }

    BeginDrawing();
    {
      ClearBackground(BLACK);
      int x{0};
      int y{0};
      int viewMax{scrollPos + screenHeight};
      for (const auto& album : albums) {
        // Either the top's gotta be in the viewport, or the bottom
        // has gotta be in the viewport, or the top has got to be above
        // the top of the viewport and the bottom has got to be below
        // the bottom of the viewport, to be visible.
        int top{y * albumSize};
        int bottom{((y + 1) * albumSize) - 1};
        if (
          (top >= scrollPos && top <= viewMax)
          || (bottom >= scrollPos && bottom <= viewMax)
          || (top < scrollPos && bottom > viewMax)

        ) {
          DrawTexturePro(
            album.cover,
            {
              0.0f,
              0.0f,
              static_cast<float>(album.cover.width),
              static_cast<float>(album.cover.height)
            },
            {
              static_cast<float>(x * albumSize),
              static_cast<float>((y * albumSize) - scrollPos),
              static_cast<float>(albumSize),
              static_cast<float>(albumSize)
            },
            {0.0f, 0.0f},
            0.0f,
            WHITE
          );
        }

        x += 1;
        if (x >= abreast) { x = 0; y += 1; }
      }

      // Gutter represents the height of the content, handle height
      // represents the height of the view.
      //
      // Handle is defined by its top edge.
      DrawRectangle(
        screenWidth - SCROLLBAR_WIDTH,
        0,
        SCROLLBAR_WIDTH,
        screenHeight,
        DARKGRAY
      );

      DrawRectangle(
        screenWidth - SCROLLBAR_WIDTH,
        // `scrollPos` scaled by however much the gutter is scaled.
        // TODO: this might be horribly wrong.
        scrollPos * screenHeight / (rows * albumSize),
        SCROLLBAR_WIDTH,
        (screenHeight * screenHeight) / (rows * albumSize),
        GRAY
      );
    }
    EndDrawing();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      int x{GetMouseX()};
      if (x <= albumSize * abreast) {
        int y{GetMouseY()};
        y += scrollPos;
        y /= albumSize;
        x /= albumSize;
        auto i{static_cast<decltype(albums)::size_type>((y * abreast) + x)};
        if (i < albums.size()) playAlbum(albums[i]);
      }
    }

    if (false /*scrollStart != -1*/) {
      SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    } else if (IsCursorOnScreen() || IsWindowFocused()) {
      SetTargetFPS(30);
    } else {
      SetTargetFPS(15);
    }
  }

  CloseWindow();

  for (const auto& album : albums) UnloadTexture(album.cover);

  return 0;
}
