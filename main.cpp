#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <cmath> // Necessário para a função fmod e sqrt

// ----------------------------------------------------------------------
// ESTRUTURAS E VARIÁVEIS GLOBAIS
// ----------------------------------------------------------------------

// Estrutura aprimorada para representar um objeto, incluindo dados de movimento
struct GameObject {
    float x, y;          // Posição do canto inferior esquerdo
    float width, height; // Dimensões
    float r, g, b;       // Cor

    // Propriedades de movimento
    float speed_x, speed_y; // Velocidade nos eixos
    float min_pos, max_pos; // Limites de movimento (em X ou Y)
};

// --- NOVO: Estrutura para representar o ventilador ---
struct Fan {
    float x, y;                // Posição do centro (eixo)
    float blade_length;        // Comprimento das pás
    float rotation_angle;      // Ângulo de rotação atual em graus
    float rotation_speed;      // Velocidade da rotação (graus por quadro)
    float r, g, b;             // Cor das pás
};


//Estrutura para rastrear o estado das teclas ---
struct KeyState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
} keyStates;

// Variáveis de estado do jogo
GameObject player;
std::vector<GameObject> obstacles;
Fan fan; // --- NOVO: Variável global para o nosso ventilador
int lives = 3;
bool gameOver = false;
bool gameWon = false;
int colorScheme = 0;
const float PLAYER_SPEED = 7.0f; //Velocidade constante para o jogador

// Dimensões da tela (serão definidas dinamicamente)
int screenWidth;
int screenHeight;

// ----------------------------------------------------------------------
// DECLARAÇÕES DE FUNÇÕES
// ----------------------------------------------------------------------
void specialKeysUp(int key, int x, int y);

// ----------------------------------------------------------------------
// FUNÇÕES AUXILIARES
// ----------------------------------------------------------------------

void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void applyColors() {
    if (colorScheme == 0) {
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        player.r = 0.8f; player.g = 0.2f; player.b = 0.2f;
        for (size_t i = 1; i < obstacles.size(); ++i) { obstacles[i].r = 0.2f; obstacles[i].g = 0.2f; obstacles[i].b = 0.8f; }
        fan.r = 0.3f; fan.g = 0.3f; fan.b = 0.3f;
    }
    else if (colorScheme == 1) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        player.r = 0.1f; player.g = 0.9f; player.b = 0.1f;
        for (size_t i = 1; i < obstacles.size(); ++i) { obstacles[i].r = 0.9f; obstacles[i].g = 0.1f; obstacles[i].b = 0.5f; }
        fan.r = 0.8f; fan.g = 0.8f; fan.b = 0.8f;
    }
    else {
        glClearColor(0.9f, 0.85f, 0.7f, 1.0f);
        player.r = 0.2f; player.g = 0.6f; player.b = 0.3f;
        for (size_t i = 1; i < obstacles.size(); ++i) { obstacles[i].r = 0.5f; obstacles[i].g = 0.35f; obstacles[i].b = 0.05f; }
        fan.r = 0.2f; fan.g = 0.2f; fan.b = 0.2f;
    }
}

void resetGame() {
    lives = 3;
    gameOver = false;
    gameWon = false;
    player.x = 50;
    player.y = screenHeight / 2;
    player.speed_x = 0;
    player.speed_y = 0;
}

// ----------------------------------------------------------------------
// LÓGICA DO JOGO
// ----------------------------------------------------------------------

bool checkCollision() {
    for (size_t i = 0; i < obstacles.size(); ++i) {
        if (player.x < obstacles[i].x + obstacles[i].width &&
            player.x + player.width > obstacles[i].x &&
            player.y < obstacles[i].y + obstacles[i].height &&
            player.y + player.height > obstacles[i].y)
        {
            return true;
        }
    }
    return false;
}

