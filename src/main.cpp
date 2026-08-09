#include "raylib.h"

#include <filesystem>
#include <iostream>
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

int main() {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1, 1, "fucture");
  SetTargetFPS(30);
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
  int scrollMax{0};
  int lastWidth{-1};
  int lastHeight{-1};
  int albumSize{-1};
  int abreast{0};
  BeginDrawing(); EndDrawing();  // Make first `GetScreenWidth()` not `1`.
  while (!WindowShouldClose()) {
    {
      auto screenWidth{GetScreenWidth()};
      if (lastWidth != screenWidth) {
        lastWidth = screenWidth;

        abreast = (screenWidth - SCROLLBAR_WIDTH) / MAXIMUM_ALBUM_WIDTH;
        if (abreast <= 0) abreast = 1;
        albumSize = (screenWidth - SCROLLBAR_WIDTH) / abreast;
        if (albumSize > MAXIMUM_ALBUM_WIDTH) albumSize = MAXIMUM_ALBUM_WIDTH;
      }
    }

    {
      auto screenHeight{GetScreenHeight()};
      if (lastHeight != screenHeight) {
        lastHeight = screenHeight;

        int rows {((static_cast<int>(albums.size()) + abreast - 1) / abreast)};
        scrollMax = (albumSize * rows) - screenHeight;
        if (scrollMax < 0) scrollMax = 0;
      }
    }

    BeginDrawing();
    {
      ClearBackground(BLACK);
      int x{0};
      int y{0};
      for (const auto& album : albums) {
        if (
          scrollPos <= y * albumSize
          && scrollPos + lastHeight > y * albumSize
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
              static_cast<float>(y * albumSize),
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
    }
    EndDrawing();
  }

  CloseWindow();

  for (const auto& album : albums) UnloadTexture(album.cover);

  return 0;
}
