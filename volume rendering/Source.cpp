#define GLM_ENABLE_EXPERIMENTAL
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<shader.h>
#include<camera.h>
//#include<stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>
#include<bitset>
#include<glm/gtc/type_ptr.hpp>
#include<iostream>
#include<vector>

struct ball {
	enum Type { SPHERE, CUBE } type;
	glm::vec3 position;  // 球的起始位置
	glm::vec3 velocity;  // 球的移動速度（方向 * 速度）
	int ball_color;
	int cube_size;
	glm::quat rotation = glm::quat(1, 0, 0, 0); // 初始四元數為單位旋轉
	glm::vec3 angular_velocity = glm::vec3(0.0f); // 每秒轉動的軸向速度
};
struct Node { float x, y; };
std::vector<Node> nodesR = { {0.0, 0.0}, {0.5, 1.0}, {1.0, 0.0} }; // 範例
std::vector<Node> nodesG = { {0.0, 0.0}, {0.3, 0.5}, {1.0, 0.0} };
std::vector<Node> nodesB = { {0.0, 0.0}, {0.7, 0.8}, {1.0, 0.0} };
GLuint lenght;
glm::mat4 stair_model;
unsigned int VBO, VAO;
unsigned int VBO2, cubeVAO;
unsigned int stairVBO, stairVAO;
unsigned int lightCubeVAO;
unsigned int floorVAO, floorVBO;
unsigned int gridVAO, gridVBO;

