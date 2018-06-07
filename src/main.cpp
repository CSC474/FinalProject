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

#define NUM_PARTICLES 805
#define GRAV_FACTOR .15

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
    std::shared_ptr<Program> shape, prog, postproc, partProg;
    
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
    
    //Frame Buffer
    GLuint fb, depth_fb, FBOtex;
    
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
        
        for (int ii = 0; ii < 200; ii++){
            //Center Dancer
            animmat[ii] = mat4(1);
            //Back Up Dancers
            animmat2[ii] = mat4(1);
        }
        
        //Center Dancer
        readtobone("../../resources/test.fbx",&all_animation,&root);
        root->set_animations(&all_animation,animmat,animmatsize);
        
        //Back-Up Dancers
        readtobone("../../resources/thrustChar00.fbx",&all_animation2,&root2);
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
        // *************** Particle *******************
        
        
        ///////////////////////////
        // FRAMEBUFFER CODE BELOW
        ///////////////////////////
        GLuint Tex1Location = glGetUniformLocation(postproc->pid, "tex");//tex, tex2... sampler in the fragment shader
        glUseProgram(postproc->pid);
        glUniform1i(Tex1Location, 0);
        int width, height;
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
        int anim_step_width_ms = 8490 / 204;
        //Back-Up Dancers (Thrust)
        int anim_step_width_ms_2 = 5711 / 138;

        ///////////////////////////////////
        static int frame = 0;
        if (totaltime_untilframe_ms >= anim_step_width_ms)
        {
            totaltime_untilframe_ms = 0;
            frame++;
        }
        
        //Center Dancer
        root->play_animation(frame,"axisneurontestfile_Avatar00");
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
        float xLocRight = 0.5;
        Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.3f, -6));
        M = Trans * S;
        prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        glDrawArrays(GL_LINES, 4, size_stick_2-4);
        
        //Left Dancer : Decreasing Transparency for Glow effect
        static float j;
        for(j = 0.002; j < 0.012; j+=0.002){
            //Left
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocLeft+j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick-4);
            //Left
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocLeft-j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick-4);
            //Right
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocRight+j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick-4);
            //Right
            Trans = glm::translate(glm::mat4(1.0f), glm::vec3(xLocRight-j, -1.3f, -6));
            M = Trans * S;
            prog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
            glUniform1f(prog->getUniform("Dancer"), j+1);
            glDrawArrays(GL_LINES, 4, size_stick-4);
        }
        
        glBindVertexArray(0);
        prog->unbind();
        
        if (frame == 10) {
            GenPartMats(&aParts.cParts[0], partAnims);
            aParts.cParts.back().ResetFall();
        }
        else if (frame == 70) {
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims2);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
        }
        else if (frame == 110) {
            particles nParts;
            GenParticles(root, &nParts);
            aParts.cParts.push_back(nParts);
            GenPartMats(&aParts.cParts.back(), partAnims3);
            aParts.cParts.back().ResetFall();
            aParts.isFalling.push_back(false);
        }
        
        if (frame > 10) {
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
            for (int pI = 0; pI < aParts.cParts.size(); pI++) {
                for (int i = 0; i < aParts.cParts[pI].pos.size(); i++) {
                    if (pI == 0)
                        MA = partAnims[i];
                    else if (pI == 1)
                        MA = partAnims2[i];
                    else if (pI == 2)
                        MA = partAnims3[i];
                    
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
                    glPointSize(3.0f);
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

int main(int argc, char **argv) {
	std::string resourceDir = "../../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

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
		// Render scene.
        application->render_to_framebuffer();
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
