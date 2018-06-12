// Core libraries
#include <iostream>
#include <cmath>
#include <ctime>
#include <string>
#include <fstream>
#include <streambuf>

// Third party libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Local headers
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include "line.h"
#include "bone.h"
#include "particle.h"
#include "allParts.h"
#include "Audio.h"
#include "Accelerate/accelerate.h"

#define NUM_PARTICLES 805
#define GRAV_FACTOR .15
#define HEAD_INDEX 15

/**** OpenAL setup ****/
#define FREQ 22050   // Sample rate
#define CAP_SIZE 2048 // How much to capture at a time (affects latency)

short buffer[FREQ*2]; // A buffer to hold captured audio

ALubyte texels[100*100*4];
ALubyte amplitudes2[1024];
float amplitudes[1024];
float buf2[2048];

using namespace std;
using namespace glm;

float totaltime_ms=0;

double get_last_elapsed_time() {
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

void GenParticles(bone *broot, particles *parts);
void GenPartMats(particles *parts, mat4 mats[]);
void CreateExtraVerts(vec3 lPos, vec3 cPos, particles *parts, bone *broot);

class Application : public EventCallbacks {
public:
	WindowManager *windowManager = nullptr;
    Camera *camera = nullptr;
    
    // Our shader program
    std::shared_ptr<Program> shape, prog, postproc, partProg, billProg;
    
    //Center Dancer
    GLuint VertexArrayID;
    GLuint VertexBufferID, VertexBufferIDimat, VertexNormDBox, VertexTexBox, IndexBufferIDBox;
    
    //Backup Dancers
    GLuint VertexArrayID2;
    GLuint VertexBufferID2, VertexBufferIDimat2, VertexNormDBox2, VertexTexBox2, IndexBufferIDBox2;
    
    //Frame Buffer
    GLuint VertexArrayIDScreen, VertexBufferIDScreen, VertexBufferTexScreen;
    
    //Particle Buffer
    GLuint VertexArrayIDPart;
    GLuint VertexBufferPart, VertexBufferPartMat;
    
    //Billboard
    GLuint BillboardVertexArrayID;
    GLuint BillboardVertexBufferID, BillboardVertexBufferIDimat, BillboardVertexNormDBox, BillboardVertexTexBox, BillboardIndexBufferIDBox;
    
    //Frame Buffer
    GLuint fb, depth_fb, FBOtex;
    
    //texture data
    GLuint Texture, AudioTexture, AudioTexLoc;
    
    // audio
    Audio audio;
    
    //animation matrices:
    mat4 animmat[200];
    mat4 animmat2[200];
    int animmatsize = 0;
    int animmatsize2 = 0;
    
    string filename;
    double gametime = 0;
    bool mousePressed = false;
    bool mouseCaptured = false;
    glm::vec2 mouseMoveOrigin = glm::vec2(0);
    glm::vec3 mouseMoveInitialCameraRot;
    bone *root = NULL;
    bone *root2 = NULL;
    particles parts;
    allParts aParts;
    mat4 partAnims[NUM_PARTICLES];
    mat4 partAnims2[NUM_PARTICLES];
    mat4 partAnims3[NUM_PARTICLES];
    mat4 partAnims4[NUM_PARTICLES];
    mat4 partAnims5[NUM_PARTICLES];
    mat4 partAnims6[NUM_PARTICLES];
    mat4 partAnims7[NUM_PARTICLES];
    mat4 partAnims8[NUM_PARTICLES];
    mat4 partAnims9[NUM_PARTICLES];
    mat4 partAnims10[NUM_PARTICLES];
    mat4 partAnims11[NUM_PARTICLES];
    mat4 partAnims12[NUM_PARTICLES];
    mat4 partAnims13[NUM_PARTICLES];
    mat4 partAnims14[NUM_PARTICLES];
    mat4 partAnims15[NUM_PARTICLES];
    mat4 partAnims16[NUM_PARTICLES];
    mat4 partAnims17[NUM_PARTICLES];
    mat4 partAnims18[NUM_PARTICLES];
    mat4 partAnims19[NUM_PARTICLES];
    mat4 partAnims20[NUM_PARTICLES];
    mat4 partAnims21[NUM_PARTICLES];
    mat4 partAnims22[NUM_PARTICLES];
    mat4 partAnims23[NUM_PARTICLES];
    mat4 partAnims24[NUM_PARTICLES];
    mat4 partAnims25[NUM_PARTICLES];
    mat4 partAnims26[NUM_PARTICLES];
    mat4 partAnims27[NUM_PARTICLES];
    mat4 partAnims28[NUM_PARTICLES];
    mat4 partAnims29[NUM_PARTICLES];
    mat4 partAnims30[NUM_PARTICLES];

    int size_stick = 0;
    int size_stick_2 = 0;
    all_animations all_animation;
    all_animations all_animation2;
    
    Application() {
        camera = new Camera();
    }
    
    ~Application() {
        delete camera;
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
		// Movement
        if (key == GLFW_KEY_W && action != GLFW_REPEAT) camera->vel.z = (action == GLFW_PRESS) * -0.2f;
        if (key == GLFW_KEY_S && action != GLFW_REPEAT) camera->vel.z = (action == GLFW_PRESS) * 0.2f;
        if (key == GLFW_KEY_A && action != GLFW_REPEAT) camera->vel.x = (action == GLFW_PRESS) * -0.2f;
        if (key == GLFW_KEY_D && action != GLFW_REPEAT) camera->vel.x = (action == GLFW_PRESS) * 0.2f;
        // Rotation
        if (key == GLFW_KEY_I && action != GLFW_REPEAT) camera->rotVel.x = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_K && action != GLFW_REPEAT) camera->rotVel.x = (action == GLFW_PRESS) * -0.02f;
        if (key == GLFW_KEY_J && action != GLFW_REPEAT) camera->rotVel.y = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_L && action != GLFW_REPEAT) camera->rotVel.y = (action == GLFW_PRESS) * -0.02f;
        if (key == GLFW_KEY_U && action != GLFW_REPEAT) camera->rotVel.z = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_O && action != GLFW_REPEAT) camera->rotVel.z = (action == GLFW_PRESS) * -0.02f;
        // Disable cursor (allows unlimited scrolling)
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            mouseCaptured = !mouseCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            resetMouseMoveInitialValues(window);
        }
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
    {
        mousePressed = (action != GLFW_RELEASE);
        if (action == GLFW_PRESS) {
            resetMouseMoveInitialValues(window);
        }
    }
    
    void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
    {
        //does nothing rn
    }

	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }
    
    // Reset mouse move initial position and rotation
    void resetMouseMoveInitialValues(GLFWwindow *window)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseMoveOrigin = glm::vec2(mouseX, mouseY);
        mouseMoveInitialCameraRot = camera->rot;
    }
   
	void initGeom(const std::string& resourceDirectory)
    {
        srand(std::time(0));
        //screen plane
        glGenVertexArrays(1, &VertexArrayIDScreen);
        glBindVertexArray(VertexArrayIDScreen);
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferIDScreen);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDScreen);
        vec3 vertices[6];
        vertices[0] = vec3(-1, -1, 0);
        vertices[1] = vec3(1, -1, 0);
        vertices[2] = vec3(1, 1, 0);
        vertices[3] = vec3(-1, -1, 0);
        vertices[4] = vec3(1, 1, 0);
        vertices[5] = vec3(-1, 1, 0);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vec3), vertices, GL_STATIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferTexScreen);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTexScreen);
        vec2 texscreen[6];
        texscreen[0] = vec2(0, 0);
        texscreen[1] = vec2(1, 0);
        texscreen[2] = vec2(1, 1);
        texscreen[3] = vec2(0, 0);
        texscreen[4] = vec2(1, 1);
        texscreen[5] = vec2(0, 1);
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vec2), texscreen, GL_STATIC_DRAW);
        //we need to set up the vertex array
        glEnableVertexAttribArray(1);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindVertexArray(0);

        //generate the VAO
        glGenVertexArrays(1, &BillboardVertexArrayID);
        glBindVertexArray(BillboardVertexArrayID);
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &BillboardVertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, BillboardVertexBufferID);
        
        GLfloat cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,//LD
            1.0, -1.0,  1.0,//RD
            1.0,  1.0,  1.0,//RU
            -1.0,  1.0,  1.0,//LU
        };
        //make it a bit smaller
        for (int i = 0; i < 12; i++)
            cube_vertices[i] *= 0.5;
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);
        
        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //color
        GLfloat cube_norm[] = {
            // front colors
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
        };
        
        glGenBuffers(1, &BillboardVertexNormDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, BillboardVertexNormDBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        //color
        glm::vec2 cube_tex[] = {
            // front colors
            glm::vec2(0.0, 1.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.0),
            glm::vec2(0.0, 0.0),
            
        };
        glGenBuffers(1, &BillboardVertexTexBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, BillboardVertexTexBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glGenBuffers(1, &BillboardIndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BillboardIndexBufferIDBox);
        GLushort cube_elements[] = {
            
            // front
            0, 1, 2,
            2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
        
        
        glBindVertexArray(0);
        
        
        int width, height, channels;
        char filepath[1000];
        
        //texture 1
        string str = resourceDirectory + "/glowHead.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        //set texture to the correct samplers in the fragment shader:
        GLuint Tex2Location = glGetUniformLocation(billProg->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(billProg->pid);
        glUniform1i(Tex2Location, 0);
        
        for (int ii = 0; ii < 200; ii++){
            //Center Dancer
            animmat[ii] = mat4(1);
            //Back Up Dancers
            animmat2[ii] = mat4(1);
        }
        
        //Center Dancer
        readtobone("../../resources/Liam_Main_6B_Char00.fbx",&all_animation,&root);
        root->set_animations(&all_animation,animmat,animmatsize);
        
        //Back-Up Dancers
        readtobone("../../resources/Liam_BackUp_6B_Char00.fbx",&all_animation2,&root2);
        root2->set_animations(&all_animation2,animmat2,animmatsize2);
    
        //Center Dancer
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        
        glGenBuffers(1, &VertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        
        vector<vec3> pos;
        vector<unsigned int> imat;
        root->write_to_VBOs(vec3(0, 0, 0), pos, imat);
        size_stick = pos.size();
        
        // Allocate Space for Bones
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*pos.size(), pos.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // Allocate Space for Animations
        glGenBuffers(1, &VertexBufferIDimat);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDimat);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint)*imat.size(), imat.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)0);
        
        
        //Back Up Dancers
        glGenVertexArrays(1, &VertexArrayID2);
        glBindVertexArray(VertexArrayID2);
        
        glGenBuffers(1, &VertexBufferID2);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID2);
        
        vector<vec3> pos2;
        vector<unsigned int> imat2;
        root2->write_to_VBOs(vec3(0, 0, 0), pos2, imat2);
        size_stick_2 = pos2.size();
        
        // Allocate Space for Bones
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*pos2.size(), pos2.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // Allocate Space for Animations
        glGenBuffers(1, &VertexBufferIDimat2);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDimat2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint)*imat2.size(), imat2.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, (void*)0);
        
        glUseProgram(prog->pid);
        
        
        // *************** Particle *******************
        //Center Dancer
        glGenVertexArrays(1, &VertexArrayIDPart);
        glBindVertexArray(VertexArrayIDPart);
        
        glGenBuffers(1, &VertexBufferPart);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferPart);
        
        GenParticles(root, &parts);
        vector<vec3> tPos;
        tPos.push_back(root->pos);
        // Allocate Space for Bones
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), tPos.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Backup Dancer
        aParts.cParts.push_back(parts);
        aParts.isFalling.push_back(false);
        aParts.mvInvolved.push_back(false);
        // *************** Particle *******************
        
        // ************* AudioTex *********************
        // dynamic audio texture
        glGenTextures(1, &AudioTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AudioTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        
        ///////////////////////////
        // FRAMEBUFFER CODE BELOW
        ///////////////////////////
        GLuint Tex1Location = glGetUniformLocation(postproc->pid, "tex");//tex, tex2... sampler in the fragment shader
        glUseProgram(postproc->pid);
        glUniform1i(Tex1Location, 0);
        //int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        //RGBA8 2D texture, 24 bit depth texture, 256x256
        glGenTextures(1, &FBOtex);
        glBindTexture(GL_TEXTURE_2D, FBOtex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //NULL means reserve texture memory, but texels are undefined
        //**** Tell OpenGL to reserve level 0
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        //You must reserve memory for other mipmaps levels as well either by making a series of calls to
        //glTexImage2D or use glGenerateMipmapEXT(GL_TEXTURE_2D).
        //Here, we'll use :
        glGenerateMipmap(GL_TEXTURE_2D);
        //make a frame buffer
        //-------------------------
        glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        //Attach 2D texture to this FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
        //-------------------------
        glGenRenderbuffers(1, &depth_fb);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_fb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        //-------------------------
        //Attach depth buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fb);
        //-------------------------
        //Does the GPU support current FBO configuration?
        GLenum status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (status)
        {
            case GL_FRAMEBUFFER_COMPLETE:
                cout << "status framebuffer: good";
                break;
            default:
                cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
        
	}
	
	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
        
        if(filename.length() < 1) {
            cerr << "file size err" << endl;
        }
        cout << "FILENAME: " << filename << endl;
        
		// Initialize the GLSL programs
        prog = std::make_shared<Program>();
        prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
        prog->init();
        prog->addUniform("Manim");
        prog->addUniform("Dancer");
        
        //program for the postprocessing
        postproc = std::make_shared<Program>();
        postproc->setVerbose(true);
        postproc->setShaderNames(resourceDirectory + "/ppshader_vertex.glsl", resourceDirectory + "/ppshader_fragment.glsl");
        if (!postproc->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        postproc->addAttribute("vertPos");
        postproc->addAttribute("vertTex");
        postproc->addUniform("timef");
        postproc->addUniform("resolution");
        
        // Program for particles
        partProg = std::make_shared<Program>();
        partProg->setShaderNames(resourceDirectory + "/part_vertex.glsl", resourceDirectory + "/part_fragment.glsl");
        partProg->init();
        partProg->addUniform("Panim");
        partProg->addUniform("Dancer");
        
        billProg = std::make_shared<Program>();
        billProg->setShaderNames(resourceDirectory + "/billboard_shader_vertex.glsl", resourceDirectory + "/billboard_shader_fragment.glsl");
        billProg->init();
        //glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        //glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

	}
    
    glm::mat4 getPerspectiveMatrix() {
        float fov = 3.14159f / 4.0f;
        float aspect = windowManager->getAspect();
        return glm::perspective(fov, aspect, 0.01f, 10000.0f);
    }
    
    void render() {
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        postproc->bind();
        glUniform1f(postproc->getUniform("timef"), totaltime_ms);
        vec2 res = vec2(800.0,600.0);
        glUniform2fv(postproc->getUniform("resolution"), 1, &res[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FBOtex);
        glBindVertexArray(VertexArrayIDScreen);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        postproc->unbind();
    
    }
  
	void render_to_framebuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Get current frame buffer size.
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width / (float)height;
        glViewport(0, 0, width, height);

        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        // Frame Data
        double frametime = get_last_elapsed_time();
        totaltime_ms += frametime;
        static double totaltime_untilframe_ms = 0;
        totaltime_untilframe_ms += frametime*1000.0;
        
        for (int ii = 0; ii < 200; ii++){
            animmat[ii] = mat4(1);
            animmat2[ii] = mat4(1);
        }
        
        //animation frame system
        //Center Dancer
        int anim_step_width_ms = root->animation[0]->duration / root->animation[0]->keyframes.size();
        //Back-Up Dancers (Thrust)
        int anim_step_width_ms_2 = root2->animation[0]->duration / root2->animation[0]->keyframes.size();

        ///////////////////////////////////
        static int frame = 600;
        if (totaltime_untilframe_ms >= anim_step_width_ms)
        {
            totaltime_untilframe_ms = 0;
            frame++;
        }
        
        //Center Dancer
        root->play_animation(frame,"avatar_0_fbx_tmp");
        //Back Up Dancers
        root2->play_animation(frame,"avatar_0_fbx_tmp");
        
        // Setup Matrices
        glm::mat4 V, M, P;
        P = getPerspectiveMatrix();
        V = camera->getViewMatrix();
        M = glm::mat4(1);
        
        //Center Dancer
        float xLoc = -1.3;
        glm::mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLoc, -1.3f, -4));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
        M = Trans * S;
        
        // Send to Shaders and draw
        glBindVertexArray(VertexArrayID);
        
        //Center Dancer
        prog->bind();
        prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("Manim"), 200, GL_FALSE, &animmat[0][0][0]);
        glUniform1f(prog->getUniform("Dancer"), 0);
        glDrawArrays(GL_LINES, 4, size_stick-4);
        
        //Center Dancer : Decreasing Transparency for Glow effect
        static float i;
        for(i = 0.002; i < 0.012; i+=0.002){
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLoc+i, -1.3f, -4));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), i);
            glDrawArrays(GL_LINES, 4, size_stick-4);
            
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLoc-i, -1.3f, -4));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), i);
            glDrawArrays(GL_LINES, 4, size_stick-4);
        }
    
        glBindVertexArray(0);
        
        //Back Up Dancers
        glBindVertexArray(VertexArrayID2);
        
        //Left Dancer
        float xLocLeft = -1.5;
        Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocLeft, -1.3f, -6));
        M = Trans * S;
        prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("Manim"), 200, GL_FALSE, &animmat2[0][0][0]);
        glUniform1f(prog->getUniform("Dancer"), 1);
        glDrawArrays(GL_LINES, 4, size_stick_2-4);
        
        //Right Dancer
        float xLocRight = 2.0;
        Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocRight, -1.3f, -6));
        M = Trans * S;
        prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        glDrawArrays(GL_LINES, 4, size_stick_2-4);
        
        //Left Dancer : Decreasing Transparency for Glow effect
        static float j;
        for(j = 0.002; j < 0.006; j+=0.002){
            //Left
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocLeft+j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick_2-4);
            //Left
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocLeft-j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick_2-4);
            //Right
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocRight+j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick_2-4);
            //Right
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocRight-j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick_2-4);
        }
        
        glBindVertexArray(0);
        prog->unbind();
        
        
        //Stick Figure Head
        billProg->bind();
        glBindVertexArray(BillboardVertexArrayID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BillboardIndexBufferIDBox);
        billProg->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        mat4 Vi = glm::transpose(V);
        Vi[0][3] = 0;
        Vi[1][3] = 0;
        Vi[2][3] = 0;
        
        //glUniform3fv(billProg->getUniform("campos"), 1, &mycam.pos[0]);
        Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLoc+1.2, 0.6, -4));
        mat4 MF = animmat[HEAD_INDEX];
        S = glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.35f, 0.35f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        M = Trans * S * Vi;
        glUniformMatrix4fv(billProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniformMatrix4fv(billProg->getUniform("MFollow"), 1, GL_FALSE, &MF[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        glBindVertexArray(0);
        
        cout << "frame: " << frame << endl;
        billProg->unbind();
        
        if (frame == 600) {
            GenPartMats(&aParts.cParts[0], partAnims);
            aParts.cParts.back().ResetFall();
        }
        else if (frame == 700) {
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims2);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 800) {
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims3);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 900) {
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims4);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1000){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims5);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1100){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims6);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1200){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims7);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1300){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims8);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1400){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims9);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1500){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims10);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1600){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims11);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1700){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims12);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1800){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims13);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 1900){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims14);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2000){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims15);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2100){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims16);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2200){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims17);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2300){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims18);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2400){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims19);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2500){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims20);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2600){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims21);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2700){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims22);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2800){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims23);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 2900){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims24);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3000){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims25);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3100){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims26);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3200){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims27);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3300){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims28);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3400){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims29);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        else if (frame == 3500){
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims30);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
            aParts.mvInvolved.push_back(false);
        }
        
        if (frame > 600) {
            // *********** Particles *****************
            partProg->bind();
            // Center Dancer particle
            glBindVertexArray(VertexArrayIDPart);
            
            xLoc = -1.3;
            S = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLoc, -1.3f, -4));
            M = Trans * S;
            mat4 MA;
            
            aParts.UpdateisFalling(frame);
            aParts.UpdatemvInvolved(frame);
            for (int pI = 0; pI < aParts.cParts.size(); pI++) {
                for (int i = 0; i < aParts.cParts[pI].pos.size(); i++) {
                    if (pI == 0)
                        MA = partAnims[i];
                    else if (pI == 1)
                        MA = partAnims2[i];
                    else if (pI == 2)
                        MA = partAnims3[i];
                    else if (pI == 3)
                        MA = partAnims4[i];
                    else if (pI == 4)
                        MA = partAnims5[i];
                    else if (pI == 5)
                        MA = partAnims6[i];
                    else if (pI == 6)
                        MA = partAnims7[i];
                    else if (pI == 7)
                        MA = partAnims8[i];
                    else if (pI == 8)
                        MA = partAnims9[i];
                    else if (pI == 9)
                        MA = partAnims10[i];
                    else if (pI == 10)
                        MA = partAnims11[i];
                    else if (pI == 11)
                        MA = partAnims12[i];
                    else if (pI == 12)
                        MA = partAnims13[i];
                    else if (pI == 13)
                        MA = partAnims14[i];
                    else if (pI == 14)
                        MA = partAnims15[i];
                    else if (pI == 15)
                        MA = partAnims16[i];
                    else if (pI == 16)
                        MA = partAnims17[i];
                    else if (pI == 17)
                        MA = partAnims18[i];
                    else if (pI == 18)
                        MA = partAnims19[i];
                    else if (pI == 19)
                        MA = partAnims20[i];
                    else if (pI == 20)
                        MA = partAnims21[i];
                    else if (pI == 21)
                        MA = partAnims22[i];
                    else if (pI == 22)
                        MA = partAnims23[i];
                    else if (pI == 23)
                        MA = partAnims24[i];
                    else if (pI == 24)
                        MA = partAnims25[i];
                    else if (pI == 25)
                        MA = partAnims26[i];
                    else if (pI == 26)
                        MA = partAnims27[i];
                    else if (pI == 27)
                        MA = partAnims28[i];
                    else if (pI == 28)
                        MA = partAnims29[i];
                    else if (pI == 29)
                        MA = partAnims30[i];
                    
                    if (amplitudes2[100] > 230 && aParts.mvInvolved[pI])
                        aParts.cParts[pI].speed[i].y = -3;
                    
                    if (amplitudes2[1023] > 230 && pI == 2 && aParts.mvInvolved[pI])
                        aParts.cParts[pI].speed[i].y = 5;
                    
                    if (aParts.isFalling[pI]) {
                        aParts.cParts[pI].impulse[i].y -= GRAV_FACTOR;
                        aParts.cParts[pI].speed[i] += aParts.cParts[pI].impulse[i];
                        MA[3][0] += aParts.cParts[pI].speed[i].x;
                        MA[3][1] += aParts.cParts[pI].speed[i].y;
                        MA[3][2] += aParts.cParts[pI].speed[i].z;
                        
                    }

                    if (MA[3][1] < 0) {
                        MA[3][1] = 0;
                        aParts.cParts[pI].impulse[i] *= -.5;
                    }

                    partProg->setMVP(&M[0][0], &V[0][0], &P[0][0]);
                    partProg->setMatrix("MA", &MA[0][0]);
                    glUniform1f(partProg->getUniform("Dancer"), 0);
                    glPointSize(4.0f);
                    glDrawArrays(GL_POINTS, 0, 3);
                }
            }
            
            partProg->unbind();
        }
        // *********** Particles *****************
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, FBOtex);
        glGenerateMipmap(GL_TEXTURE_2D);
	}
    
    void fft(short buffer[]) {
        //******* set up fourier transform *******//
        
        //clear texel buffer
        for (int i=0; i<40000; i++) {
            texels[i] = 0;
        }
        
        //create float array from short array
        copy(buffer, buffer + 2048, buf2);
        
        const int n = 2048;
        const int log2n = 11; // 2^11 = 2048
        
        DSPSplitComplex a;
        a.realp = new float[n/2];
        a.imagp = new float[n/2];
        
        // prepare the fft algo (you want to reuse the setup across fft calculations)
        FFTSetup setup = vDSP_create_fftsetup(log2n, kFFTRadix2);
        
        // copy the input to the packed complex array that the fft algo uses
        vDSP_ctoz((DSPComplex *) buf2, 2, &a, 1, n/2);
        
        // calculate the fft
        vDSP_fft_zrip(setup, &a, 1, log2n, FFT_FORWARD);
        
        // do something with the complex spectrum
        int k=0;
        for (size_t i = 0; i < n/2; i++) {
            amplitudes[k] = a.imagp[i];
            k++;
        }
        
        //map each value from 'float' array to size of byte (originally size of short)
        for (int i=0; i<1024; i++) {
            amplitudes2[i] = (amplitudes[i] - SHRT_MIN)/(SHRT_MAX - SHRT_MIN) * 255;
        }
    }
};

