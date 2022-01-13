#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
// window
gps::Window myWindow;


// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPosition;


// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lightPositionLoc;
GLuint constant;
GLuint linear;
GLuint quadratic;
GLuint shadowMap;

// camera
gps::Camera myCamera(
    glm::vec3(-2.0f, 8.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D cube;
gps::Model3D plane;
gps::Model3D sphere;
gps::Model3D monkey;

GLfloat angle;
GLfloat scale;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader debugDepthQuad;
gps::Shader shader;

float deltaTime_in_miliSecs;
float currentTimeStamp = 0;
float lastTimeStamp = 0;


bool is_mouseCentered = true;
double last_xpos, last_ypos;
float x_offset, y_offset;
float sensitivity;
float yaw = 0, pitch = 0;

bool wireWiew = false;

unsigned int depthMapFBO;

glm::mat4 lightProjection, lightView;
glm::mat4 lightSpaceMatrix;
float near_plane = 1.1f, far_plane = 50.0f;
glm::vec3 lightPos(-2.0f, 10.0f, -1.0f);
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int depthMap;
unsigned int planeVAO;

unsigned int quadVAO = 0;
unsigned int quadVBO;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO //DONE

    glViewport(0, 0, width, height);
    //glScissor(0, 0, width, height);

    /*WindowDimensions dims;
    dims.width = width;
    dims.height = height;*/

    myWindow.setWindowDimensions({width, height});

    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 40.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    last_xpos = (double)width / 2;
    last_ypos = (double)height / 2;
    is_mouseCentered = true;
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO //DONE

    if (is_mouseCentered) {
        last_xpos = xpos;
        last_ypos = ypos;
        is_mouseCentered = false;
    }

    x_offset = xpos - last_xpos;
    y_offset = last_ypos - ypos;
    last_xpos = xpos;
    last_ypos = ypos;

    sensitivity = 0.1f * deltaTime_in_miliSecs;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    myCamera.rotate(pitch, yaw);

    myBasicShader.useShaderProgram(); //use default shader
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void initUniforms();

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * deltaTime_in_miliSecs);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime_in_miliSecs);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f * deltaTime_in_miliSecs;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f * deltaTime_in_miliSecs;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_T]) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (pressedKeys[GLFW_KEY_Y]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (pressedKeys[GLFW_KEY_U]) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    if (pressedKeys[GLFW_KEY_O]) {
        scale += 0.01 * deltaTime_in_miliSecs;

        model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_P]) {
        scale -= 0.01 * deltaTime_in_miliSecs;

        model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_B]) {
        myBasicShader.loadShader(
            "Resource/Shader/solid_vert.shader",
            "Resource/Shader/solid_frag.shader");

        initUniforms();
    }

    if (pressedKeys[GLFW_KEY_N]) {
        myBasicShader.loadShader(
            "Resource/Shader/basic_vert_directional_light.shader",
            "Resource/Shader/basic_frag_directional_light.shader");

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

        initUniforms();
    }

    if (pressedKeys[GLFW_KEY_M]) {
        myBasicShader.loadShader(
            "Resource/Shader/basic_vert_point_light.shader",
            "Resource/Shader/basic_frag_point_light.shader");

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        initUniforms();
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);

    /*hidden mouse cursor and doesn't let it leave the window*/
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    /*hidden mouse cursor and dont let it leave the window*/
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
 // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("Resource/obj/teapot20segUT.obj");
    cube.LoadModel("Resource/obj/cube.obj");
    sphere.LoadModel("Resource/obj/sphere.obj");
    monkey.LoadModel("Resource/obj/monkey.obj");
    plane.LoadModel("Resource/obj/plane3.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "Resource/Shader/basic_vert_directional_light.shader",
        "Resource/Shader/basic_frag_directional_light.shader");

    depthMapShader.loadShader("Resource/Shader/simpleDepthShader.shader", "Resource/Shader/emptyfragmentshader.shader");
    debugDepthQuad.loadShader("Resource/Shader/debug_quad.vs", "Resource/Shader/debug_quad_depth.fs");
    shader.loadShader("Resource/Shader/shadow_mapping.vs", "Resource/Shader/shadow_mapping.fs");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 40.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(2.0f, 2.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPosition = glm::vec3(-2.0f, 10.0f, -1.0f);
    lightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosition");
    /*send light position to shader*/
    glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPosition));

    constant = glGetUniformLocation(myBasicShader.shaderProgram, "constant");
    glUniform1f(constant, 1.0f);

    linear = glGetUniformLocation(myBasicShader.shaderProgram, "linear_");
    glUniform1f(linear, 0.22f);

    quadratic = glGetUniformLocation(myBasicShader.shaderProgram, "quadratic");
    glUniform1f(quadratic, 0.20f);
}

void renderTeapotShader(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    shader.setMat4("model", model);
    teapot.Draw(shader);
}

void renderPlaneShader(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
   // model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    shader.setMat4("model", model);
  
    // draw teapot
    plane.Draw(shader);
}

