//GLAD (OpenGL)
#include <glad/glad.h>
//GTK (GIMP ToolKit)
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/label.h>
#include <gtkmm/glarea.h>
#include <gtkmm/paned.h>
#include <gtkmm/grid.h>
#include <giomm/resource.h>
#include <gdkmm/frameclock.h>
//GLM (OpenGL Mathematics Library)
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
//OBJ Loader
#include <OBJ_Loader/OBJ_Loader.h>
//Standard Library
#include <iostream>
#include <algorithm>
#include <istream>

class MainWindow : public Gtk::Window
{
protected:
  Gtk::GLArea mGlArea;
  Gtk::Label mLabelAngleX;
  Gtk::Label mLabelAngleY;
  Gtk::Label mLabelAngleZ;
  Gtk::Paned mPaned;
  Gtk::Grid mGrid;
  Gtk::Scale mScaleAngleX;
  Gtk::Scale mScaleAngleY;
  Gtk::Scale mScaleAngleZ;
  GLuint vao, vbo;
  GLuint shaderProgram;
  std::vector<float> vertexPositions;
  GLint uniform_location_mvp;

  void onRealize()
  {
    mGlArea.make_current();
    
    if (!gladLoadGL()) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
    }
    
    /*
    glClearColor (0.0, 0.0, 1.0, 1.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    */
   
    //Load Resources
    //Load CanSat Model
    objl::Loader loader;
    bool loadout = loader.LoadFile("assets/models/cansat_cylinder.obj");
    if (!loadout)
    { //TODO: Don't exit the program, just stop rendering the GLArea
      std::cout << "ERROR: Can't load CanSat model." << std::endl;
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < loader.LoadedVertices.size(); i++)
    {
      vertexPositions.push_back(loader.LoadedVertices[i].Position.X);
      vertexPositions.push_back(loader.LoadedVertices[i].Position.Y);
      vertexPositions.push_back(loader.LoadedVertices[i].Position.Z);
    }

    std::ifstream vertex_shader_file;
    vertex_shader_file.open("assets/shaders/simple.vert");
    std::stringstream vertex_shader_stream;
    vertex_shader_stream << vertex_shader_file.rdbuf();
    vertex_shader_file.close();
    std::string vertex_code = vertex_shader_stream.str();
    const char *vertex_shader_code = vertex_code.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_code, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog << std::endl;
    }

    std::ifstream fragment_shader_file;
    fragment_shader_file.open("assets/shaders/simple.frag");
    std::stringstream fragment_shader_stream;
    fragment_shader_stream << fragment_shader_file.rdbuf();
    fragment_shader_file.close();
    std::string fragment_code = fragment_shader_stream.str();
    const char *fragment_shader_code = fragment_code.c_str();

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_code, NULL);
    glCompileShader(fragmentShader);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexPositions.size() * sizeof(vertexPositions[0]), vertexPositions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

    uniform_location_mvp = glGetUniformLocation(shaderProgram, "mvp");

    //This point you have the context and you can use opengl methods.
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    
  }

  void onUnrealize()
  {
    //your cleanUp. Deleting Vao etc.
  }

  bool onRender(const Glib::RefPtr<Gdk::GLContext> &context)
  {
    //Report OpenGL Details
    int major;
    int minor;
    context->get_version(major,minor);
    if(context->get_use_es()){
        std::cout << "Using OpenGL ES " << major << "." << minor << std::endl;
    }else{
        std::cout << "Using OpenGL " << major << "." << minor << std::endl;
    }
    /*
    auto allocation = mGlArea.get_allocation();
    glViewport(0, 0, (GLsizei) allocation.get_width(), (GLsizei) allocation.get_height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, float(allocation.get_width()) / float(allocation.get_height()), 1.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -3.6);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);
    glVertex3f(-2.0, -1.0, 0.0);
    glVertex3f(-2.0, 1.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -1.0, 0.0);

    glVertex3f(1.0, -1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(2.41421, 1.0, -1.41421);
    glVertex3f(2.41421, -1.0, -1.41421);
    glEnd();
    glFlush();
    */
    
    //Calculate model-view-projection matrix
    glm::mat4 model = glm::eulerAngleYXZ(
      glm::radians(mScaleAngleY.get_value()),
      glm::radians(mScaleAngleX.get_value()),
      glm::radians(mScaleAngleZ.get_value()));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.5f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    auto allocation = mGlArea.get_allocation();
    glm::mat4 proj = glm::perspective(70.0f, float(allocation.get_width()) / float(allocation.get_height()), 0.01f, 100.0f);

    glm::mat4 mvp = proj * view * model;
    
    //Render
    glUseProgram(shaderProgram);
    //Update model-view-projection matrix
    glUniformMatrix4fv(uniform_location_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    //Clear framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Draw geometry
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexPositions.size());
    return true;
  }
  
  bool graphicsBodge(const Glib::RefPtr<Gdk::FrameClock>& frame_clock){
      queue_draw();
      return true;
  }
  