void GenParticles(bone *broot, particles *parts) {
    vec3 lPos = vec3(INT_MIN, INT_MIN, INT_MIN);
    
    if (!parts->pos.empty())
        lPos = parts->pos.back();
    
    parts->pos.push_back(broot->pos);
    parts->ma.push_back(broot->mat);
    parts->speed.push_back(vec3(0,0,0));
    parts->impulse.push_back(vec3(0,0,0));
    
    for (auto kid: broot->kids)
        GenParticles(kid, parts);
}

void CreateExtraVerts(vec3 lPos, vec3 cPos, particles *parts, bone *broot) {
    if (distance(lPos, cPos) < 3.5) // do not lower this number
        return;
    
    vec3 avg = vec3((lPos.x + cPos.x)/2, (lPos.y + cPos.y)/2, (lPos.z + cPos.z)/2);
    parts->pos.push_back(avg);
    parts->ma.push_back(broot->mat);
    parts->speed.push_back(vec3(0,0,0));
    parts->impulse.push_back(vec3(0,0,0));
    CreateExtraVerts(cPos, parts->pos.back(), parts, broot);
    CreateExtraVerts(lPos, parts->pos.back(), parts, broot);
}

void GenPartMats(particles *parts, mat4 mats[]) {
    
    for (int i = 0; i < parts->ma.size(); i++) {
        mats[i] = *(parts->ma[i]);
        
    }
}