void init() {
    // Define as dimensões da tela dinamicamente
    screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    // Inicializa o jogador
    player = {50, (float)screenHeight / 2, 25, 25, 1, 0, 0, 0, 0, 0, 0};
    
    // --- NOVO: Inicializa as propriedades do ventilador ---
    fan.x = screenWidth / 2.0f;
    fan.y = screenHeight / 2.0f;
    fan.blade_length = 200.0f;
    fan.rotation_angle = 0.0f;
    fan.rotation_speed = 1.0f; // Ajuste para deixar mais rápido ou lento

    obstacles.clear();
    
    // --- NOVO: Adiciona uma caixa de colisão para o ventilador ---
    // Esta é uma "hitbox" invisível. O jogador colidirá com este quadrado,
    // que engloba toda a área de rotação do ventilador. Será o obstáculo de índice 0.
    float hitbox_size = fan.blade_length * 2;
    obstacles.push_back({
        fan.x - fan.blade_length, // x
        fan.y - fan.blade_length, // y
        hitbox_size,              // width
        hitbox_size,              // height
        0,0,0,0,0,0,0 // O resto não importa, pois é estático e a cor não será usada
    });


    // Define os obstáculos (estáticos e móveis)
    // Obstáculos estáticos
    obstacles.push_back({(float)screenWidth * 0.2f, (float)screenHeight * 0.5f, 40, (float)screenHeight * 0.5f});
    obstacles.push_back({(float)screenWidth * 0.3f, 0, 40, (float)screenHeight * 0.4f});
    obstacles.push_back({(float)screenWidth * 0.75f, 0, 40, (float)screenHeight * 0.7f});
    obstacles.push_back({(float)screenWidth * 0.85f, (float)screenHeight * 0.3f, 40, (float)screenHeight * 0.7f});

    // Obstáculos móveis
    // Obstáculo 1: Movimento Vertical
    float obs1_y_start = 0;
    float obs1_y_end = screenHeight - 200;
    obstacles.push_back({(float)screenWidth * 0.45f, obs1_y_start, 40, 200, 1,1,1, 0, 2.0f, obs1_y_start, obs1_y_end});

    // Obstáculo 2: Movimento Horizontal
    float obs2_x_start = screenWidth * 0.55f;
    float obs2_x_end = screenWidth * 0.7f;
    obstacles.push_back({obs2_x_start, (float)screenHeight * 0.4f, 150, 40, 1,1,1, 2.5f, 0, obs2_x_start, obs2_x_end});
    
    // Obstáculo 3: Movimento Vertical Rápido
    float obs3_y_start = screenHeight * 0.3f;
    float obs3_y_end = screenHeight - 40;
    obstacles.push_back({(float)screenWidth * 0.65f, obs3_y_end, 150, 40, 1,1,1, 0, -3.5f, obs3_y_start, obs3_y_end});

    applyColors();
}

// ----------------------------------------------------------------------
// FUNÇÕES DE CALLBACK DO OPENGL
// ----------------------------------------------------------------------

void display() {
    applyColors();
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- ALTERADO: Começa o loop do 1 para não desenhar a hitbox do ventilador ---
    for (size_t i = 1; i < obstacles.size(); ++i) {
        glColor3f(obstacles[i].r, obstacles[i].g, obstacles[i].b);
        glRectf(obstacles[i].x, obstacles[i].y, obstacles[i].x + obstacles[i].width, obstacles[i].y + obstacles[i].height);
    }

    // --- NOVO: Lógica para desenhar o ventilador ---
    glPushMatrix(); // Salva o estado atual da matriz de transformação

    // 1. Move o sistema de coordenadas para o centro do ventilador
    glTranslatef(fan.x, fan.y, 0.0f);
    // 2. Rotaciona o sistema de coordenadas de acordo com o ângulo atual do ventilador
    glRotatef(fan.rotation_angle, 0.0f, 0.0f, 1.0f); // Rotação em torno do eixo Z

    // Define a cor das pás
    glColor3f(fan.r, fan.g, fan.b);

    // 3. Desenha as 3 pás (triângulos)
    for (int i = 0; i < 3; ++i) {
        glBegin(GL_TRIANGLES);
            // Vértice no centro do ventilador
            glVertex2f(0.0f, 0.0f);
            // Vértices que formam a pá. Desenhamos como se estivesse na vertical.
            glVertex2f(-15.0f, fan.blade_length); // Canto esquerdo da ponta da pá
            glVertex2f(15.0f, fan.blade_length);  // Canto direito da ponta da pá
        glEnd();
        // Rotaciona o sistema em 120 graus para a próxima pá (360 / 3 = 120)
        glRotatef(120.0f, 0.0f, 0.0f, 1.0f);
    }
    
    glPopMatrix(); // Restaura a matriz de transformação original para não afetar outros desenhos


    if (!gameWon) {
        glColor3f(player.r, player.g, player.b);
        glRectf(player.x, player.y, player.x + player.width, player.y + player.height);
    }

    glColor3f(0.05f, 0.05f, 0.05f);
    char lifeText[20];
    sprintf(lifeText, "Vidas: %d", lives);
    drawText(20, screenHeight - 40, lifeText);
    drawText(screenWidth - 200, screenHeight - 40, "META ->");

    if (gameOver) {
        drawText(screenWidth/2 - 50, screenHeight/2, "DERROTA!");
        drawText(screenWidth/2 - 170, screenHeight/2 - 30, "Pressione 'r' para reiniciar ou 'ESC' para sair.");
    }

    if (gameWon) {
        drawText(screenWidth/2 - 50, screenHeight/2, "VITORIA!");
        drawText(screenWidth/2 - 170, screenHeight/2 - 30, "Pressione 'r' para reiniciar ou 'ESC' para sair.");
    }

    glutSwapBuffers();
}

/**
 * @brief Função chamada periodicamente. Usada para toda a lógica de atualização do jogo.
 */
