Para a compilação do código do jogo basta instalar a biblioteca "Raylib" e utilizar o seguinte comando no terminal:

gcc main.c -o jogo_trex.exe -IC:/raylib/raylib-5.5_win64_mingw-w64/include -LC:/raylib/raylib-5.5_win64_mingw-w64/lib -lraylibdll -lopengl32 -lgdi32 -lwinmm