void fft(short buffer[]) {
    //******* set up fourier transform *******//
    
    //clear texel buffer
    for (int i=0; i<40000; i++) {
        texels[i] = 0;
    }
    
    //create float array from short array
    copy(buffer, buffer + 2048, buf2);
    
    const int n = 2048;
    const int log2n = 11; // 2^11 = 2048
    
    DSPSplitComplex a;
    a.realp = new float[n/2];
    a.imagp = new float[n/2];
    
    // prepare the fft algo (you want to reuse the setup across fft calculations)
    FFTSetup setup = vDSP_create_fftsetup(log2n, kFFTRadix2);
    
    // copy the input to the packed complex array that the fft algo uses
    vDSP_ctoz((DSPComplex *) buf2, 2, &a, 1, n/2);
    
    // calculate the fft
    vDSP_fft_zrip(setup, &a, 1, log2n, FFT_FORWARD);
    
    // do something with the complex spectrum
    int k=0;
    for (size_t i = 0; i < n/2; i++) {
        amplitudes[k] = a.imagp[i];
        k++;
    }
    
    //map each value from 'float' array to size of byte (originally size of short)
    for (int i=0; i<1024; i++) {
        amplitudes2[i] = (amplitudes[i] - SHRT_MIN)/(SHRT_MAX - SHRT_MIN) * 255;
    }
}

int main(int argc, char **argv) {
	std::string resourceDir = "../../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();
    
    // Inititalize audio
    Audio *audio = new Audio();
    audio->initAL();

    // Initialize window.
	WindowManager * windowManager = new WindowManager();
	windowManager->init(800, 600);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);
    
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle())) {
        // Update camera position.
        application->camera->update();
        //Audio stuff
        audio->readAudio();
        application->fft(audio->buffer);
		// Render scene.
        application->render_to_framebuffer();
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
    audio->cleanAL();
	windowManager->shutdown();
	return 0;
}
