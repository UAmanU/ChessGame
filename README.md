# ChessGame

Небольшие шахматы против компьютера на C++20 и Raylib. Белыми управляет игрок, чёрными — движок Minimax с Alpha-Beta pruning.

## Сборка

Из PowerShell в каталоге проекта:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j1
ctest --test-dir build --output-on-failure
```

CMake использует статическую библиотеку Raylib из `../Libraries/raylib_files`.

## Управление

- Нажмите «Играть», затем щёлкните по белой фигуре и по подсвеченной клетке назначения.
- `Esc` возвращает в меню.
- После завершения партии щёлкните, чтобы вернуться в меню.

В `include/config/` находятся настройки окна, цветов и подписей меню. Шахматные фигуры отображаются символами Unicode, доступными встроенному шрифту Raylib.
