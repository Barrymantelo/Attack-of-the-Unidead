#include "window.h"
#include "render2D.h"
#include <iostream>
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#endif
#if defined(__LINUX__)

#endif
#if defined(_APPLE_)

#endif

struct	BMTWindow {
	GLFWwindow* glfw_window;
	int x;
	int y;
	int width;
	int height;
	int mousex;
	int mousey;

	bool hidden;
	bool locked;

	double targetFPS;

	const char* title;
	int keys[MAX_KEYS];
	int buttons[MAX_BUTTONS];

	double currentTime, previousTime;
	double updateTime, drawTime;
	double frameTime = 0.0;
	double targetTime = 0.0;

	void(*BMTResizeCallback)(int width, int height);
};
BMTWindow bmt_win;

void rebuildState() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void keycallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	bmt_win.keys[key] = action;
	Panel::key_callback_func(key, action);
}

void cursorPosCallback(GLFWwindow* win, double xPos, double yPos) {
	bmt_win.mousex = xPos;
	bmt_win.mousey = yPos;
	Panel::mouse_pos_callback_func(xPos, yPos);
}

void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
	bmt_win.buttons[button] = action;
	Panel::mouse_callback_func(button, action, bmt_win.mousex, bmt_win.mousey);
}

void resizeCallback(GLFWwindow* win, int width, int height) {
	if (bmt_win.BMTResizeCallback != NULL)
		bmt_win.BMTResizeCallback(width, height);
	bmt_win.width = width;
	bmt_win.height = height;
	set2DRenderViewport(0, 0, width, height);
}

void initWindow(int width, int height, const char* title, bool fullscreen, bool resizable, bool primary_monitor) {
	bmt_win.width = width;
	bmt_win.height = height;

	//INIT GLFW
	if (!glfwInit()) {
		std::cout << "GLFW could not initialize" << std::endl;
	}
	else {
		std::cout << "GLFW has initialized" << std::endl;
	}

	for (int i = 0; i < MAX_KEYS; ++i)
		bmt_win.keys[i] = -1;
	for (int i = 0; i < MAX_BUTTONS; ++i)
		bmt_win.buttons[i] = -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//END INIT GLFW

	glfwWindowHint(GLFW_RESIZABLE, resizable);
	if (fullscreen) {
		glfwWindowHint(GLFW_VISIBLE, false);
		glfwWindowHint(GLFW_DECORATED, false);
		bmt_win.glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (!bmt_win.glfw_window) {
			std::cout << "Windowed Window failed to be created" << std::endl;
			glfwTerminate();
		}
		bmt_win.x = bmt_win.y = 0;

		setWindowSize(
			glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
			glfwGetVideoMode(glfwGetPrimaryMonitor())->height
		);

		glfwSetWindowPos(bmt_win.glfw_window, 0, 0);
		if(!primary_monitor) 
			glfwSetWindowPos(bmt_win.glfw_window, glfwGetVideoMode(glfwGetPrimaryMonitor())->width, 0);
	}
	else {
		glfwWindowHint(GLFW_VISIBLE, false);
		bmt_win.glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (!bmt_win.glfw_window) {
			std::cout << "Windowed Window failed to be created" << std::endl;
			glfwTerminate();
		}
		bmt_win.x = bmt_win.y = 0;

		float scrWidth = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
		float scrHeight = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

		glfwSetWindowPos(bmt_win.glfw_window, (scrWidth / 2) - (width / 2), (scrHeight / 2) - (height / 2));
		if (!primary_monitor)
			glfwSetWindowPos(bmt_win.glfw_window, glfwGetVideoMode(glfwGetPrimaryMonitor())->width + (scrWidth / 2) - (width / 2), (scrHeight / 2) - (height / 2));
	}

	//INIT GLEW
	glfwMakeContextCurrent(bmt_win.glfw_window);
	glfwSwapInterval(0);
	glfwShowWindow(bmt_win.glfw_window);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cout << glewGetErrorString(err) << std::endl;
		std::cout << "GLEW could not initialize" << std::endl;
	}
	else {
		std::cout << "GLEW has initialized" << std::endl;
	}
	rebuildState();

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Graphics Card: " << glGetString(GL_RENDERER) << std::endl << std::endl;

	glfwSetKeyCallback(bmt_win.glfw_window, keycallback);
	glfwSetWindowSizeCallback(bmt_win.glfw_window, resizeCallback);
	glfwSetMouseButtonCallback(bmt_win.glfw_window, mouseButtonCallback);
	glfwSetCursorPosCallback(bmt_win.glfw_window, cursorPosCallback);

	//END INIT GLEW

	init2D(0, 0, width, height);
}

void setWindowPos(int x, int y) {
	bmt_win.x = x;
	bmt_win.y = y;
	glfwSetWindowPos(bmt_win.glfw_window, x, y);
}

void setWindowSize(int width, int height) {
	bmt_win.width = width;
	bmt_win.height = height;
	glfwSetWindowSize(bmt_win.glfw_window, width, height);
	set2DRenderViewport(0, 0, width, height);
}