public:
  MainWindow()
  {
    //Window setup
    set_title("GWC CanSat GUI");
    resize(400, 400);
    //Label setup
    mLabelAngleX.set_text(" Angle X: ");
    mLabelAngleY.set_text(" Angle Y: ");
    mLabelAngleZ.set_text(" Angle Z: ");
    mLabelAngleX.set_halign(Gtk::Align::ALIGN_START);
    mLabelAngleY.set_halign(Gtk::Align::ALIGN_START);
    mLabelAngleZ.set_halign(Gtk::Align::ALIGN_START);
    //Add labels to grid
    mGrid.attach(mLabelAngleX, 0, 0);
    mGrid.attach(mLabelAngleY, 0, 1);
    mGrid.attach(mLabelAngleZ, 0, 2);
    //Scale setup
    mScaleAngleX.set_range(0.0, 360.0);
    mScaleAngleY.set_range(0.0, 360.0);
    mScaleAngleZ.set_range(0.0, 360.0);
    mScaleAngleX.set_size_request(100,-1);
    mScaleAngleY.set_size_request(100,-1);
    mScaleAngleZ.set_size_request(100,-1);
    mScaleAngleX.set_hexpand(true);
    mScaleAngleY.set_hexpand(true);
    mScaleAngleZ.set_hexpand(true);
    mScaleAngleX.set_halign(Gtk::ALIGN_FILL);
    mScaleAngleY.set_halign(Gtk::ALIGN_FILL);
    mScaleAngleZ.set_halign(Gtk::ALIGN_FILL);
    //mScaleAngleX.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
    //mScaleAngleY.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
    //mScaleAngleZ.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
    //Add scales to grid
    mGrid.attach(mScaleAngleX, 1, 0);
    mGrid.attach(mScaleAngleY, 1, 1);
    mGrid.attach(mScaleAngleZ, 1, 2);
    //Pane setup
    mPaned.set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
    mPaned.pack1(mGlArea);
    mPaned.pack2(mGrid, false, false);
    add(mPaned);
    //GLArea setup
    mGlArea.set_use_es(true);
    mGlArea.set_auto_render();
    mGlArea.set_hexpand();
    mGlArea.set_vexpand();
    mGlArea.set_halign(Gtk::ALIGN_FILL);
    mGlArea.set_valign(Gtk::ALIGN_FILL);
    mGlArea.set_size_request(32, 32);
    mGlArea.signal_realize().connect(sigc::mem_fun(this, &MainWindow::onRealize));
    mGlArea.signal_unrealize().connect(sigc::mem_fun(this, &MainWindow::onUnrealize), false);
    mGlArea.signal_render().connect(sigc::mem_fun(this, &MainWindow::onRender));
    //Ignore this
    add_tick_callback(sigc::mem_fun(this, &MainWindow::graphicsBodge));
    //Show everything, otherwise the window looks empty
    show_all();
    //mGlArea.hide();
  }
};

int main(int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");
  MainWindow mainWindow;
  return app->run(mainWindow);
}
