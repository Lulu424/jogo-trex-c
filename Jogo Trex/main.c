#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    OBSTACULO_TIPO_CHAO,
    OBSTACULO_TIPO_CEU
} TipoObstaculo;

typedef struct Obstaculo {
    Rectangle ret;
    TipoObstaculo tipo;
    bool ativo;
    bool pontuado;
} Obstaculo;

void IniciarObstaculo(Obstaculo *obs, float x, float y, float w, float h) {
    obs->ret = (Rectangle){ x, y, w, h };
    obs->ativo = true;
    obs->pontuado = false;
}

void SalvarRecorde(int recorde) {
    FILE *f = fopen("recorde.txt", "w");
    if (f) {
        fprintf(f, "%d", recorde);
        fclose(f);
    }
}

int CarregarRecorde() {
    int r = 0;
    FILE *f = fopen("recorde.txt", "r");
    if (f) {
        fscanf(f, "%d", &r);
        fclose(f);
    }
    return r;
}

int main(void) {
    const int larguraTela = 1280; // Largura da tela
    const int alturaTela = 720; // Altura da tela
    const float nivelChao = alturaTela - 80; // Posição do chão
    bool mostrandoRecorde = false;
    int recorde = CarregarRecorde();

    InitWindow(larguraTela, alturaTela, "Jogo Trex");

    // Inicializa a imagem do menu
    Texture2D telaInicial = LoadTexture("texturas/tela_inicial.png");
    bool jogoIniciado = false;

    // Carregar texturas e sons
    Texture2D fundo = LoadTexture("texturas/fundo.png");
    Texture2D texturaRex = LoadTexture("texturas/trex.png");
    Texture2D texturaObstaculoChao = LoadTexture("texturas/obstaculo.png");
    Texture2D texturaObstaculoCeu = LoadTexture("texturas/fly.png");
    Texture2D recordefundo = LoadTexture("texturas/recordefundo.png");
    InitAudioDevice();
    Music musicaFundo = LoadMusicStream("sons/musica_fundo.wav");
    PlayMusicStream(musicaFundo);
    Sound somColisao = LoadSound("sons/colisao.wav"); 
    Sound somPonto = LoadSound("sons/ponto.wav"); 

    // Jogador
    float deslocamentoRex = -120; //Altura do Trex em relação ao chão
    float escalaRex = 0.5f; // Escala do T-Rex
    Vector2 jogador = { 0, nivelChao - texturaRex.height - deslocamentoRex };
    float velocidadeY = 0;
    bool pulando = false;
    const float gravidade = 0.5f; // Gravidade aplicada ao T-Rex
    const float forcaPulo = -15.0f; // Força do pulo do T-Rex

    // Obstáculos
    Obstaculo obstaculos[5] = {0};
    float velocidadeObstaculo = 6.0f;  
    float tempoGerarObstaculo = 1.5f;
    float tempoDesdeUltimoObstaculo = 0;
    float escalaChao = 0.18f; // Escala dos obstáculos do chão
    float escalaCeu = 0.33f; // Escala dos obstáculos do céu
    float reduzirColisaoChao = 17.0f; // Redução para colisão do obstáculo do chão
    float reduzirColisaoCeu = 35.0f; // Redução para colisão do obstáculo do céu

    // Redução para colisão
    float reduzirColisaoTrex = 17.0f; // ajuste menor para T-Rex

    // Tempo de jogo
    float tempoJogo = 120.0f; // Tempo total do jogo em segundos
    bool jogoAcabou = false;

    // Representação de colisão
    Rectangle rex;
    SetTargetFPS(60);

    int pontuacao = 0;
    int obstaculosAtivos = 0;
    bool MusicaParada = false;

    while (!WindowShouldClose()) {

        // Atualiza a música de fundo
        UpdateMusicStream(musicaFundo);

        //Menu ou tela inicial
        if (!jogoIniciado && !mostrandoRecorde) {
            BeginDrawing();
            DrawTexturePro(
                telaInicial,
                (Rectangle){0, 0, telaInicial.width, telaInicial.height},
                (Rectangle){0, 0, larguraTela, alturaTela},
                (Vector2){0, 0},
                0.0f,
                WHITE
            );

            // Título centralizado
            const char* titulo = "T-REX GAME";
            int larguraTitulo = MeasureText(titulo, 80);
            DrawText(titulo, larguraTela/2 - larguraTitulo/2, 100, 80, BLACK);

            // Funções das teclas centralizadas
            const char* textoEspaco = "APERTE \"ESPACO\" PARA COMECAR";
            int larguraEspaco = MeasureText(textoEspaco, 30);
            DrawText(textoEspaco, larguraTela/2 - larguraEspaco/2, 250, 30, BLACK);

            const char* textoB = "APERTE \"B\" PARA VER SUA MELHOR PONTUACAO";
            int larguraB = MeasureText(textoB, 30);
            DrawText(textoB, larguraTela/2 - larguraB/2, 300, 30, BLACK);

            EndDrawing();

            if (IsKeyPressed(KEY_SPACE)) {
                jogoIniciado = true;
            }
            if(IsKeyPressed(KEY_B)) {
                mostrandoRecorde = true;
                jogoIniciado = false; 
            }
            continue;
        }

        // Tela de recorde
        if (mostrandoRecorde) {
            BeginDrawing();
            // Desenha a imagem de fundo
                DrawTexturePro(
                    recordefundo,
                    (Rectangle){0, 0, recordefundo.width, recordefundo.height},
                    (Rectangle){0, 0, larguraTela, alturaTela},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );

            ClearBackground(RAYWHITE);
            DrawText("Recorde:", larguraTela / 2 - MeasureText("Recorde:", 20) / 2, alturaTela / 2 - 40, 20, BLACK);
            char textoRecorde[64];
            sprintf(textoRecorde, "%06i", recorde);
            DrawText(textoRecorde, larguraTela / 2 - MeasureText(textoRecorde, 30) / 2, alturaTela / 2, 30, RED);
            DrawText("Pressione R para voltar ao jogo", larguraTela / 2 - MeasureText("Pressione R para voltar ao jogo", 20) / 2, alturaTela / 2 + 40, 20, DARKGRAY);
            EndDrawing();

            if (IsKeyPressed(KEY_R)) {
                mostrandoRecorde = false;
            }
            continue;
        }

        // Atualizar sempre os valores de rex
        rex.x = jogador.x + reduzirColisaoTrex;
        rex.y = jogador.y + reduzirColisaoTrex;
        rex.width = texturaRex.width * escalaRex - 2 * reduzirColisaoTrex;
        rex.height = texturaRex.height * escalaRex - 2 * reduzirColisaoTrex;

        // Atualização do tempo de jogo
        if (!jogoAcabou) {
            tempoJogo -= GetFrameTime();
            if (tempoJogo <= 0) {
                jogoAcabou = true;
                tempoJogo = 0;
            }
        }

        // Atualiza o recorde se necessário (logo após detectar o fim do jogo)
        if (jogoAcabou && pontuacao > recorde) {
            recorde = pontuacao;
            SalvarRecorde(recorde);
        }

        // Fora do if (!jogoAcabou), detecte a transição e pare a trilha:
        if (jogoAcabou && !MusicaParada) {
            StopMusicStream(musicaFundo);
            MusicaParada = true;
        }

        if (!jogoAcabou) {

            // Atualização do jogador (pulo)
            if (IsKeyPressed(KEY_SPACE) && !pulando) {
                velocidadeY = forcaPulo;
                pulando = true;
            }
            jogador.y += velocidadeY;
            velocidadeY += gravidade;
            if (jogador.y >= nivelChao - texturaRex.height - deslocamentoRex) {
                jogador.y = nivelChao - texturaRex.height - deslocamentoRex;
                velocidadeY = 0;
                pulando = false;
            }

            // Geração de obstáculos
            tempoDesdeUltimoObstaculo += GetFrameTime();
            if (tempoDesdeUltimoObstaculo >= tempoGerarObstaculo) {
                for (int i = 0; i < 5; i++) {
                    if (!obstaculos[i].ativo) {
                        int tipoObstaculo = GetRandomValue(0, 1);
                        if (tipoObstaculo == 0) {
                            obstaculos[i].tipo = OBSTACULO_TIPO_CHAO;
                            float obsY = 547;
                            IniciarObstaculo(&obstaculos[i], (float)larguraTela, obsY, (float)texturaObstaculoChao.width * 0.18f, (float)texturaObstaculoChao.height * 0.18f);
                        } else {
                            obstaculos[i].tipo = OBSTACULO_TIPO_CEU;
                            float obsY = 470;
                            IniciarObstaculo(&obstaculos[i], (float)larguraTela, obsY, (float)texturaObstaculoCeu.width * 0.33f, (float)texturaObstaculoCeu.height * 0.33f);
                        }
                        tempoDesdeUltimoObstaculo = 0;
                        velocidadeObstaculo += 0.2f;
                        break;
                    }
                }
            }

            // Atualização dos obstáculos
            for (int i = 0; i < 5; i++) {
                if (obstaculos[i].ativo) {
                    obstaculos[i].ret.x -= velocidadeObstaculo;
                    if (obstaculos[i].ret.x + obstaculos[i].ret.width < 0) {
                        obstaculos[i].ativo = false;
                    }
                }
            }

            for (int i = 0; i < 5; i++) {
                if (obstaculos[i].ativo && !obstaculos[i].pontuado) {
                    Rectangle retObs;
                    if (obstaculos[i].tipo == OBSTACULO_TIPO_CHAO) {
                        retObs = (Rectangle){
                            obstaculos[i].ret.x + reduzirColisaoChao,
                            obstaculos[i].ret.y + reduzirColisaoChao,
                            texturaObstaculoChao.width * escalaChao - 2 * reduzirColisaoChao,
                            texturaObstaculoChao.height * escalaChao - 2 * reduzirColisaoChao
                        };
                    } else {
                        retObs = (Rectangle){
                            obstaculos[i].ret.x + reduzirColisaoCeu,
                            obstaculos[i].ret.y + reduzirColisaoCeu,
                            texturaObstaculoCeu.width * escalaCeu - 2 * reduzirColisaoCeu,
                            texturaObstaculoCeu.height * escalaCeu - 2 * reduzirColisaoCeu
                        };
                    }

                    // Se colidir, perde pontos e marca como já processado
                    if (CheckCollisionRecs(rex, retObs)) {
                        pontuacao -= 50;
                        obstaculos[i].pontuado = true;
                        PlaySound(somColisao);
                    }
                    // Se passar, ganha pontos e marca como já processado
                    else if ((obstaculos[i].ret.x + obstaculos[i].ret.width / 8) < (jogador.x + reduzirColisaoTrex)) {
                        pontuacao += 100;
                        obstaculos[i].pontuado = true;
                        PlaySound(somPonto);
                    }
                }
            }
        }

            // Reiniciar ao apertar R
                if (IsKeyPressed(KEY_R)) {
                    jogador.y = nivelChao - texturaRex.height - deslocamentoRex;
                    velocidadeY = 0;
                    pulando = false;
                    pontuacao = 0;
                    for (int i = 0; i < 5; i++) {
                        obstaculos[i].ativo = false;
                        obstaculos[i].pontuado = false;
                    }
                    obstaculosAtivos = 0;
                    tempoDesdeUltimoObstaculo = 0;
                    velocidadeObstaculo = 6.0f;
                    tempoJogo = 120.0f; // Reinicia o tempo de jogo
                    jogoAcabou = false; // Reseta o estado do jogo

                    MusicaParada = false; // Reseta o estado da música
                    SeekMusicStream(musicaFundo, 0.0f); // Reinicia a música
                    PlayMusicStream(musicaFundo); // Reproduz a música novamente
            }

        // Desenho
        BeginDrawing();
        DrawTexturePro(
            fundo,
            (Rectangle){0, 0, fundo.width, fundo.height},
            (Rectangle){0, 0, larguraTela, alturaTela},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        // Chão
        DrawRectangle(0, nivelChao, larguraTela, alturaTela - nivelChao, BEIGE);

        // Jogador (T-Rex)
        DrawTextureEx(
            texturaRex,
            (Vector2){ jogador.x, jogador.y },
            0.0f,
            escalaRex,
            WHITE
        );

        // Obstáculos
        for (int i = 0; i < 5; i++) {
            if (obstaculos[i].ativo) {
                if (obstaculos[i].tipo == OBSTACULO_TIPO_CHAO) {
                    DrawTextureEx(
                        texturaObstaculoChao,
                        (Vector2){ obstaculos[i].ret.x, obstaculos[i].ret.y },
                        0.0f,
                        escalaChao,
                        WHITE
                    );
                } else {
                    DrawTextureEx(
                        texturaObstaculoCeu,
                        (Vector2){ obstaculos[i].ret.x, obstaculos[i].ret.y },
                        0.0f,
                        escalaCeu,
                        WHITE
                    );
                }

                /*
                // Desenha retângulo de colisão do obstáculo
                Rectangle retObs;
                if (obstaculos[i].tipo == OBSTACULO_TIPO_CHAO) {
                    retObs = (Rectangle){
                        obstaculos[i].ret.x + reduzirColisaoChao,
                        obstaculos[i].ret.y + reduzirColisaoChao,
                        texturaObstaculoChao.width * escalaChao - 2 * reduzirColisaoChao,
                        texturaObstaculoChao.height * escalaChao - 2 * reduzirColisaoChao
                    };
                } else {
                    retObs = (Rectangle){
                        obstaculos[i].ret.x + reduzirColisaoCeu,
                        obstaculos[i].ret.y + reduzirColisaoCeu,
                        texturaObstaculoCeu.width * escalaCeu - 2 * reduzirColisaoCeu,
                        texturaObstaculoCeu.height * escalaCeu - 2 * reduzirColisaoCeu
                    };
                }
                DrawRectangleLines(retObs.x, retObs.y, retObs.width, retObs.height, BLUE);
                */
            } 
        } 

        /*
        // Desenha retângulo de colisão do T-Rex
        DrawRectangleLines(
            rex.x,
            rex.y,
            rex.width,
            rex.height,
            RED
        );
        */
       
        // Desenha pontuação
        DrawText(TextFormat("Pontuação: %06i", pontuacao), 10, 10, 20, BLACK);
        DrawText(TextFormat("Tempo: %02d", (int)tempoJogo), 10, 40, 20, BLACK);

        if (jogoAcabou) {
            int centroX = larguraTela / 2;

            const char* textoFim = "FIM DE JOGO!";
            int larguraFim = MeasureText(textoFim, 40);
            DrawText(textoFim, centroX - larguraFim/2, alturaTela/2 - 40, 40, RED);

            char textoPontuacao[64];
            sprintf(textoPontuacao, "Pontuação final: %06i", pontuacao);
            int larguraPontuacao = MeasureText(textoPontuacao, 30);
            DrawText(textoPontuacao, centroX - larguraPontuacao/2, alturaTela/2 + 10, 30, BLACK);

            const char* textoReiniciar = "Pressione R para reiniciar";
            int larguraReiniciar = MeasureText(textoReiniciar, 20);
            DrawText(textoReiniciar, centroX - larguraReiniciar/2, alturaTela/2 + 50, 20, DARKGRAY);

            const char* textoRecorde = "Pressione B para ver o recorde";
            int larguraRecorde = MeasureText(textoRecorde, 20);
            DrawText(textoRecorde, centroX - larguraRecorde/2, alturaTela/2 + 80, 20, DARKGRAY);

            if (IsKeyPressed(KEY_B)) {
                mostrandoRecorde = true;
                jogoIniciado = false;
            }        
        }
        EndDrawing();
    }

    // Libera recursos e fecha janela
    UnloadTexture(fundo);
    UnloadTexture(texturaRex);
    UnloadTexture(texturaObstaculoChao);
    UnloadTexture(texturaObstaculoCeu);
    UnloadSound(somPonto);
    UnloadSound(somColisao);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}