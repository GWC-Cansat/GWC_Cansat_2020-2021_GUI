/*
Copyright (C) 2015 Tom Schoonjans
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
//GTK (GIMP ToolKit)
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/label.h>
#include <gtkmm/glarea.h>
#include <gtkmm/paned.h>
#include <gtkmm/grid.h>
#include <giomm/resource.h>
#include <gtkmm/application.h>
#include <gdkmm/frameclock.h>
#include <gtkmm/main.h>
#include <gtkmm/notebook.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <gtkmm/printsettings.h>
#include <gtkmm/pagesetup.h>
#include <gtkmm/printoperation.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/button.h>
//GTK Extras
#include <gtkmm-plplot.h>
//Epoxy (OpenGL)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
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
#include <vector>
#include <random>
#include <string>
 
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

using namespace std;

class CansatData{
public:
	//Pressure
	vector<double> p;
	//Temperature
	vector<double> t;
	//Humidity
	vector<double> h;
	//Acceleration
	vector<double> a_x;
	vector<double> a_y;
	vector<double> a_z;
	//Magnetometer
	vector<double> m_x;
	vector<double> m_y;
	vector<double> m_z;
	//Gyroscope
	vector<double> g_x;
	vector<double> g_y;
	vector<double> g_z;
	void generate_data(){
		random_device rd;
		mt19937 e2(rd());
		uniform_real_distribution<> dist(-1, 1);
		unsigned max = 100;
		for(unsigned i = 0; i < max; i++){
			//Pressure
			p.push_back(100.0+(double)i+dist(e2));
			//Temperature
			t.push_back(22.7-(double)i+dist(e2));
			//Humidity
			h.push_back(40.0f+dist(e2));
			//Acceleration
			a_x.push_back(dist(e2));
			a_y.push_back(dist(e2));
			a_z.push_back(1.0+dist(e2));
			//Magnetometer
			m_x.push_back(50.0+dist(e2));
			m_y.push_back(dist(e2));
			m_z.push_back(50.0+dist(e2));
			//Gyroscope
			g_x.push_back(dist(e2));
			g_y.push_back(dist(e2));
			g_z.push_back(dist(e2));
		}
	}
};


class MainWindow : public Gtk::Window{
protected:
	//Widgets
	Gtk::GLArea mGlArea;
	Gtk::Label mLabelAngleX;
	Gtk::Label mLabelAngleY;
	Gtk::Label mLabelAngleZ;
	Gtk::Paned mPaned;
	Gtk::Grid mGrid;
	Gtk::Scale mScaleAngleX;
	Gtk::Scale mScaleAngleY;
	Gtk::Scale mScaleAngleZ;
	Gtk::Notebook mNotebook;
	//OpenGL
	GLuint vao, vbo;
	GLuint shaderProgram;
	vector<float> vertexPositions;
	GLint uniform_location_mvp;
	//Plotting
    Gtk::PLplot::Canvas *canvas;
	Gtk::PLplot::Plot2D *plot;
	Gtk::Grid mPlotGrid;
	Gtk::Grid mPlotSettingsGrid;
	Gtk::Label mLabelXVarComboBoxText;
	Gtk::Label mLabelYVarComboBoxText;
	Gtk::ComboBoxText mXVarComboBoxText;
	Gtk::ComboBoxText mYVarComboBoxText;
	Gtk::Button mUpdatePlotButton;
	//Cansat
	CansatData data;
	bool onKeyPress(GdkEventKey* key_event){
		if(key_event->keyval == GDK_KEY_Escape){
			Gtk::Main::quit();
			return true;
		}
		return false;
	}
	
	void onRealize(){
		mGlArea.make_current();
	 
		//Load Resources
		//Load CanSat Model
		objl::Loader loader;
		bool loadout = loader.LoadFile("assets/models/cansat_cylinder.obj");
		if (!loadout){ //TODO: Don't exit the program, just stop rendering the GLArea
			cout << "ERROR: Can't load CanSat model." << endl;
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < loader.LoadedVertices.size(); i++){
			vertexPositions.push_back(loader.LoadedVertices[i].Position.X);
			vertexPositions.push_back(loader.LoadedVertices[i].Position.Y);
			vertexPositions.push_back(loader.LoadedVertices[i].Position.Z);
		}

		ifstream vertex_shader_file;
		vertex_shader_file.open("assets/shaders/simple.vert");
		stringstream vertex_shader_stream;
		vertex_shader_stream << vertex_shader_file.rdbuf();
		vertex_shader_file.close();
		string vertex_code = vertex_shader_stream.str();
		const char *vertex_shader_code = vertex_code.c_str();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertex_shader_code, NULL);
		glCompileShader(vertexShader);

		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success){
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog << endl;
		}

		ifstream fragment_shader_file;
		fragment_shader_file.open("assets/shaders/simple.frag");
		stringstream fragment_shader_stream;
		fragment_shader_stream << fragment_shader_file.rdbuf();
		fragment_shader_file.close();
		string fragment_code = fragment_shader_stream.str();
		const char *fragment_shader_code = fragment_code.c_str();

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragment_shader_code, NULL);
		glCompileShader(fragmentShader);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success){
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog << endl;
		}

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success){
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED: " << infoLog << endl;
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

	void onUnrealize(){
		//Cleanup. Deleting Vao etc.
	}

	bool onRender(const Glib::RefPtr<Gdk::GLContext> &context){
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
	
	void redraw_glarea(){
		mGlArea.queue_render();
	}
	
	void onPlotUpdateButtonClicked(){
		update_plot_data();
	}
	void update_plot_data(){
		try{
			while(true){
				plot->remove_data(0);
			}
   		}catch(...) {}
		plot->set_axis_title_x("NONE");
		plot->set_axis_title_y("NONE");

		vector<double> * x_var;
		if(mXVarComboBoxText.get_active_text() == "PRESSURE"){
			x_var = &data.p; 
		}else if(mXVarComboBoxText.get_active_text() == "TEMPERATURE"){
			x_var = &data.t; 
		}else if(mXVarComboBoxText.get_active_text() == "HUMIDITY"){
			x_var = &data.h; 
		}else{
			return;
		}

		vector<double> * y_var;
		if(mYVarComboBoxText.get_active_text() == "PRESSURE"){
			y_var = &data.p; 
		}else if(mYVarComboBoxText.get_active_text() == "TEMPERATURE"){
			y_var = &data.t; 
		}else if(mYVarComboBoxText.get_active_text() == "HUMIDITY"){
			y_var = &data.h; 
		}else{
			return;
		}

		plot->set_axis_title_x(mXVarComboBoxText.get_active_text());
		plot->set_axis_title_y(mYVarComboBoxText.get_active_text());

		Gtk::PLplot::PlotData2D * plot_data = new Gtk::PLplot::PlotData2D(*x_var, *y_var, Gdk::RGBA("red"),Gtk::PLplot::LineStyle::NONE);
		plot_data->set_symbol("X");
		plot->add_data(*plot_data);
		
		return;
	}
public:
	MainWindow(){
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
		
		mScaleAngleX.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
		mScaleAngleY.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
		mScaleAngleZ.signal_value_changed().connect(sigc::mem_fun(this, &MainWindow::redraw_glarea));
		//Add scales to grid
		mGrid.attach(mScaleAngleX, 1, 0);
		mGrid.attach(mScaleAngleY, 1, 1);
		mGrid.attach(mScaleAngleZ, 1, 2);
		//Pane setup
		mPaned.set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
		mPaned.pack1(mNotebook);
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
		//Test Data
		
		data.generate_data();
		//Plotting setup
		plot = new Gtk::PLplot::Plot2D("NONE", "NONE", "Cansat Data Plot");
      	canvas = new Gtk::PLplot::Canvas(*plot);
		plot->hide_legend();
		plot->add_object(*Gtk::manage(new Gtk::PLplot::PlotObject2DLine(Gtk::ORIENTATION_HORIZONTAL, 0)));
		plot->add_object(*Gtk::manage(new Gtk::PLplot::PlotObject2DLine(Gtk::ORIENTATION_VERTICAL, 0)));

		set_default_size(720, 580);
		Gdk::Geometry geometry;
		geometry.min_aspect = geometry.max_aspect = double(720)/double(580);
		set_geometry_hints(*this, geometry, Gdk::HINT_ASPECT);
		set_title("GWC Cansat Interface");
		canvas->set_hexpand(true);
		canvas->set_vexpand(true);

		canvas->set_size_request(200,200);

		//X Var Dropdown Setup
		mXVarComboBoxText.append("PRESSURE");
		mXVarComboBoxText.append("TEMPERATURE");
		mXVarComboBoxText.append("HUMIDITY");
		mXVarComboBoxText.append("ACCELERATION X");
		mXVarComboBoxText.append("ACCELERATION Y");
		mXVarComboBoxText.append("ACCELERATION Z");
		mXVarComboBoxText.append("MAGNETOMETER X");
		mXVarComboBoxText.append("MAGNETOMETER Y");
		mXVarComboBoxText.append("MAGNETOMETER Z");
		mXVarComboBoxText.append("GYROSCOPE X");
		mXVarComboBoxText.append("GYROSCOPE Y");
		mXVarComboBoxText.append("GYROSCOPE Z");
		mXVarComboBoxText.set_active(0);
		mXVarComboBoxText.set_margin_bottom(10);
		mXVarComboBoxText.set_margin_top(10);
		mLabelXVarComboBoxText.set_text(" X Variable: ");
		//Y Var Dropdown Setup
		mYVarComboBoxText.append("PRESSURE");
		mYVarComboBoxText.append("TEMPERATURE");
		mYVarComboBoxText.append("HUMIDITY");
		mYVarComboBoxText.append("ACCELERATION X");
		mYVarComboBoxText.append("ACCELERATION Y");
		mYVarComboBoxText.append("ACCELERATION Z");
		mYVarComboBoxText.append("MAGNETOMETER X");
		mYVarComboBoxText.append("MAGNETOMETER Y");
		mYVarComboBoxText.append("MAGNETOMETER Z");
		mYVarComboBoxText.append("GYROSCOPE X");
		mYVarComboBoxText.append("GYROSCOPE Y");
		mYVarComboBoxText.append("GYROSCOPE Z");
		mYVarComboBoxText.set_active(0);
		mYVarComboBoxText.set_margin_bottom(10);
		mYVarComboBoxText.set_margin_top(10);
		mLabelYVarComboBoxText.set_text(" Y Variable: ");
		//Update Plot Button Setup
		mUpdatePlotButton = Gtk::Button("Update Plot");
		mUpdatePlotButton.set_margin_top(10);
		mUpdatePlotButton.set_margin_bottom(10);
		mUpdatePlotButton.set_margin_left(10);
		mUpdatePlotButton.set_margin_right(10);
		mUpdatePlotButton.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::onPlotUpdateButtonClicked));
		//Plot Settings Grid Setup
		mPlotSettingsGrid.attach(mLabelXVarComboBoxText,0,0);
		mPlotSettingsGrid.attach(mLabelYVarComboBoxText,0,1);
		mPlotSettingsGrid.attach(mXVarComboBoxText,1,0);
		mPlotSettingsGrid.attach(mYVarComboBoxText,1,1);
		mPlotSettingsGrid.attach(mUpdatePlotButton,2,0,1,2);
		mPlotSettingsGrid.set_halign(Gtk::ALIGN_CENTER);
		mPlotSettingsGrid.set_valign(Gtk::ALIGN_CENTER);
		//Plot Pane Setup
		mPlotGrid.attach(mPlotSettingsGrid,0,0);
		mPlotGrid.attach(*canvas,0,1);

		//Notepad Setup
		mNotebook.append_page(mGlArea, "3D");
		mNotebook.append_page(mPlotGrid, "Plot");
		this->signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyPress), false);
		
		//Add Data To Plot
		update_plot_data();

		//Show everything, otherwise the window looks empty
		show_all();
	}

	void on_draw_page(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr) {
      Cairo::RefPtr< ::Cairo::Context> cr = context->get_cairo_context();
      canvas->draw_plot(cr, 842, 595);
    }
};

int main(int argc, char *argv[]){
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");
	MainWindow mainWindow;
	return app->run(mainWindow);
}