void setClearColor(float r, float g, float b, float a) {
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void setClearColor(vec4f& color) {
	setClearColor(color.x, color.y, color.z, color.w);
}

void beginDrawing() {
	bmt_win.currentTime = glfwGetTime();
	bmt_win.updateTime = bmt_win.currentTime - bmt_win.previousTime;
	bmt_win.previousTime = bmt_win.currentTime;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void endDrawing() {
	for (int i = 0; i < MAX_KEYS; ++i) {
		bmt_win.keys[i] = -1;
	}
	for (int i = 0; i < MAX_BUTTONS; ++i) {
		bmt_win.buttons[i] = -1;
	}

	glfwSwapBuffers(bmt_win.glfw_window);
	glfwPollEvents();

	bmt_win.currentTime = glfwGetTime();
	bmt_win.drawTime = bmt_win.currentTime - bmt_win.previousTime;
	bmt_win.previousTime = bmt_win.currentTime;

	bmt_win.frameTime = bmt_win.updateTime + bmt_win.drawTime;

	// Wait for some milliseconds...
	if (bmt_win.frameTime < bmt_win.targetTime)
	{
		double prevTime = glfwGetTime();
		double nextTime = 0.0;

		// Busy wait loop
#if defined(_BUSY_WAIT)
		while ((bmt_win.nextTime - bmt_win.prevTime) < ((bmt_win.targetTime - bmt_win.frameTime)*1000.0f) / 1000.0f) bmt_win.nextTime = glfwGetTime();
#elif defined(_WIN32)
		Sleep((bmt_win.targetTime - bmt_win.frameTime)*1000.0f);
#elif defined(__LINUX__)
		usleep((bmt_win.targetTime - bmt_win.frameTime)*1000.0f);
#elif defined(_APPLE_)
		usleep((bmt_win.targetTime - bmt_win.frameTime)*1000.0f);
#endif

		bmt_win.currentTime = glfwGetTime();
		double extraTime = bmt_win.currentTime - bmt_win.previousTime;
		bmt_win.previousTime = bmt_win.currentTime;

		bmt_win.frameTime += extraTime;
	}
	static int framecount = 0;
	framecount++;
	if (framecount > 150) {
		std::cout << "FPS:" << (int)(1.0f / (float)bmt_win.frameTime) << std::endl;
		framecount = 0;
	}
}

double getElapsedTime() {
	return glfwGetTime();
}

void setFPSCap(double FPS) {
	if (FPS < 1) bmt_win.targetTime = 0.0;
	else bmt_win.targetTime = 1.0 / FPS;
}

bool isWindowClosed() {
	return glfwWindowShouldClose(bmt_win.glfw_window) == 1;
}

void setWindowResizeCallback(void(*BMTmousecallback)(int width, int height)) {
	bmt_win.BMTResizeCallback = BMTmousecallback;
}

bool isKeyPressed(unsigned int keycode) {
	if (bmt_win.keys[keycode] == GLFW_PRESS)
		return true;
	return false;
}

bool isKeyReleased(unsigned int keycode) {
	if (bmt_win.keys[keycode] == GLFW_RELEASE) {
		return true;
	}
	return false;
}

bool isButtonPressed(unsigned int button) {
	if (bmt_win.buttons[button] == GLFW_PRESS) {
		return true;
	}
	return false;
}

bool isButtonReleased(unsigned int button) {
	if (bmt_win.buttons[button] == GLFW_RELEASE) {
		return true;
	}
	return false;
}

bool isKeyDown(unsigned int keycode) {
	if (glfwGetKey(bmt_win.glfw_window, keycode) == 1) {
		return true;
	}
	return false;
}

bool isButtonDown(unsigned int button) {
	if (glfwGetMouseButton(bmt_win.glfw_window, button) == 1) {
		return true;
	}
	return false;
}

bool isKeyUp(unsigned int keycode) {
	if (glfwGetKey(bmt_win.glfw_window, keycode) == 0) {
		return true;
	}
	return false;
}

bool isButtonUp(unsigned int button) {
	if (glfwGetMouseButton(bmt_win.glfw_window, button) == 0) {
		return true;
	}
	return false;
}

void setVSync(bool vsync) {
	if (vsync)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);
}

void getMousePos(double* mousex, double* mousey) {
	*mousex = bmt_win.mousex;
	*mousey = bmt_win.mousey;
}

vec2f getMousePos() {
	return vec2f(bmt_win.mousex, bmt_win.mousey);
}

void disposeWindow() {
	glfwSetWindowShouldClose(bmt_win.glfw_window, true);
	//I could actually free all memory and stuff but who cares if the window is closing the program will end and the OS will do it for me.
}

void setMouseLocked(bool locked) {
	bmt_win.locked = locked;
}

int getWindowWidth() {
	int width;
	glfwGetWindowSize(bmt_win.glfw_window, &width, 0);
	return width;
}

int getWindowHeight() {
	int height;
	glfwGetWindowSize(bmt_win.glfw_window, 0, &height);
	return height;
}

void setMouseHidden(bool hidden) {
	bmt_win.hidden = hidden;
	if (hidden)
		glfwSetInputMode(bmt_win.glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else
		glfwSetInputMode(bmt_win.glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}