void update(int value) {
    if (!gameOver && !gameWon) {
        
        // --- NOVO: Atualiza o ângulo de rotação do ventilador ---
        fan.rotation_angle += fan.rotation_speed;
        // Reseta o ângulo para evitar que o número cresça indefinidamente
        if (fan.rotation_angle >= 360.0f) {
            fan.rotation_angle -= 360.0f;
        }

        // 1. Redefine a velocidade do jogador a cada quadro
        player.speed_x = 0;
        player.speed_y = 0;

        // 2. Define a direção com base nas teclas pressionadas
        if (keyStates.up)    player.speed_y += PLAYER_SPEED;
        if (keyStates.down)  player.speed_y -= PLAYER_SPEED;
        if (keyStates.left)  player.speed_x -= PLAYER_SPEED;
        if (keyStates.right) player.speed_x += PLAYER_SPEED;

        // 3. Normaliza a velocidade para movimento diagonal
        if (player.speed_x != 0 && player.speed_y != 0) {
            float magnitude = sqrt(player.speed_x * player.speed_x + player.speed_y * player.speed_y);
            player.speed_x = (player.speed_x / magnitude) * PLAYER_SPEED;
            player.speed_y = (player.speed_y / magnitude) * PLAYER_SPEED;
        }

        // 4. Atualiza a posição do jogador
        player.x += player.speed_x;
        player.y += player.speed_y;

        // 5. Garante que o jogador não saia da tela
        if (player.y < 0) player.y = 0;
        if (player.y + player.height > screenHeight) player.y = screenHeight - player.height;
        if (player.x < 0) player.x = 0;
        
        // 6. Verifica se o jogador alcançou a meta
        if (player.x + player.width > screenWidth) {
            gameWon = true;
        }

        // Move cada obstáculo que tem velocidade definida
        // --- ALTERADO: Começa o loop do 1 para não mover a hitbox do ventilador ---
        for (size_t i = 1; i < obstacles.size(); ++i) {
            // Movimento em Y
            if (obstacles[i].speed_y != 0) {
                obstacles[i].y += obstacles[i].speed_y;
                if (obstacles[i].y < obstacles[i].min_pos || obstacles[i].y > obstacles[i].max_pos) {
                    obstacles[i].speed_y *= -1; // Inverte a direção
                }
            }
            // Movimento em X
            if (obstacles[i].speed_x != 0) {
                obstacles[i].x += obstacles[i].speed_x;
                if (obstacles[i].x < obstacles[i].min_pos || obstacles[i].x > obstacles[i].max_pos) {
                    obstacles[i].speed_x *= -1; // Inverte a direção
                }
            }
        }

        // Checagem de colisão após toda a movimentação
        if (checkCollision()) {
            lives--;
            player.x = 50;
            player.y = screenHeight / 2;
            if (lives <= 0) {
                gameOver = true;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Re-chama o update a aprox. 60 FPS
}

// ... (O restante do código: reshape, specialKeys, specialKeysUp, keyboard, mouse e main permanecem os mesmos) ...
void reshape(int w, int h) {
    // Atualiza as dimensões da tela em caso de redimensionamento
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}


//Controle de teclas para os movimentos do player
void specialKeys(int key, int x, int y) {
    if (gameOver || gameWon) return;

    switch (key) {
        case GLUT_KEY_UP:    keyStates.up = true; break;
        case GLUT_KEY_DOWN:  keyStates.down = true; break;
        case GLUT_KEY_LEFT:  keyStates.left = true; break;
        case GLUT_KEY_RIGHT: keyStates.right = true; break;
    }
}

// Chamada quando uma tecla especial é solta
void specialKeysUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    keyStates.up = false; break;
        case GLUT_KEY_DOWN:  keyStates.down = false; break;
        case GLUT_KEY_LEFT:  keyStates.left = false; break;
        case GLUT_KEY_RIGHT: keyStates.right = false; break;
    }
}


void keyboard(unsigned char key, int x, int y) {
    // Tecla 'ESC' para sair do jogo
    if (key == 27) { // 27 é o código ASCII para ESC
        exit(0);
    }
    // Tecla 'r' para reiniciar
    if ((key == 'r' || key == 'R') && (gameOver || gameWon)) {
        resetGame();
        init(); // Re-inicializa os obstáculos para suas posições originais
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        colorScheme = (colorScheme + 1) % 3;
    }
}

// ----------------------------------------------------------------------
// FUNÇÃO PRINCIPAL
// ----------------------------------------------------------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    
    // Pega as dimensões da tela antes de criar a janela
    screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowSize(screenWidth, screenHeight);

    glutCreateWindow("Desafio de Obstaculos v2 - CG 2025.1");
    glutFullScreen(); // Força o modo de tela cheia

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutTimerFunc(16, update, 0); // Inicia a função de update/animação

    glutSpecialUpFunc(specialKeysUp);

    glutMainLoop();
    return 0;
}