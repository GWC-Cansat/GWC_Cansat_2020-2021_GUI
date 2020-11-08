#include <gtkmm.h>
#include <giomm/resource.h>
#include <GL/glew.h>
#include <iostream>

class MainWindow : public Gtk::Window{
protected:
  Gtk::GLArea mGlArea;
  Gtk::Label mTestLabel1;
  Gtk::Label mTestLabel2;
  Gtk::Label mTestLabel3;
  Gtk::Label mTestLabel4;
  Gtk::Paned mPaned;
  Gtk::Grid mGrid;
  void onRealize(){
    mGlArea.make_current();

    glewExperimental = true; //GLArea only support Core profile.
    if (glewInit() != GLEW_OK){
      std::cout << "ERROR: TIS FUCKERED" << std::endl;
    }

    //This point you have the context and you can use opengl methods.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  }

  void onUnrealize(){
    //your cleanUp. Deleting Vao etc.
  }

  bool onRender(const Glib::RefPtr<Gdk::GLContext> &context){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //here comes your rendering code
  }

public:
  MainWindow(){
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
    mGrid.attach(mTestLabel1,0,0);
    mGrid.attach(mTestLabel2,1,0);
    mGrid.attach(mTestLabel3,0,1);
    mGrid.attach(mTestLabel4,1,1);
    mPaned.set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
    mPaned.pack1(mGlArea);
    mPaned.pack2(mGrid,false,false);
    add(mPaned);
    mGlArea.set_auto_render();
    mGlArea.set_hexpand();
    mGlArea.set_vexpand();
    mGlArea.set_halign(Gtk::ALIGN_FILL);
    mGlArea.set_valign(Gtk::ALIGN_FILL);
    mGlArea.set_size_request(32, 32);
    mGlArea.set_required_version(3, 3); //your desired gl version

    mGlArea.signal_realize().connect(sigc::mem_fun(this,&MainWindow::onRealize));
    mGlArea.signal_unrealize().connect(sigc::mem_fun(this,&MainWindow::onUnrealize),false);
    mGlArea.signal_render().connect(sigc::mem_fun(this,&MainWindow::onRender));

    show_all();
  }
};

int main(int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc,argv,"org.gtkmm.examples.base");
  MainWindow mainWindow;
  mainWindow.resize(400, 400);
  return app->run(mainWindow);
}