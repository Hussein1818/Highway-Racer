#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <GL/glut.h>

using namespace std;

// --- Structs ---
struct Obstacle {             // هيكل يمثل العقبة
    float x, y;               // الإحداثيات
    float width, height;      // الأبعاد
    bool active;              // حالة النشاط
};

// --- Global Variables ---
float carY = 0.0f;            // موقع السيارة الرأسي
float carSpeed = 0.04f;       // سرعة حركة السيارة
float roadSpeed = 0.02f;      // سرعة الطريق
float roadOffset = 0.0f;      // إزاحة خطوط الطريق
int score = 0;                // النتيجة الحالية
int highScore = 0;            // أعلى نتيجة
bool gameOver = false;        // حالة نهاية اللعبة
bool isPaused = false;        // حالة التوقف المؤقت
bool gameStarted = false;     // هل بدأت اللعبة أم لا

// --- Countdown Variables ---
bool inCountdown = false;     // هل نحن في وضع العد التنازلي؟
int countdownValue = 3;       // قيمة العد التنازلي

// حالة الأزرار للحركة الناعمة
bool keyUp = false;
bool keyDown = false;

vector<Obstacle> obstacles;   // قائمة العقبات

// --- 1: Game Setup ---
void initGame() {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // لون الخلفية (أسفلت غامق)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// دالة المؤقت للعد التنازلي
void countdownTimer(int value) {
    if (countdownValue > 1) {
        countdownValue--;
        glutTimerFunc(1000, countdownTimer, 0);
    }
    else {
        inCountdown = false; // انتهاء العد التنازلي وبدء اللعب
    }
    glutPostRedisplay();
}

// --- 2: Draw the Road ---
void drawRoad() {
    glColor3f(0.0f, 0.6f, 0.0f); // العشب الأخضر
    glRectf(-1.0f, 0.8f, 1.0f, 1.0f);
    glRectf(-1.0f, -1.0f, 1.0f, -0.8f);

    glColor3f(1.0f, 1.0f, 1.0f); // خطوط الطريق البيضاء
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    for (float i = -1.0f; i < 1.0f; i += 0.4f) {
        glVertex2f(i - roadOffset, 0.25f);
        glVertex2f(i + 0.2f - roadOffset, 0.25f);
        glVertex2f(i - roadOffset, -0.25f);
        glVertex2f(i + 0.2f - roadOffset, -0.25f);
    }
    glEnd();
}

// --- 3: Draw the Car ---
void drawCar() {
    float x = -0.7f;
    float y = carY;

    glColor3f(1.0f, 0.0f, 0.0f); // جسم السيارة أحمر زاهي
    glRectf(x - 0.1f, y - 0.05f, x + 0.1f, y + 0.05f);
    glColor3f(0.8f, 0.0f, 0.0f);
    glRectf(x - 0.05f, y + 0.05f, x + 0.05f, y + 0.1f);
    glColor3f(0.0f, 0.0f, 0.0f); // الإطارات
    glRectf(x - 0.08f, y - 0.08f, x - 0.04f, y - 0.05f);
    glRectf(x + 0.04f, y - 0.08f, x + 0.08f, y - 0.05f);
    glColor3f(0.5f, 0.9f, 1.0f); // الزجاج
    glRectf(x + 0.01f, y + 0.06f, x + 0.04f, y + 0.09f);
    glColor3f(1.0f, 1.0f, 0.0f); // الأنوار
    glRectf(x + 0.1f, y, x + 0.11f, y + 0.03f);
}

// --- 4: Draw Obstacles (Modified with X shape) ---
void drawObstacles() {
    for (const auto& obs : obstacles) {
        if (!obs.active) continue;

        // رسم المربع البني
        glColor3f(0.6f, 0.4f, 0.2f);
        glRectf(obs.x, obs.y, obs.x + obs.width, obs.y + obs.height);

        // رسم حرف X داخل المربع كما في الصورة المطلوبة
        glColor3f(0.4f, 0.2f, 0.1f); // بني غامق للـ X
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(obs.x, obs.y);
        glVertex2f(obs.x + obs.width, obs.y + obs.height);
        glVertex2f(obs.x, obs.y + obs.height);
        glVertex2f(obs.x + obs.width, obs.y);
        glEnd();

        // إطار خارجي رفيع لزيادة الوضوح
        glColor3f(0.3f, 0.15f, 0.05f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(obs.x, obs.y);
        glVertex2f(obs.x + obs.width, obs.y);
        glVertex2f(obs.x + obs.width, obs.y + obs.height);
        glVertex2f(obs.x, obs.y + obs.height);
        glEnd();
    }
}

// --- 5: Update Mechanics ---
void updateMechanics() {
    if (gameOver || isPaused || inCountdown) return;

    roadOffset += roadSpeed;
    if (roadOffset > 0.4f) roadOffset = 0.0f;

    if (rand() % 50 == 0) {
        Obstacle o;
        o.x = 1.2f;
        o.y = (rand() % 140 - 70) / 100.0f;
        o.width = 0.15f; o.height = 0.15f;
        o.active = true;
        obstacles.push_back(o);
    }

    for (int i = 0; i < (int)obstacles.size(); i++) {
        obstacles[i].x -= roadSpeed;
        if (obstacles[i].x < -1.2f) {
            obstacles.erase(obstacles.begin() + i);
            score++;
            if (score > highScore) highScore = score;
            if (score % 10 == 0) roadSpeed += 0.005f;
            i--;
        }
    }
}

// --- 6: Collision Detection ---
void checkCollision() {
    if (gameOver || isPaused || inCountdown) return;
    float carLeft = -0.8f, carRight = -0.6f;
    float carBottom = carY - 0.05f, carTop = carY + 0.1f;

    for (const auto& obs : obstacles) {
        bool collisionX = carRight >= obs.x && carLeft <= obs.x + obs.width;
        bool collisionY = carTop >= obs.y && carBottom <= obs.y + obs.height;
        if (collisionX && collisionY) gameOver = true;
    }
}

// --- 7: Render Scene & UI ---
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (!gameStarted) {
        // شاشة البداية (بنفس التنسيق الزاهي)
        glColor3f(1.0f, 1.0f, 0.0f);
        glRasterPos2f(-0.35f, 0.65f);
        string title = "HIGHWAY RACER PRO";
        for (char c : title) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(-0.25f, 0.52f);
        string hs = "HIGH SCORE: " + to_string(highScore);
        for (char c : hs) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glColor3f(1.0f, 0.4f, 0.4f);
        glRasterPos2f(-0.20f, 0.35f);
        string ruleTitle = "[ GAME RULES ]";
        for (char c : ruleTitle) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glColor3f(0.9f, 0.9f, 0.9f);
        glRasterPos2f(-0.75f, 0.25f);
        string r1 = ">> Avoid brown 'X' blocks. Collision = Game Over!";
        for (char c : r1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.75f, 0.15f);
        string r2 = ">> Each obstacle cleared adds +1 to your SCORE.";
        for (char c : r2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glColor3f(0.0f, 1.0f, 1.0f);
        glRasterPos2f(-0.20f, -0.05f);
        string ctrlTitle = "[ CONTROLS ]";
        for (char c : ctrlTitle) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(-0.45f, -0.15f);
        string inst1 = "- ARROWS UP/DOWN : Move the Car";
        for (char c : inst1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.45f, -0.25f);
        string inst2 = "- SPACE BAR      : Pause / Resume";
        for (char c : inst2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glColor3f(0.0f, 1.0f, 0.0f);
        glRasterPos2f(-0.30f, -0.55f);
        string startMsg = "PRESS ENTER TO START";
        for (char c : startMsg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glutSwapBuffers();
        return;
    }

    drawRoad();
    drawObstacles();
    drawCar();

    // عرض النتيجة
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.95f, 0.88f);
    string txt = gameOver ? "CRASHED! Score: " + to_string(score) + " - Press 'R' to Restart" :
        "SCORE: " + to_string(score) + "  |  HIGH SCORE: " + to_string(highScore);

    for (char c : txt) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    // عرض المؤقت التنازلي
    if (inCountdown) {
        glColor3f(1.0f, 1.0f, 0.0f); // أصفر زاهي
        glRasterPos2f(-0.05f, 0.0f);
        string cMsg = to_string(countdownValue);
        for (char c : cMsg) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }

    if (isPaused && !gameOver && !inCountdown) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glRasterPos2f(-0.4f, 0.0f);
        string pMsg = "PAUSED - PRESS SPACE TO RESUME";
        for (char c : pMsg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glutSwapBuffers();
}

// --- System Callbacks ---
void gameLoop(int value) {
    if (gameStarted && !gameOver && !isPaused && !inCountdown) {
        if (keyUp && carY < 0.7f) carY += carSpeed;
        if (keyDown && carY > -0.7f) carY -= carSpeed;
        updateMechanics();
        checkCollision();
    }
    glutPostRedisplay();
    glutTimerFunc(16, gameLoop, 0);
}

void normalKeys(unsigned char key, int x, int y) {
    if (key == 13 && !gameStarted) { // Enter
        gameStarted = true;
        inCountdown = true;
        countdownValue = 3;
        glutTimerFunc(1000, countdownTimer, 0);
    }
    else if (key == ' ' && gameStarted && !gameOver) { // Space
        if (isPaused) { // Resume with countdown
            isPaused = false;
            inCountdown = true;
            countdownValue = 3;
            glutTimerFunc(1000, countdownTimer, 0);
        }
        else {
            isPaused = true;
        }
    }
    else if ((key == 'r' || key == 'R') && gameOver) { // Restart
        gameOver = false; isPaused = false; score = 0; roadSpeed = 0.02f;
        obstacles.clear(); carY = 0.0f;
        inCountdown = true; countdownValue = 3;
        glutTimerFunc(1000, countdownTimer, 0);
    }
    else if (key == 27) exit(0);
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) keyUp = true;
    if (key == GLUT_KEY_DOWN) keyDown = true;
}

void specialKeysUp(int key, int x, int y) {
    if (key == GLUT_KEY_UP) keyUp = false;
    if (key == GLUT_KEY_DOWN) keyDown = false;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Highway Racer - Pro Project");
    initGame();
    glutDisplayFunc(renderScene);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutKeyboardFunc(normalKeys);
    glutTimerFunc(0, gameLoop, 0);
    glutMainLoop();
    return 0;
}