const int Y_SEGMENTS = 50;
const int X_SEGMENTS = 50;
const GLfloat PI = 3.14159265358979323846f;
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
// camera
Camera camera(glm::vec3(0.0f, 5.0f, 5.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
int get_force = 0;
bool get_mouse = true;// 滑鼠控制權
bool borc = false;
bool is_rotating = false;
bool cube_is_rotating = false;
// timing
float deltaTime = 0.05f;
float lastFrame = 0.0f;
float deltaTime_t = 0.05f;
float lastFrame_t = 0.0f;
glm::vec3 robotPos;
float ball_radius = 0.3f;
glm::vec3 T;
glm::vec3 cube_T;
int wall_color;
//bool ball_thrown = false;  // 判斷球是否正在運動
bool attach;
bool lit_switch;
bool lit_switch1;
GLFWwindow* window;
//建立shader
Shader lightingShader;
Shader lightCubeShader;
Shader ballShader;
Shader gridShader;
Shader volumeShader;
Shader uiShader;
float getValueFromNodes(float x, const std::vector<Node>& nodes);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void draw_tf_interface();
void draw_line_from_nodes(const std::vector<Node>& nodes);
void print_vec3(glm::vec3 vector) {
	std::cout << "x: " << vector.x << std::endl;
	std::cout << "y: " << vector.y << std::endl;
	std::cout << "z: " << vector.z << std::endl;
}
void processInput(GLFWwindow* window);
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
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	//global opengl state
	glEnable(GL_DEPTH_TEST);
	Shader* newshader = new Shader("color.vs", "color.fs");
	lightingShader = *newshader;
	newshader = new Shader("light_cube.vs", "light_cube.fs");
	lightCubeShader = *newshader;
	newshader = new Shader("ball.vs", "ball.fs");
	ballShader = *newshader;

	newshader = new Shader("volume.vs", "volume.frag");
	volumeShader = *newshader;
	newshader = new Shader("ui_shader.vs", "ui_shader.frag");
	uiShader = *newshader;



}

int res_width = 110, res_height = 208, res_depth = 149;
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

int main() {
	ini();
	unsigned int texture1;
	//ch2
	float wallvertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};
	std::vector<glm::vec4> tf(256);
	for (int i = 0; i < 256; ++i) {
		float f = i / 255.0f;
		tf[i] = glm::vec4(f); // 灰階轉 RGBA，含透明度
	}
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wallvertices), wallvertices, GL_STATIC_DRAW);

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);



	unsigned char* data;
	//std::vector<unsigned char> data;
	data = new unsigned char[res_width * res_height * res_depth * 4];
	//unsigned char* data = (unsigned char*)malloc(res_width * res_height * res_depth * 4* sizeof(unsigned char));
	data = loadRawFile("engine.raw", res_width, res_height, res_depth);
	//for (int i = 0; i < data.size(); i++) {
	//	//std::bitset<8> bits(data[i]);
	//	std::cout << "data: "<<int(data[i]) << std::endl;
	//}
	// 1. 統計直方圖 (Histogram)
	int histogram[256] = { 0 };
	int totalPixels = res_width * res_height * res_depth;
	for (int i = 0; i < totalPixels; i++) {
		//std::cout << data[i] << std::endl;
		histogram[data[i*4+3]]++;		
	}

	// 2. 計算累積分布函數 (CDF)
	float cdf[256] = { 0 };
	int sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += histogram[i];
		//cdf[i] = (float)sum/totalPixels;
		cdf[i] = (float)sum;
		//std::cout << cdf[i] << std::endl;
	}
	for (int i = 0; i < 256; i++) {
		cdf[i] /= cdf[255];
		//std::cout << cdf[i];
	}
	//3.
	size_t new_dataSize = res_width * res_height * res_depth * 4;
	unsigned char* new_data = new unsigned char[new_dataSize];
	for (int i = 0; i < totalPixels; i++) {
		unsigned char originalVal = data[i*4+3];
		// 使用 CDF 進行 Equalization：將舊值映射到新值 (0-255)
		unsigned char equalizedVal = (unsigned char)(cdf[originalVal] * 255.0f);
		//std::cout << float(equalizedVal)<<std::endl;
		new_data[4 * i + 3] = equalizedVal; // 存入 Alpha
		new_data[4 * i] = 0;
		new_data[4 * i + 1] = 0;
		new_data[4 * i + 2] = 0;
	}

	GLuint volumeTex, transferFuncTex;
	glGenTextures(1, &volumeTex);
	glBindTexture(GL_TEXTURE_3D, volumeTex);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, res_depth, res_height, res_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, new_data);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, volumeTex);

	volumeShader.use();
	volumeShader.setInt("volumeData", 0);
	//glUseProgram(volumeShader.ID);
	//glUniform1i(glGetUniformLocation(volumeShader.ID, "volumeData"), 0);
	//glUniform1i(glGetUniformLocation(volumeShader.ID, "transferFunc"), 1);
	//glUniform3fv(glGetUniformLocation(volumeShader.ID, "eyePos"), camera.Position);

	glGenTextures(1, &transferFuncTex);
	glBindTexture(GL_TEXTURE_1D, transferFuncTex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//render loop
	while (!glfwWindowShouldClose(window))
	{
		//frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//std::cout << "fps:" << 1.0f/ deltaTime << "\n";
		//input
		processInput(window);
		// render command
		// -------------
		//clear color buffer
		glClearColor(0.1f, 0.1f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




		glm::mat4 view;
		glm::mat4 projection;

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		//glViewport(halfW, 0, halfW, halfH);
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		volumeShader.use();
		// --- 在 Render Loop 內，volumeShader.use() 之後 ---
		// 1. 先計算最新的 TF 資料
		std::vector<glm::vec4> tfData(256);
		for (int i = 0; i < 256; ++i) {
			float x = i / 255.0f;

			float r = getValueFromNodes(x, nodesR);
			float g = getValueFromNodes(x, nodesG);
			float b = getValueFromNodes(x, nodesB);
			float a = (r + g + b) / 3.0f; // 簡易 alpha
			if (i < 200) a = 0.0f;
			if (x < 0.05f) a = 0.0f; // 濾除低密度雜訊
			tfData[i] = glm::vec4(r, g, b, a);
		}
		glActiveTexture(GL_TEXTURE1); // 使用 Unit 1
		glBindTexture(GL_TEXTURE_1D, transferFuncTex);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, tfData.data());
		volumeShader.setInt("transferFunc", 1); // 對應 GL_TEXTURE1

		volumeShader.setVec3("eyePos", camera.Position);
		glUniform1f(glGetUniformLocation(volumeShader.ID, "stepSize"), 0.01f);
		volumeShader.setMat4("projection", projection);
		volumeShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(5.0f));
		model = glm::scale(model, glm::vec3(14.9f, 20.8f, 11.0f));
		volumeShader.setMat4("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glViewport(0, 0, SCR_WIDTH * 0.2, SCR_HEIGHT * 0.2);
		draw_tf_interface();


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
float getValueFromNodes(float x, const std::vector<Node>& nodes) {
	// 確保 nodes 是按 x 座標排序的
	if (x <= nodes[0].x) return nodes[0].y;
	if (x >= nodes[nodes.size() - 1].x) return nodes[nodes.size() - 1].y;

	// 尋找所在的線段區間
	for (size_t i = 0; i < nodes.size() - 1; ++i) {
		if (x >= nodes[i].x && x <= nodes[i + 1].x) {
			// 線性插值公式：y = y0 + (y1 - y0) * (x - x0) / (x1 - x0)
			float t = (x - nodes[i].x) / (nodes[i + 1].x - nodes[i].x);
			return nodes[i].y + t * (nodes[i + 1].y - nodes[i].y);
		}
	}
	return 0.0f;
}
void draw_line_from_nodes(const std::vector<Node>& nodes) {
	if (nodes.empty()) return;

	unsigned int vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// nodes 的內存布局與 float[2] 是一致的，可以直接上傳
	glBufferData(GL_ARRAY_BUFFER, nodes.size() * sizeof(Node), &nodes[0], GL_STREAM_DRAW);

	// 設定頂點屬性 (Location = 0)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Node), (void*)0);
	glEnableVertexAttribArray(0);

	// 增加線條寬度以便觀察
	glLineWidth(2.0f);
	glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(nodes.size()));
	glLineWidth(1.0f); // 恢復預設值

	// 清理
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void draw_tf_interface() {
	glDisable(GL_DEPTH_TEST);
	uiShader.use();
	// 使用正交投影，將座標對應到 [0, 1] 範圍
	glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	uiShader.setMat4("projection", projection);

	// 繪製背景（黑色矩形）
	float bgVertices[] = {
		// x,   y
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	unsigned int bgVAO, bgVBO;
	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);

	glBindVertexArray(bgVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	uiShader.setVec3("color", glm::vec3(0.1f, 0.1f, 0.1f)); // 深灰色背景
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// 釋放資源（正式實作建議將 VAO/VBO 緩存起來，不要每幀建立）
	glDeleteVertexArrays(1, &bgVAO);
	glDeleteBuffers(1, &bgVBO);

	// 繪製 R 線段
	uiShader.setVec3("color", glm::vec3(1.0, 0.0, 0.0));
	draw_line_from_nodes(nodesR);

	// 繪製 G 線段
	uiShader.setVec3("color", glm::vec3(0.0, 1.0, 0.0));
	draw_line_from_nodes(nodesG);

	// 繪製 B 線段
	uiShader.setVec3("color", glm::vec3(0.0, 0.0, 1.0));
	draw_line_from_nodes(nodesB);
	glEnable(GL_DEPTH_TEST);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		borc = !borc;
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
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		attach = !attach;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		lit_switch = !lit_switch;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		lit_switch1 = !lit_switch1;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		robotPos.z += 0.05;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		robotPos.z -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		robotPos.x -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		robotPos.x += 0.05;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (get_mouse) {
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
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}