void renderCubeShader(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    shader.setMat4("model", model);

    // draw teapot
    cube.Draw(shader);
}

void renderSphereShader(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 1.0f, 2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    shader.setMat4("model", model);
  
    // draw teapot
    sphere.Draw(shader);
}

void renderMonkeyShader(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 1.0f, -2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    shader.setMat4("model", model);
    //send teapot model matrix data to shader
  
    // draw teapot
    monkey.Draw(shader);
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale + glm::vec3(1.0f, 1.0f, 1.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
   
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}


void renderCube(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    cube.Draw(shader);
}

void renderSphere(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    sphere.Draw(shader);
}

void renderMonkey(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, -2.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    monkey.Draw(shader);
}

void renderPlane(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    //model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    plane.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    glClear(GL_STENCIL_BUFFER_BIT);

	// render the teapot
	renderTeapot(myBasicShader);
    renderCube(myBasicShader);
    renderSphere(myBasicShader);
    renderMonkey(myBasicShader);
    renderPlane(myBasicShader);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void  renderScene(const gps::Shader& shader)
{
    renderTeapotShader(shader);
    renderCubeShader(shader);
    renderSphereShader(shader);
    renderMonkeyShader(shader);
  
}

void PlaneSetUp()
{
    float planeVertices[] = {
        // positions            // normals         // texcoords


        25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        25.0f, -0.5f,  -25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         -25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    // plane VAO
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);


}
void shadowWork()
{
    PlaneSetUp();
    glGenFramebuffers(1, &depthMapFBO);
   
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float boadercolor[] = { 1,1,1,1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, boadercolor);


    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    debugDepthQuad.useShaderProgram();
    debugDepthQuad.setInt("depthMap", 0);
    shader.useShaderProgram();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 1);
  
  
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


void renderSceneShadow(const gps::Shader& shader)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //Testing Cubes---------------------------------------------------------------------------------------------------------
    //// cubes
    //model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    //model = glm::scale(model, glm::vec3(0.5f));
    //shader.setMat4("model", model);
    //renderCube();
    //model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    //model = glm::scale(model, glm::vec3(0.5f));
    //shader.setMat4("model", model);
    //renderCube();
    //model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    //model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    //model = glm::scale(model, glm::vec3(0.25));
    //shader.setMat4("model", model);
    //renderCube();
}

void LightWork()
{
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f
        , near_plane, far_plane);
    lightView = glm::lookAt(glm::vec3(-10.0f, 14.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    lightSpaceMatrix = lightProjection * lightView;

    depthMapShader.useShaderProgram();
    depthMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

}


void PreRenderSetUp()
{

    glViewport(0, 0, (double)myWindow.getWindowDimensions().width, (double)myWindow.getWindowDimensions().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.useShaderProgram();

    glm::mat4 Projection = glm::perspective(glm::radians(myCamera.Zoom), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 40.0f);
    glm::mat4 view = myCamera.getViewMatrix();
    shader.setMat4("projection", Projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", myCamera.cameraPosition.r, myCamera.cameraPosition.g, myCamera.cameraPosition.b);
    shader.setVec3("lightPos", lightPos.r, lightPos.g, lightPos.b);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    
    

}


int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    shadowWork();
    unsigned int woodTexture = loadTexture(std::string("Resource/wood.png").c_str());
  
    last_xpos = (double)myWindow.getWindowDimensions().width / 2;
    last_ypos = (double)myWindow.getWindowDimensions().height / 2;
    is_mouseCentered = true;

	
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        currentTimeStamp = 1000.0f * glfwGetTime() / 20;
        deltaTime_in_miliSecs = currentTimeStamp - lastTimeStamp;

        if (currentTimeStamp < 200)
        {
            // Presentation 
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime_in_miliSecs);
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime_in_miliSecs);
            angle -= 1.0f * deltaTime_in_miliSecs;
            // update model matrix for teapot
            model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
            // update normal matrix for teapot
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        }
        else
        {
            processMovement();

        }
        LightWork();
    // DepthTexture Flling Rendering on Depth Texture
      
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        renderScene(depthMapShader);    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //   Testing Purpose//-------------------------------------------------------------------------------------------------------
    //    glViewport(0, 0, (double)myWindow.getWindowDimensions().width, (double)myWindow.getWindowDimensions().height);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //debugDepthQuad.useShaderProgram();
    //debugDepthQuad.setFloat("near_plane", near_plane);
    //debugDepthQuad.setFloat("far_plane", far_plane);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, depthMap);
    //renderQuad();
    //   Testing Purpose//-------------------------------------------------------------------------------------------------------
        PreRenderSetUp();
       
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        // Renders Plane for Depth Tex
        renderSceneShadow(shader);
        //Renders Pot Sphere Monkey
        renderScene(shader);


		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();

        lastTimeStamp = currentTimeStamp;
	}

	cleanup();

    return EXIT_SUCCESS;
}

// Rendering ON Quad For Testing Purpose
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

//Not Using As this was for Testing Purpose
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}