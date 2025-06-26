#define _USE_MATH_DEFINES // Necessário para M_PI no Windows com alguns compiladores
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <cmath>

// ----------------------------------------------------------------------
// ESTRUTURAS E VARIÁVEIS GLOBAIS
// ----------------------------------------------------------------------

struct GameObject {
    float x, y;
    float width, height;
    float r, g, b;
    float speed_x, speed_y;
    float min_pos, max_pos;
};

struct Point {
    float x, y;
};

struct Fan {
    float x, y;
    float blade_length;
    float blade_width;
    float rotation_angle;
    float rotation_speed;
    float r, g, b;
};

struct KeyState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
} keyStates;

GameObject player;
std::vector<GameObject> obstacles;
Fan fan;
int lives = 3;
bool gameOver = false;
bool gameWon = false;
int colorScheme = 0;
const float PLAYER_SPEED = 7.0f;

int screenWidth;
int screenHeight;

// --- NOVO: Variáveis para a cor do texto ---
float textColorR, textColorG, textColorB;

// ----------------------------------------------------------------------
// DECLARAÇÕES DE FUNÇÕES
// ----------------------------------------------------------------------
void specialKeysUp(int key, int x, int y);
bool checkFanCollision();

// ... Funções auxiliares ...
void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

// --- ALTERADO: A função agora também define a cor do texto ---
void applyColors() {
    if (colorScheme == 0) {
        // Esquema Claro (Fundo Cinza Claro)
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        textColorR = 0.05f; textColorG = 0.05f; textColorB = 0.05f; // Texto escuro
        player.r = 0.8f; player.g = 0.2f; player.b = 0.2f;
        for (size_t i = 0; i < obstacles.size(); ++i) { obstacles[i].r = 0.2f; obstacles[i].g = 0.2f; obstacles[i].b = 0.8f; }
        fan.r = 0.3f; fan.g = 0.3f; fan.b = 0.3f;
    }
    else if (colorScheme == 1) {
        // Esquema Escuro (Fundo Cinza Escuro)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        textColorR = 0.9f; textColorG = 0.9f; textColorB = 0.9f; // Texto claro
        player.r = 0.1f; player.g = 0.9f; player.b = 0.1f;
        for (size_t i = 0; i < obstacles.size(); ++i) { obstacles[i].r = 0.9f; obstacles[i].g = 0.1f; obstacles[i].b = 0.5f; }
        fan.r = 0.8f; fan.g = 0.8f; fan.b = 0.8f;
    }
    else {
        // Esquema Sépia (Fundo Creme)
        glClearColor(0.9f, 0.85f, 0.7f, 1.0f);
        textColorR = 0.2f; textColorG = 0.15f; textColorB = 0.1f; // Texto marrom escuro
        player.r = 0.2f; player.g = 0.2f; player.b = 0.7f;
        for (size_t i = 0; i < obstacles.size(); ++i) { obstacles[i].r = 0.5f; obstacles[i].g = 0.35f; obstacles[i].b = 0.05f; }
        fan.r = 0.2f; fan.g = 0.0f; fan.b = 0.2f;
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

float sign(Point p1, Point p2, Point p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool isPointInTriangle(Point pt, Point v1, Point v2, Point v3) {
    float d1, d2, d3;
    bool has_neg, has_pos;
    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);
    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(has_neg && has_pos);
}

bool checkFanCollision() {
    Point playerCorners[4] = {
        {player.x, player.y},
        {player.x + player.width, player.y},
        {player.x + player.width, player.y + player.height},
        {player.x, player.y + player.height}
    };

    for (int i = 0; i < 3; ++i) {
        float current_angle_deg = fan.rotation_angle + (i * 120.0f);
        float current_angle_rad = current_angle_deg * M_PI / 180.0f;

        Point blade_local_verts[3] = {
            {0, 0},
            {-fan.blade_width, fan.blade_length},
            {fan.blade_width, fan.blade_length}
        };

        Point blade_world_verts[3];
        for (int j = 0; j < 3; ++j) {
            float lx = blade_local_verts[j].x;
            float ly = blade_local_verts[j].y;
            blade_world_verts[j].x = (lx * cos(current_angle_rad) - ly * sin(current_angle_rad)) + fan.x;
            blade_world_verts[j].y = (lx * sin(current_angle_rad) + ly * cos(current_angle_rad)) + fan.y;
        }
        
        for (int k = 0; k < 4; ++k) {
            if (isPointInTriangle(playerCorners[k], blade_world_verts[0], blade_world_verts[1], blade_world_verts[2])) {
                return true;
            }
        }
    }
    return false;
}

void init() {
    screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    player = {50, (float)screenHeight / 2, 25, 25, 1, 0, 0, 0, 0, 0, 0};
    
    fan.x = screenWidth / 2.0f;
    fan.y = screenHeight / 2.0f;
    fan.blade_length = 400.0f;
    fan.blade_width = 50.0f;
    fan.rotation_angle = 0.0f;
    fan.rotation_speed = 1.0f;

    obstacles.clear();
    
    obstacles.push_back({(float)screenWidth * 0.2f, (float)screenHeight * 0.5f, 40, (float)screenHeight * 0.5f});
    obstacles.push_back({(float)screenWidth * 0.3f, 0, 40, (float)screenHeight * 0.4f});
    obstacles.push_back({(float)screenWidth * 0.75f, 0, 40, (float)screenHeight * 0.7f});
    obstacles.push_back({(float)screenWidth * 0.85f, (float)screenHeight * 0.3f, 40, (float)screenHeight * 0.7f});
    
    obstacles.push_back({(float)screenWidth * 0.45f, 0.0f, 40, 200.0f, 1,1,1, 0, 2.0f, 0.0f, screenHeight - 200.0f});
    obstacles.push_back({screenWidth * 0.55f, (float)screenHeight * 0.7f, 150, 40, 1,1,1, 5.0f, 0, screenWidth * 0.55f, screenWidth * 0.75f});
    obstacles.push_back({screenWidth * 0.55f, (float)screenHeight * 0.5f, 150, 40, 1,1,1, 5.0f, 0, screenWidth * 0.55f, screenWidth * 0.75f});
    obstacles.push_back({screenWidth * 0.55f, (float)screenHeight * 0.3f, 150, 40, 1,1,1, 5.0f, 0, screenWidth * 0.55f, screenWidth * 0.75f});
    obstacles.push_back({(float)screenWidth * 0.65f, screenHeight - 40.0f, 150, 40, 1,1,1, 0, -3.5f, screenHeight * 0.3f, screenHeight - 40.0f});

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

    for (size_t i = 0; i < obstacles.size(); ++i) {
        glColor3f(obstacles[i].r, obstacles[i].g, obstacles[i].b);
        glRectf(obstacles[i].x, obstacles[i].y, obstacles[i].x + obstacles[i].width, obstacles[i].y + obstacles[i].height);
    }

    glPushMatrix();
    glTranslatef(fan.x, fan.y, 0.0f);
    glRotatef(fan.rotation_angle, 0.0f, 0.0f, 1.0f);
    glColor3f(fan.r, fan.g, fan.b);
    for (int i = 0; i < 3; ++i) {
        glBegin(GL_TRIANGLES);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(-fan.blade_width, fan.blade_length);
            glVertex2f(fan.blade_width, fan.blade_length);
        glEnd();
        glRotatef(120.0f, 0.0f, 0.0f, 1.0f);
    }
    glPopMatrix();
    
    if (!gameWon) {
        glColor3f(player.r, player.g, player.b);
        glRectf(player.x, player.y, player.x + player.width, player.y + player.height);
    }

    // --- ALTERADO: Usa as variáveis de cor de texto dinâmicas ---
    glColor3f(textColorR, textColorG, textColorB);
    char lifeText[20];
    sprintf(lifeText, "Vidas: %d", lives);
    drawText(20, screenHeight - 40, lifeText);
    drawText(screenWidth - 120, screenHeight - 60, "META ->");

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

void update(int value) {
    if (!gameOver && !gameWon) {
        fan.rotation_angle += fan.rotation_speed;
        if (fan.rotation_angle >= 360.0f) {
            fan.rotation_angle -= 360.0f;
        }

        player.speed_x = 0;
        player.speed_y = 0;
        if (keyStates.up)    player.speed_y += PLAYER_SPEED;
        if (keyStates.down)  player.speed_y -= PLAYER_SPEED;
        if (keyStates.left)  player.speed_x -= PLAYER_SPEED;
        if (keyStates.right) player.speed_x += PLAYER_SPEED;

        if (player.speed_x != 0 && player.speed_y != 0) {
            float magnitude = sqrt(player.speed_x * player.speed_x + player.speed_y * player.speed_y);
            player.speed_x = (player.speed_x / magnitude) * PLAYER_SPEED;
            player.speed_y = (player.speed_y / magnitude) * PLAYER_SPEED;
        }

        player.x += player.speed_x;
        player.y += player.speed_y;

        if (player.y < 0) player.y = 0;
        if (player.y + player.height > screenHeight) player.y = screenHeight - player.height;
        if (player.x < 0) player.x = 0;
        
        if (player.x + player.width > screenWidth) {
            gameWon = true;
        }

        for (size_t i = 0; i < obstacles.size(); ++i) {
            if (obstacles[i].speed_y != 0) {
                obstacles[i].y += obstacles[i].speed_y;
                if (obstacles[i].y < obstacles[i].min_pos || obstacles[i].y > obstacles[i].max_pos) {
                    obstacles[i].speed_y *= -1;
                }
            }
            if (obstacles[i].speed_x != 0) {
                obstacles[i].x += obstacles[i].speed_x;
                if (obstacles[i].x < obstacles[i].min_pos || obstacles[i].x > obstacles[i].max_pos) {
                    obstacles[i].speed_x *= -1;
                }
            }
        }

        if (checkCollision() || checkFanCollision()) {
            lives--;
            player.x = 50;
            player.y = screenHeight / 2;
            if (lives <= 0) {
                gameOver = true;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void reshape(int w, int h) {
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
}

void specialKeys(int key, int x, int y) {
    if (gameOver || gameWon) return;
    switch (key) {
        case GLUT_KEY_UP:    keyStates.up = true; break;
        case GLUT_KEY_DOWN:  keyStates.down = true; break;
        case GLUT_KEY_LEFT:  keyStates.left = true; break;
        case GLUT_KEY_RIGHT: keyStates.right = true; break;
    }
}

void specialKeysUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    keyStates.up = false; break;
        case GLUT_KEY_DOWN:  keyStates.down = false; break;
        case GLUT_KEY_LEFT:  keyStates.left = false; break;
        case GLUT_KEY_RIGHT: keyStates.right = false; break;
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
    if ((key == 'r' || key == 'R') && (gameOver || gameWon)) {
        resetGame();
        init();
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        colorScheme = (colorScheme + 1) % 3;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Desafio de Obstaculos v2 - CG 2025.1");
    glutFullScreen();
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutTimerFunc(16, update, 0);
    glutSpecialUpFunc(specialKeysUp);
    glutMainLoop();
    return 0;
}