#define GLM_ENABLE_EXPERIMENTAL
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<glm/glm.hpp>
#include<vector>
#include<camera.h>
#include<shader.h>
#include<MarchingCubesTables.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
// 宣告
namespace cube {
    extern std::array<short, 256> const kCubeEdges;
    extern std::array<std::array<short, 16>, 256> const kCubeTriangles;
}
extern std::array<std::array<short, 16>, 256> const kCubeTriangles;
extern int triTable[256][16];
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
// camera
Camera camera(glm::vec3(0.0f, 5.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
GLFWwindow* window;
bool firstMouse = true;
Shader lightingShader;
Shader volumeShader;
float deltaTime = 0.05f;
int isoValue = 180;
bool needUpdate = false;
// 在全域變數區新增
int isoValue2 = 80; // 假設第二個值是 120
unsigned int isoVAO2, isoVBO2;


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};
glm::vec3 getNormal(unsigned char* data, int x, int y, int z, int w, int h, int d) {
    float nx, ny, nz;

    // X 方向梯度 (Central Difference)
    if (x > 0 && x < w - 1)
        nx = (float)data[(z * h * w + y * w + (x + 1)) * 4 + 3] - (float)data[(z * h * w + y * w + (x - 1)) * 4 + 3];
    else
        nx = 0.0f;

    // Y 方向梯度
    if (y > 0 && y < h - 1)
        ny = (float)data[(z * h * w + (y + 1) * w + x) * 4 + 3] - (float)data[(z * h * w + (y - 1) * w + x) * 4 + 3];
    else
        ny = 0.0f;

    // Z 方向梯度
    if (z > 0 && z < d - 1)
        nz = (float)data[((z + 1) * h * w + y * w + x) * 4 + 3] - (float)data[((z - 1) * h * w + y * w + x) * 4 + 3];
    else
        nz = 0.0f;

    // 注意：在 OpenGL 中通常取負梯度作為法線，並單位化
    return glm::normalize(glm::vec3(-nx, -ny, -nz));
}
unsigned char* loadRawFile(const std::string& filename, int width, int height, int depth) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "cannot open raw\n";
        return nullptr;
    }

    size_t dataSize = width * height * depth;
    unsigned char* data = new unsigned char[dataSize];

    file.read(reinterpret_cast<char*>(data), dataSize);


    size_t new_dataSize = width * height * depth * 4;
    unsigned char* new_data = new unsigned char[new_dataSize];
    for (int i = 0; i < width * height * depth; i++) {

        new_data[4 * i + 3] = data[i];
        new_data[4 * i] = 0;
        new_data[4 * i + 1] = 0;
        new_data[4 * i + 2] = 0;
    }

    return new_data;
}
// 線性插值計算交點位置
glm::vec3 interpolate(glm::vec3 p1, glm::vec3 p2, float val1, float val2, float iso) {
    if (abs(iso - val1) < 0.00001f) return p1;
    if (abs(iso - val2) < 0.00001f) return p2;
    if (abs(val1 - val2) < 0.00001f) return p1;
    float mu = (iso - val1) / (val2 - val1);
    return p1 + mu * (p2 - p1);
}
void ini() {
    //init and configure
    camera.MovementSpeed = 10.0f;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //glfw window creation
    window = glfwCreateWindow(800, 600, "8888", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //global opengl state
    glEnable(GL_DEPTH_TEST);
    Shader* newshader = new Shader("color.vs", "color.fs");
    lightingShader = *newshader;

    //newshader = new Shader("volume.vs", "volume.fs");
    //volumeShader = *newshader;



}
#include <array>

// 宣告外部 Table
extern std::array<short, 256> kCubeEdges;
extern std::array<std::array<short, 16>, 256> const kCubeTriangles;

std::vector<Vertex> generateIsosurface(unsigned char* data, int w, int h, int d, float isoValue) {
    std::vector<Vertex> vertices;

    // 定義立方體 12 條邊連接的頂點索引 (Marching Cubes 標準)
    const int edgeToVertices[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    // 遍歷每一個 Grid Cell
    for (int z = 0; z < d - 1; z++) {
        for (int y = 0; y < h - 1; y++) {
            for (int x = 0; x < w - 1; x++) {

                // 1. 取得 8 個頂點的座標與數值
                glm::vec3 p[8];
                float val[8];
                // 這裡的 cornerIdx 順序必須嚴格對應 Table 定義
                    int cornerIdx[8][3] = {
                    {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, // 底面 (z=0) 四個點，逆時針
                    {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}  // 頂面 (z=1) 四個點，逆時針
                    };

                    for (int i = 0; i < 8; i++) {
                        int cx = x + cornerIdx[i][0];
                        int cy = y + cornerIdx[i][1];
                        int cz = z + cornerIdx[i][2];
                        p[i] = glm::vec3((float)cx, (float)cy, (float)cz);

                        // 這裡最危險！必須確保順序與讀取時一致
                        // 根據常用的 RAW 格式 (X 增加最快，接著 Y，最後 Z)
                        size_t index = (size_t)cz * (h * w) + (size_t)cy * w + cx;
                        val[i] = (float)data[index * 4 + 3];
                    }

                // 2. 計算 Table Index (哪些點在表面內)
                int cubeIndex = 0;
                if (val[0] >= isoValue) cubeIndex |= 1;
                if (val[1] >= isoValue) cubeIndex |= 2;
                if (val[2] >= isoValue) cubeIndex |= 4;
                if (val[3] >= isoValue) cubeIndex |= 8;
                if (val[4] >= isoValue) cubeIndex |= 16;
                if (val[5] >= isoValue) cubeIndex |= 32;
                if (val[6] >= isoValue) cubeIndex |= 64;
                if (val[7] >= isoValue) cubeIndex |= 128;

                if (cube::kCubeEdges[cubeIndex] == 0) continue;

                // 注意這裡：使用 cube::kCubeTriangles
                for (int i = 0; cube::kCubeTriangles[cubeIndex][i] != -1; i += 3) {
                    // 在 generateIsosurface 的頂點循環中：
                    for (int j = 0; j < 3; j++) {
                        int edgeIdx = cube::kCubeTriangles[cubeIndex][i + j];
                        int v1_idx = edgeToVertices[edgeIdx][0];
                        int v2_idx = edgeToVertices[edgeIdx][1];

                        // 1. 計算插值後的座標
                        glm::vec3 pos = interpolate(p[v1_idx], p[v2_idx], val[v1_idx], val[v2_idx], isoValue);

                        // 2. 計算兩個端點的梯度法向量
                        glm::vec3 n1 = getNormal(data, (int)p[v1_idx].x, (int)p[v1_idx].y, (int)p[v1_idx].z, w, h, d);
                        glm::vec3 n2 = getNormal(data, (int)p[v2_idx].x, (int)p[v2_idx].y, (int)p[v2_idx].z, w, h, d);

                        // 3. 對法向量也進行線性插值 (或是直接取 pos 處的梯度)
                        float mu = (isoValue - val[v1_idx]) / (val[v2_idx] - val[v1_idx]);
                        glm::vec3 normal = glm::normalize(n1 + mu * (n2 - n1));

                        Vertex v;
                        v.Position = pos;
                        v.Normal = normal; // 現在有正確的法向量了！
                        vertices.push_back(v);
                    }
                }
            }
        }
    }
    return vertices;
}

int main() {
    ini();

    int res_width = 149, res_height = 208, res_depth = 110;
    
    unsigned char* data;
    data = loadRawFile("engine.raw", res_width, res_height, res_depth);
    std::vector<Vertex> isoVertices = generateIsosurface(data, res_width, res_height, res_depth, isoValue);
    std::cout << "Generated Triangles: " << isoVertices.size() / 3 << std::endl;
    // 2. 更新 VAO/VBO
    unsigned int isoVAO, isoVBO;
    glGenVertexArrays(1, &isoVAO);
    glGenBuffers(1, &isoVBO);
    glBindVertexArray(isoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, isoVBO);
    glBufferData(GL_ARRAY_BUFFER, isoVertices.size() * sizeof(Vertex), &isoVertices[0], GL_STATIC_DRAW);

    // 設定 Attribute (Position & Normal)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // --- 在生成第一組之後加入 ---
    std::vector<Vertex> isoVertices2 = generateIsosurface(data, res_width, res_height, res_depth, (float)isoValue2);

    glGenVertexArrays(1, &isoVAO2);
    glGenBuffers(1, &isoVBO2);
    glBindVertexArray(isoVAO2);
    glBindBuffer(GL_ARRAY_BUFFER, isoVBO2);
    glBufferData(GL_ARRAY_BUFFER, isoVertices2.size() * sizeof(Vertex), &isoVertices2[0], GL_STATIC_DRAW);

    // 設定 Attribute (與第一組相同)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    while (!glfwWindowShouldClose(window)) {
        
        // 更新時間與輸入 (如果有 processInput 函數的話)
        float currentFrame = glfwGetTime();
        // deltaTime = currentFrame - lastFrame; ...
        processInput(window);
        if (needUpdate) {
            // 重新計算兩組
            isoVertices = generateIsosurface(data, res_width, res_height, res_depth, (float)isoValue);
            isoVertices2 = generateIsosurface(data, res_width, res_height, res_depth, (float)isoValue2);

            // 更新 VBO 1
            glBindBuffer(GL_ARRAY_BUFFER, isoVBO);
            if (!isoVertices.empty())
                glBufferData(GL_ARRAY_BUFFER, isoVertices.size() * sizeof(Vertex), isoVertices.data(), GL_STATIC_DRAW);

            // 更新 VBO 2
            glBindBuffer(GL_ARRAY_BUFFER, isoVBO2);
            if (!isoVertices2.empty())
                glBufferData(GL_ARRAY_BUFFER, isoVertices2.size() * sizeof(Vertex), isoVertices2.data(), GL_STATIC_DRAW);

            std::cout << "Iso1: " << isoValue << " | Iso2: " << isoValue2 << std::endl;
            needUpdate = false;
        }
        // 清除緩衝
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 更新矩陣
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f); // 遠平面調大一點

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // --- 調整模型位置與大小 ---
        glm::mat4 model = glm::mat4(1.0f);
        // 將模型中心移到原點 (110/2, 208/2, 149/2) 並大幅縮小
        model = glm::scale(model, glm::vec3(0.05f));
        model = glm::translate(model, glm::vec3(-55.0f, -104.0f, -74.5f));
        lightingShader.setMat4("model", model);

        // 記得設定光源 (假設你的 Shader 變數名如下)
        lightingShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        lightingShader.setVec3("lightPos", camera.Position); // 或是固定一個位置
        lightingShader.setVec3("viewPos", camera.Position);

        // 如果你的 shader 需要 objectColor
        // 繪製第一層 (例如：橘紅色)
        lightingShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.3f));
        glBindVertexArray(isoVAO);
        glDrawArrays(GL_TRIANGLES, 0, isoVertices.size());

        // 繪製第二層 (例如：青藍色)
        lightingShader.setVec3("objectColor", glm::vec3(0.3f, 0.8f, 1.0f));
        glBindVertexArray(isoVAO2);
        glDrawArrays(GL_TRIANGLES, 0, isoVertices2.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
    //camera.ProcessMouseMovement(xoffset, 0);
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    static bool upPressed = false;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !upPressed) {
        isoValue += 5;
        if (isoValue > 255) isoValue = 255;
        needUpdate = true; // 設一個 Flag 告訴主迴圈要重算
        upPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) upPressed = false;

    // 偵測按鍵（減少 IsoValue）
    static bool downPressed = false;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !downPressed) {
        isoValue -= 5;
        if (isoValue < 0) isoValue = 0;
        needUpdate = true;
        downPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) downPressed = false;

}