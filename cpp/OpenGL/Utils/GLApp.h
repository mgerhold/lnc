#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLEnv.h"
#include "GLProgram.h"
#include "GLArray.h"
#include "GLBuffer.h"
#include "GLTexture2D.h"
#include "Image.h"

enum class LineDrawType {
  LIST,
  STRIP,
  LOOP
};

enum class TrisDrawType {
  LIST,
  STRIP,
  FAN
};

class GLApp {
public:
  GLApp(uint32_t w=640, uint32_t h=480, uint32_t s=4,
        const std::string& title = "My OpenGL App",
        bool fpsCounter=true, bool sync=true);
  
  void run();
  void setAnimation(bool animationActive) {
    if (this->animationActive && !animationActive)
      resumeTime = glfwGetTime();
    
    if (!this->animationActive && animationActive)
      glfwSetTime(resumeTime);
      
    this->animationActive = animationActive;
  }
  bool getAnimation() const {
    return animationActive;
  }

  void setImageFilter(GLint magFilter, GLint minFilter);
  void drawImage(const Image& image,
                 const Vec3& bl=Vec3{-1.0f,-1.0f,0.0f},
                 const Vec3& br=Vec3{1.0f,-1.0f,0.0f},
                 const Vec3& tl=Vec3{-1.0f,1.0f,0.0f},
                 const Vec3& tr=Vec3{1.0f,1.0f,0.0f});
  void drawTriangles(const std::vector<float>& data, TrisDrawType t, bool wireframe, bool lighting);
  void drawLines(const std::vector<float>& data, LineDrawType t, float lineThickness=1.0f);
  void drawPoints(const std::vector<float>& data, float pointSize=1.0f, bool useTex=false);
  void setDrawProjection(const Mat4& mat);
  void setDrawTransform(const Mat4& mat);
  void resetPointTexture(uint32_t resolution=64);
  void setPointTexture(const std::vector<uint8_t>& shape, uint32_t x, uint32_t y, uint32_t components);
  
  virtual void init() {}
  virtual void draw() {}
  virtual void animate(double animationTime) {}
  
  virtual void resize(int width, int height);
  virtual void keyboard(int key, int scancode, int action, int mods) {}
  virtual void mouseMove(double xPosition, double yPosition) {}
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) {}
  virtual void mouseWheel(double x_offset, double y_offset, double xPosition, double yPosition) {}
  
protected:
  GLEnv glEnv;
  Mat4 p;
  Mat4 mv;
  Mat4 mvi;
  GLProgram simpleProg;
  GLProgram simpleSpriteProg;
  GLProgram simpleTexProg;
  GLProgram simpleLightProg;
  GLArray simpleArray;
  GLBuffer simpleVb;
  GLTexture2D raster;
  GLTexture2D pointSprite;
  double resumeTime;
  
  void closeWindow() {
    glEnv.setClose();
  }
  
private:
  bool animationActive;

  static GLApp* staticAppPtr;
  static void sizeCallback(GLFWwindow* window, int width, int height) {
    if (staticAppPtr) staticAppPtr->resize(width, height);
  }
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (staticAppPtr) staticAppPtr->keyboard(key, scancode, action, mods);
  }
  static void cursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition) {
    if (staticAppPtr) staticAppPtr->mouseMove(xPosition, yPosition);
  }
  static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (staticAppPtr) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      staticAppPtr->mouseButton(button, state, mods, xpos, ypos);
    }
  }
  static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
    if (staticAppPtr) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      staticAppPtr->mouseWheel(x_offset, y_offset, xpos, ypos);
    }
  }
  
  void shaderUpdate();
  
  void triangulate(const Vec3& p0,
                   const Vec3& p1, const Vec4& c1,
                   const Vec3& p2, const Vec4& c2,
                   const Vec3& p3,
                   float lineThickness,
                   std::vector<float>& trisData);

};
