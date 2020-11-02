#include "nddapplication.hpp" 
 
namespace msi_vr
{
    NDDApplication::NDDApplication(GLFWwindow *window)
        : Application(window), vrContext(), withVR(vrContext.initialized)
    {
        foreground.initializeObject(1200, 600, ODSSphereObject::Mode::POINTCLOUD);
        foreground.changeOverlap(30);
        foreground.setVerticesPositions();
        background.initializeObject(1200, 600, ODSSphereObject::Mode::POINTCLOUD);
        inpainting.initializeObject(1200, 600, ODSSphereObject::Mode::MESH);

        // currently just for testing, not actual working ndd shader
        nddShader.loadShader("shader/ndd.vs", "shader/ndd.fs");
        nddShader.attachUniform("dimension", [this](GLuint location) { glUniform2i(location, foreground.width, foreground.height); });
        nddShader.attachUniform("minMaxDiscardDepth", [this](GLuint location) { glUniform3f(location, 10.0f, 50.0f, 0.6f); });

        nddShader.attachUniform("modelmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, &this->foreground.modelmatrix[0][0]); });

        // change the attached shader according to 'withVR'
        nddShader.attachUniform("projectionmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.projectionmatrixLeft[0][0] : &this->cam.projectionmatrix[0][0]); }, "left");
        nddShader.attachUniform("projectionmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.projectionmatrixRight[0][0] : &this->cam.projectionmatrix[0][0]); }, "right");
        nddShader.attachUniform("viewmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.viewmatrixLeft[0][0] : &this->cam.getViewMatrix(true)[0][0]); }, "left");
        nddShader.attachUniform("viewmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.viewmatrixRight[0][0] : &this->cam.getViewMatrix(false)[0][0]); }, "right");

        nddShader.attachUniform("leftEye", [this](GLuint location) { glUniform1i(location, 1); }, "leftEye");
        nddShader.attachUniform("leftEye", [this](GLuint location) { glUniform1i(location, 0); }, "rightEye");

        // currently just for testing, not actual working ndd shader
        foregroundNddShader.loadShader("shader/ndd_foreground.vs", "shader/ndd_foreground.fs");
        foregroundNddShader.attachUniform("dimension", [this](GLuint location) { glUniform2i(location, foreground.width, foreground.height); });
        foregroundNddShader.attachUniform("minMaxDiscardDepth", [this](GLuint location) { glUniform3f(location, 10.0f, 50.0f, 0.6f); });

        foregroundNddShader.attachUniform("modelmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, &this->foreground.modelmatrix[0][0]); });

        // change the attached shader according to 'withVR'
        foregroundNddShader.attachUniform("projectionmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.projectionmatrixLeft[0][0] : &this->cam.projectionmatrix[0][0]); }, "left");
        foregroundNddShader.attachUniform("projectionmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.projectionmatrixRight[0][0] : &this->cam.projectionmatrix[0][0]); }, "right");
        foregroundNddShader.attachUniform("viewmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.viewmatrixLeft[0][0] : &this->cam.getViewMatrix(true)[0][0]); }, "left");
        foregroundNddShader.attachUniform("viewmatrix", [this](GLuint location) { glUniformMatrix4fv(location, 1, GL_FALSE, withVR ? &this->vrContext.viewmatrixRight[0][0] : &this->cam.getViewMatrix(false)[0][0]); }, "right");

        foregroundNddShader.attachUniform("leftEye", [this](GLuint location) { glUniform1i(location, 1); }, "left");
        foregroundNddShader.attachUniform("leftEye", [this](GLuint location) { glUniform1i(location, 0); }, "right");

        // setup camera movements
        inputManager.updateKeyBindings[InputManager::Key::W] = [this]() { cam.move(cam.front); };
        inputManager.updateKeyBindings[InputManager::Key::S] = [this]() { cam.move(-cam.front); };
        inputManager.updateKeyBindings[InputManager::Key::D] = [this]() { cam.move(cam.right); };
        inputManager.updateKeyBindings[InputManager::Key::A] = [this]() { cam.move(-cam.right); };
        inputManager.updateKeyBindings[InputManager::Key::C] = [this]() { cam.move(cam.up); };
        inputManager.updateKeyBindings[InputManager::Key::X] = [this]() { cam.move(-cam.up); };
        inputManager.callbackKeyBindings[InputManager::Key::U] = [this]() { foreground.setVerticesPositions(); };

        Mouse::addScrollCallback([this](double xoffset, double yoffset) 
        {
            foreground.changeOverlap(yoffset);
        });

        // look around with mouse drag
        Mouse::addDragCallback([this](double x, double y, int button) {
            if (button != GLFW_MOUSE_BUTTON_LEFT)
                return;

            cam.rotate(static_cast<float>(x), static_cast<float>(y));
        });

        offscreenRenderPassLeft.createTexture(1024, 1024);
        offscreenRenderPassRight.createTexture(1024, 1024);
    }

    NDDApplication::~NDDApplication()
    {
    }

    void NDDApplication::loadScene(std::string const &sceneName)
    {
        this->sceneName = sceneName;

        // load videotextures
        foregroundMegaTexture.loadVideo(sceneName + "/foreground/megatexture.mp4");
        backgroundMegaTexture.loadTexture(sceneName + "/background/megatexture.png");
        inpaintingMegaTexture.loadTexture(sceneName + "/inpainting/megatexture.png");

        foregroundMegaTexture.start();

        this->sceneInitialized = true;

        // add keys for video management
        inputManager.callbackKeyBindings[InputManager::Key::SPACE] = [this]() { foregroundMegaTexture.pause(); };
        inputManager.callbackKeyBindings[InputManager::Key::P] = [this]() { foregroundMegaTexture.play(); };

        glEnable(GL_DEPTH_TEST);
    }

    void NDDApplication::update(std::chrono::duration<float> const &time)
    {
    }

    void NDDApplication::fixedUpdate(std::chrono::duration<float> const &time)
    {
        cam.update(time.count());
        foregroundMegaTexture.update();
    }

    void NDDApplication::render() const
    {
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw left to offscreen!
        offscreenRenderPassLeft.use();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glViewport(0, 0, 1024, 1024);

        // inpainting
        nddShader.activate();
        nddShader.applyUniforms("left");
        nddShader.applyUniforms("rightEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inpaintingMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, inpaintingMegaTexture.texture);
        glUniform1i(glGetUniformLocation(nddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(nddShader.programID, "depth"), 1);
        inpainting.draw(true);
        nddShader.applyUniforms("leftEye");
        inpainting.draw(true);
        glUseProgram(0);

        // background
        foregroundNddShader.activate();
        foregroundNddShader.applyUniforms("left");
        foregroundNddShader.applyUniforms("rightEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, backgroundMegaTexture.texture);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "depth"), 1);
        background.draw(true);
        foregroundNddShader.applyUniforms("leftEye");
        background.draw(true);
        glUseProgram(0);

        // foreground
        foregroundNddShader.activate();
        foregroundNddShader.applyUniforms("left");
        foregroundNddShader.applyUniforms("rightEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, foregroundMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, foregroundMegaTexture.texture);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "depth"), 1);
        foreground.draw(true);
        foregroundNddShader.applyUniforms("leftEye");
        foreground.draw(true);
        glUseProgram(0);



        // draw right to offscreen!
        offscreenRenderPassRight.use();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glViewport(0, 0, 1024, 1024);

        // inpainting
        nddShader.activate();
        nddShader.applyUniforms("right");
        nddShader.applyUniforms("leftEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inpaintingMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, inpaintingMegaTexture.texture);
        glUniform1i(glGetUniformLocation(nddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(nddShader.programID, "depth"), 1);
        inpainting.draw(false);
        nddShader.applyUniforms("rightEye");
        inpainting.draw(false);
        glUseProgram(0);

        // background
        foregroundNddShader.activate();
        foregroundNddShader.applyUniforms("right");
        foregroundNddShader.applyUniforms("leftEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, backgroundMegaTexture.texture);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "depth"), 1);
        background.draw(false);
        foregroundNddShader.applyUniforms("rightEye");
        background.draw(false);
        glUseProgram(0);

        // foreground
        foregroundNddShader.activate();
        foregroundNddShader.applyUniforms("right");
        foregroundNddShader.applyUniforms("leftEye");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, foregroundMegaTexture.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, foregroundMegaTexture.texture);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "tex"), 0);
        glUniform1i(glGetUniformLocation(foregroundNddShader.programID, "depth"), 1);
        foreground.draw(false);
        foregroundNddShader.applyUniforms("rightEye");
        foreground.draw(false);
        glUseProgram(0);



        glActiveTexture(GL_TEXTURE0);

        // left offscreen to screen
        screenRenderPass.use();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glViewport(0, 0, 1024, 1024);
        screenQuadShader.activate();
        glBindTexture(GL_TEXTURE_2D, offscreenRenderPassLeft.target.texture);
        screenQuad.draw();
        glUseProgram(0);


        // right offscreen to screen
        glViewport(1034, 0, 1024, 1024);
        screenQuadShader.activate();
        glBindTexture(GL_TEXTURE_2D, offscreenRenderPassRight.target.texture);
        screenQuad.draw();
        glUseProgram(0);
    }

} // namespace msi_vr