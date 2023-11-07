
#include <QMouseEvent>
#include <QLabel>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QScreen>

#include <array>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Window.h"

namespace
{

//constexpr std::array<GLfloat, 21u> vertices = {
//	0.0f, 0.707f, 1.f, 0.f, 0.f, 0.0f, 0.0f,
//	-0.5f, -0.5f, 0.f, 1.f, 0.f, 0.5f, 1.0f,
//	0.5f, -0.5f, 0.f, 0.f, 1.f, 1.0f, 0.0f,
//};
//constexpr std::array<GLuint, 3u> indices = {0, 1, 2};

}// namespace

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 1);

	setLayout(layout);

	timer_.start();

	connect(this, &Window::updateUI, [=] {
		fps->setText(formatFPS(ui_.fps));
	});
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		texture_.reset();
		program_.reset();
	}
}

void Window::onInit()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/cube.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/cube.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// Create VBO
//	vbo_.create();
//	vbo_.bind();
//	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
//	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	// ----------------------------------------------------------------

	loadModel("Models/Cube.gltf");
	vaoAndEbos = bindModel();

	// ---------------------------------------------

	// Create IBO
//	ibo_.create();
//	ibo_.bind();
//	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
//	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

//	texture_ = std::make_unique<QOpenGLTexture>(QImage(":/Textures/voronoi.png"));
//	texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
//	texture_->setWrapMode(QOpenGLTexture::WrapMode::Repeat);

	// Bind attributes
	program_->bind();

//	program_->enableAttributeArray(0);
//	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(7 * sizeof(GLfloat)));
//
//	program_->enableAttributeArray(1);
//	program_->setAttributeBuffer(1, GL_FLOAT, static_cast<int>(2 * sizeof(GLfloat)), 3,
//								 static_cast<int>(7 * sizeof(GLfloat)));
//
//	program_->enableAttributeArray(2);
//	program_->setAttributeBuffer(2, GL_FLOAT, static_cast<int>(5 * sizeof(GLfloat)), 2,
//								 static_cast<int>(7 * sizeof(GLfloat)));

	mvpUniform_ = program_->uniformLocation("MVP");
	sun_position_ = program_->uniformLocation("sun_position");
	sun_color_ = program_->uniformLocation("sun_color");

	// Release all
	program_->release();

	vao_.release();

//	ibo_.release();
//	vbo_.release();

	// Еnable depth test and face culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate MVP matrix
//	model_.setToIdentity();
//	model_.translate(0, 0, -2);
//	view_.setToIdentity();
//	const auto mvp = projection_ * view_ * model_;

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	displayLoop();
	// Update uniform value
//	program_->setUniformValue(mvpUniform_, mvp);

	// Activate texture unit and bind texture
//	glActiveTexture(GL_TEXTURE0);
//	texture_->bind();

	// Draw
//	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

	// Release VAO and shader program
//	texture_->release();
	vao_.release();
	program_->release();

	++frameCount_;

	// Request redraw if animated
	if (animated_)
	{
		update();
	}
}

void Window::onResize(const size_t width, const size_t height)
{
	// Configure viewport
	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));

	// Configure matrix
	const auto aspect = static_cast<float>(width) / static_cast<float>(height);
	const auto zNear = 0.1f;
	const auto zFar = 100.0f;
	const auto fov = 60.0f;
	projection_.setToIdentity();
	projection_.perspective(fov, aspect, zNear, zFar);
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{ std::move(callback) }
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}
	};
}

bool Window::loadModel(const char *filename) {
	bool res = loader.LoadASCIIFromFile(&this->model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
}

void Window::bindModelNodes(std::map<int, GLuint>& vbos,
					tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(vbos, model.meshes[node.mesh]);
	}

	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(vbos, model.nodes[node.children[i]]);
	}
}

std::pair<GLuint, std::map<int, GLuint>> Window::bindModel() {
	std::map<int, GLuint> vbos;
	//		GLuint vao;
	//		glGenVertexArrays(1, &vao);
	//		glBindVertexArray(vao);
	vao_.bind();

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(vbos, model.nodes[scene.nodes[i]]);
	}

	vao_.release();
	//		glBindVertexArray(0);
	// cleanup vbos but do not delete index buffers yet
	for (auto it = vbos.cbegin(); it != vbos.cend();) {
		// ----------------------------------------
		if (it->first < 0)
			continue;
		// ---------------------------------------
		tinygltf::BufferView bufferView = model.bufferViews[it->first];
		if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {		// TODO: target was not set in bindMesh
			glDeleteBuffers(1, &vbos[it->first]);
			vbos.erase(it++);
		}
		else {
			++it;
		}
	}

	return {vao_.objectId(), vbos};
}

void Window::bindMesh(std::map<int, GLuint>& vbos, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];

		// ------------------------------------------------------
