#include <iostream>
#include <GLApp.h>
#include <Vec2.h>
#include <Vec4.h>
#include <Mat4.h>
#include <random>

#include <BPNetwork.h>

#include "MNIST.h"

class MyGLApp : public GLApp {
public:
  MyGLApp() :
  GLApp(800,800,4, "Digit Learner", false, false)
  {}
  
  virtual void init() override {
    try {
      digitNetwork.load("network.txt");
      std::cout << "Resuming session from network.txt" << std::endl;
    } catch (const FileException& ) {
      std::cout << "Starting new session" << std::endl;
    }
    
    glEnv.setCursorMode(CursorMode::HIDDEN);
    GL(glClearColor(0,0,0,0));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
  }
  
  void dropPaint() {
    Vec2 iMousePos{mousePos*28};
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        image.setNormalizedValue(iMousePos.x(),iMousePos.y(),0,1.0f);
        image.setNormalizedValue(iMousePos.x(),iMousePos.y(),1,1.0f);
        image.setNormalizedValue(iMousePos.x(),iMousePos.y(),2,1.0f);
        image.setValue(iMousePos.x(),iMousePos.y(),3,255);
      }
    }
  }
  
  void clear() {
    image = Image{28,28};
  }

  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    mousePos = Vec2{float(xPosition/s.width),float(1.0-yPosition/s.height)};
    
    if (drawing) dropPaint();
  }
  
  virtual void mouseButton(int button, int state, int mods, double xPosition, double yPosition) override {
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
      drawing = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_RELEASE) {
      drawing = false;
      makeGuess();
    }
  }
  
  Vec getPixleData() {
    Vec theGrid{image.height*image.width};    
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        theGrid[x+((image.width-1)-y)*image.width] = image.getValue(x,y,3)/255.0;
      }
    }
    return theGrid;
  }
  
  struct GuessElem {
    size_t value;
    float activation;
    bool operator>(const GuessElem& other) const {
      return activation > other.activation;
    }
  };
  
  void makeGuess() {
    std::cout << "Guessing..." << std::endl;
    Vec guessVec = digitNetwork.feedforward(getPixleData());
    
    guess.clear();
    for (size_t i = 0;i<guessVec.size();++i) {
      guess.push_back(GuessElem{i,guessVec[i]});
    }
    std::sort(guess.begin(), guess.end(),std::greater<>());
    for (size_t i = 0;i<guess.size();++i) {
      std::cout << guess[i].value << " (" << guess[i].activation << ")\t";
    }
    std::cout  << std::endl;
  }
  
  void teach(size_t i) {
    Vec theTruth(10); theTruth[i] = 1;
    Update u = digitNetwork.backpropagation(getPixleData(), theTruth);
    digitNetwork.applyUpdate(u, 0.1f);
    
    std::cout << i << " understood ..." << std::endl;
  }
  
  void pickMIST() {
    try {
      MNIST mnist("train-images-idx3-ubyte", "train-labels-idx1-ubyte");
      std::random_device rd{};
      std::mt19937 gen{rd()};
      std::uniform_real_distribution<float> dist{0, 1};

      const size_t r = size_t(dist(gen)*mnist.data.size());
      const std::vector<uint8_t>& nistimage = mnist.data[r].image;
      for (uint32_t y = 0;y<28;++y) {
        for (uint32_t x = 0;x<28;++x) {
          const size_t sIndex = x+y*28;
          const size_t tIndex = x+(27-y)*28;
          const uint8_t p = nistimage[sIndex];
          
          image.data[tIndex*4+0] = p;
          image.data[tIndex*4+1] = p;
          image.data[tIndex*4+2] = p;
          image.data[tIndex*4+3] = p;
        }
      }


    } catch (const MNISTFileException& e) {
      std::cout << "Error loading MNIST data: " << e.what() << std::endl;
    }

  }
  
  void trainMNIST(size_t setSize, size_t epochs) {
    try {
      MNIST mnist("train-images-idx3-ubyte", "train-labels-idx1-ubyte");
        
      std::cout << "Data loaded, training in progress " << std::flush;

      for (size_t i = 0;i<epochs;++i) {
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::uniform_real_distribution<float> dist{0, 1};

        Update u;
        Vec inputVec{28*28};
        for (size_t i = 0;i<setSize;++i) {
          
          const size_t r = size_t(dist(gen)*mnist.data.size());
          Vec theTruth(10); theTruth[mnist.data[r].label] = 1;
          const std::vector<uint8_t>& image = mnist.data[r].image;
          for (uint32_t j = 0;j<28*28;++j) {
            inputVec[j] = float(image[j])/255.0f;
          }

          if (i == 0)
            u = digitNetwork.backpropagation(inputVec, theTruth);
          else
            u += digitNetwork.backpropagation(inputVec, theTruth);
        }
                      
        digitNetwork.applyUpdate(u, 0.1f/setSize);
        std::cout << "." << std::flush;
      }
    } catch (const MNISTFileException& e) {
      std::cout << "Error loading MNIST data: " << e.what() << std::endl;
    }
    std::cout << " Done" << std::endl;
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_ESCAPE:
          digitNetwork.save("network.txt");
          closeWindow();
          break;
        case GLFW_KEY_ENTER:
          teach(guess[0].value);
          break;
        case GLFW_KEY_P:
          pickMIST();
          makeGuess();
          break;
        case GLFW_KEY_C:
          clear();
          break;
        case GLFW_KEY_0:
        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
        case GLFW_KEY_7:
        case GLFW_KEY_8:
        case GLFW_KEY_9:
          teach(key - GLFW_KEY_0);
          break;
        case GLFW_KEY_T:
          trainMNIST(10,1000);
          break;
      }
    }
  }
  
  
  virtual void draw() override{
    GL(glClear(GL_COLOR_BUFFER_BIT));

    drawImage(image);
    
    std::vector<float> glShape;
    glShape.push_back(mousePos.x()*2.0f-1.0f); glShape.push_back(mousePos.y()*2.0f-1.0f); glShape.push_back(0.0f);
    glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f); glShape.push_back(1.0f);
    drawPoints(glShape, 40, true);
        
  }

private:
  Vec2 mousePos;
  std::vector<GuessElem> guess{10};
  BPNetwork digitNetwork{std::vector<size_t>{28*28,100,100,10}};
  bool drawing{false};
  Image image{28,28};
    
} myApp;

int main(int argc, char ** argv) {
  myApp.run();
  return EXIT_SUCCESS;
}
