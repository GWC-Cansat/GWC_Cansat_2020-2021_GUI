//GTK (GIMP ToolKit)
#include <gtkmm.h>
#include <giomm/resource.h>
//GLEW (OpenGL)
#include <GL/glew.h>
//GLM (OpenGL Mathematics Library)
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//OBJ Loader
#include <OBJ_Loader/OBJ_Loader.h>
//Standard Library
#include <iostream>
#include <algorithm>
#include <istream>

//TODO: Move shaders to their own file

class MainWindow : public Gtk::Window
{
protected:
  Gtk::GLArea mGlArea;
  Gtk::Label mTestLabel1;
  Gtk::Label mTestLabel2;
  Gtk::Label mTestLabel3;
  Gtk::Label mTestLabel4;
  Gtk::Paned mPaned;
  Gtk::Grid mGrid;
  GLuint vao, vbo;
  GLuint shaderProgram;
  std::vector<float> vertexPositions;
  GLint uniform_location_mvp;

  void onRealize()
  {
    mGlArea.make_current();

    glewExperimental = true; //GLArea only support Core profile.
    if (glewInit() != GLEW_OK)
    {
      std::cout << "ERROR: Can't create OpenGL context." << std::endl;
      exit(EXIT_FAILURE);
    }
    //Load Resources
    //Load CanSat Model
    objl::Loader loader;
    bool loadout = loader.LoadFile("assets/models/cansat_cylinder.obj");
    if (!loadout)
    { //TODO: Don't exit the program, just stop rendering the GLArea
      std::cout << "ERROR: Can't load CanSat model." << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "Number of Vertices: " << loader.LoadedVertices.size() << std::endl;

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
    const char* vertex_shader_code = vertex_code.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vertex_shader_code,NULL);
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
    const char* fragment_shader_code = fragment_code.c_str();

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragment_shader_code,NULL);
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
    std::cout << uniform_location_mvp << std::endl;

    //This point you have the context and you can use opengl methods.
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
  }

  void onUnrealize()
  {
    //your cleanUp. Deleting Vao etc.
  }

  bool onRender(const Glib::RefPtr<Gdk::GLContext> &context)
  {
    glm::mat4 model(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    auto allocation = mGlArea.get_allocation();

    glm::mat4 proj = glm::perspective(70.0f, float(allocation.get_width()) / float(allocation.get_height()), 0.01f, 100.0f);

    glm::mat4 mvp = proj * view * model;

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(uniform_location_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexPositions.size());
    glBindVertexArray(0);
  }

public:
  MainWindow()
  {
    set_title("GWC CanSat GUI");
    mTestLabel1.set_text("TEST1");
    mTestLabel2.set_text("TEST2");
    mTestLabel3.set_text("TEST3");
    mTestLabel4.set_text("TEST4");
    mTestLabel1.set_hexpand(true);
    mTestLabel2.set_hexpand(true);
    mTestLabel3.set_hexpand(true);
    mTestLabel4.set_hexpand(true);
    mTestLabel1.set_halign(Gtk::Align::ALIGN_START);
    mTestLabel2.set_halign(Gtk::Align::ALIGN_START);
    mTestLabel3.set_halign(Gtk::Align::ALIGN_START);
    mTestLabel4.set_halign(Gtk::Align::ALIGN_START);
    mGrid.attach(mTestLabel1, 0, 0);
    mGrid.attach(mTestLabel2, 1, 0);
    mGrid.attach(mTestLabel3, 0, 1);
    mGrid.attach(mTestLabel4, 1, 1);
    mPaned.set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
    mPaned.pack1(mGlArea);
    mPaned.pack2(mGrid, false, false);
    add(mPaned);
    mGlArea.set_auto_render();
    mGlArea.set_hexpand();
    mGlArea.set_vexpand();
    mGlArea.set_halign(Gtk::ALIGN_FILL);
    mGlArea.set_valign(Gtk::ALIGN_FILL);
    mGlArea.set_size_request(32, 32);
    mGlArea.set_required_version(3, 3); //your desired gl version

    mGlArea.signal_realize().connect(sigc::mem_fun(this, &MainWindow::onRealize));
    mGlArea.signal_unrealize().connect(sigc::mem_fun(this, &MainWindow::onUnrealize), false);
    mGlArea.signal_render().connect(sigc::mem_fun(this, &MainWindow::onRender));

    show_all();
  }
};

int main(int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");
  MainWindow mainWindow;
  mainWindow.resize(400, 400);
  return app->run(mainWindow);
}