//		int target = bufferView.target == 0 ? GL_ELEMENT_ARRAY_BUFFER : bufferView.target;
		//			glBindBuffer(target, vbos[i]);
		//			glBufferData(target, bufferView.byteLength,
		//						 &model.bufferViews.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
		// ------------------------------------------------------

		if (bufferView.target == 0) {  // TODO impl drawarrays
//		if (target == 0) {
			std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;  // Unsupported bufferView.
					 /*
                   From spec2.0 readme:
                   https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
                            ... drawArrays function should be used with a count equal to
                   the count            property of any of the accessors referenced by the
                   attributes            property            (they are all equal for a given
                   primitive).
                 */
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
					std::cout << "bufferview.target " << bufferView.target << std::endl;
//		std::cout << "bufferview.target " << target << std::endl;

		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		glBindBuffer(bufferView.target, vbo);

		std::cout << "buffer.data.size = " << buffer.data.size()
				  << ", bufferview.byteOffset = " << bufferView.byteOffset
				  << std::endl;

		glBufferData(bufferView.target, bufferView.byteLength,
					 &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) vaa = 0;
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			// --------------------------------------------------------
//			if (attrib.first.compare("TANGENT") == 0) continue;
			// --------------------------------------------------------
			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
									  accessor.normalized ? GL_TRUE : GL_FALSE,
									  byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else
				std::cout << "vaa missing: " << attrib.first << std::endl;
		}

		if (model.textures.size() > 0) {
			// fixme: Use material's baseColor
			tinygltf::Texture &tex = model.textures[0];

			if (tex.source > -1) {

				GLuint texid;
				glGenTextures(1, &texid);

				tinygltf::Image &image = model.images[tex.source];

				glBindTexture(GL_TEXTURE_2D, texid);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				GLenum format = GL_RGBA;

				if (image.component == 1) {
					format = GL_RED;
				} else if (image.component == 2) {
					format = GL_RG;
				} else if (image.component == 3) {
					format = GL_RGB;
				} else {
					// ???
				}

				GLenum type = GL_UNSIGNED_BYTE;
				if (image.bits == 8) {
					// ok
				} else if (image.bits == 16) {
					type = GL_UNSIGNED_SHORT;
				} else {
					// ???
				}

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
							 format, type, &image.image.at(0));
			}
		}
	}
}

void Window::drawMesh(tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vaoAndEbos.second.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					   indexAccessor.componentType,
					   BUFFER_OFFSET(indexAccessor.byteOffset));
	}
}

// recursively draw node and children nodes of model
void Window::drawModelNodes(tinygltf::Node &node) {
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(model.meshes[node.mesh]);
	}

	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(model.nodes[node.children[i]]);
	}
}
void Window::drawModel() {
	//		glBindVertexArray(vaoAndEbos.first);
	vao_.bind();

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(model.nodes[scene.nodes[i]]);
	}

	vao_.release();
//			glBindVertexArray(0);
}

glm::mat4 Window::genView(glm::vec3 pos, glm::vec3 lookat) {
	// Camera matrix
	glm::mat4 view = glm::lookAt(
		pos,                // Camera in World Space
		lookat,             // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	return view;
}

glm::mat4 Window::genMVP(glm::mat4 view_mat, glm::mat4 model_mat, float fov, int w,
				 int h) {
	glm::mat4 Projection =
		glm::perspective(glm::radians(fov), (float)w / (float)h, 0.01f, 1000.0f);

	// Or, for an ortho camera :
//	 glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);
	// // In world coordinates

	glm::mat4 mvp = Projection * view_mat * model_mat;

	return mvp;
}

void Window::displayLoop() {
	//		Shaders shader = Shaders();
	//		glUseProgram(shader.pid);

	// ----------------------------------
	glUseProgram(program_->programId());
	// ----------------------------------

	// grab uniforms to modify
	//		GLuint MVP_u = glGetUniformLocation(program_->programId(), "MVP");
	//		GLuint sun_position_u = glGetUniformLocation(program_->programId(), "sun_position");
	//		GLuint sun_color_u = glGetUniformLocation(program_->programId(), "sun_color");

	//		tinygltf::Model model;
	//		if (!loadModel(model, filename.c_str())) return;

	//		std::pair<GLuint, std::map<int, GLuint>> vaoAndEbos = bindModel(model);
	// dbgModel(model); return;

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 model_mat = glm::mat4(1.0f);
	glm::mat4 model_rot = glm::mat4(1.0f);
	glm::vec3 model_pos = glm::vec3(-3, 0, -3);

	// generate a camera view, based on eye-position and lookAt world-position
	glm::mat4 view_mat = genView(glm::vec3(2, 2, 20), model_pos);

	glm::vec3 sun_position = glm::vec3(3.0, 10.0, -5.0);
	glm::vec3 sun_color = glm::vec3(1.0);

	//		while (!this->close()) {
	//			this->resize();

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 trans =
		glm::translate(glm::mat4(1.0f), model_pos);  // reposition model
	model_rot = glm::rotate(model_rot, glm::radians(0.8f),
							glm::vec3(0, 1, 0));  // rotate model on y axis
	model_mat = trans * model_rot;

	// build a model-view-projection
	GLint w, h;
	//			glfwGetWindowSize(window.window, &w, &h);
	h = this->size().height();
	w = this->size().width();
	glm::mat4 mvp = genMVP(view_mat, model_mat, 45.0f, w, h);
	glUniformMatrix4fv(mvpUniform_, 1, GL_FALSE, &mvp[0][0]);

	glUniform3fv(sun_position_, 1, &sun_position[0]);
	glUniform3fv(sun_color_, 1, &sun_color[0]);

	drawModel();
	update();

	//			glSwapBuffers(window.window);		// TODO
	//			context()->swapBuffers();
	//			glfwPollEvents();
	//		}

	//		glDeleteVertexArrays(1, &vaoAndEbos.first);
}
