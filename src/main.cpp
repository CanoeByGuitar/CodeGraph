//
// Created by ChenhuiWang on 2024/2/4.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "HybridDraw.h"
#include <spdlog/spdlog.h>



std::vector<ImFont*> gFonts(30, nullptr);
GLFWwindow*          gMainWindow   = nullptr;
static float         sDisplayScale = 1.f;

static void sCreateUI(GLFWwindow* window, const char* glslVersion = nullptr) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    bool success;
    success = ImGui_ImplGlfw_InitForOpenGL(window, false);
    if (!success) {
        spdlog::error("ImGui_ImplGlfw_InitForOpenGL failed");
        assert(false);
    }

    success = ImGui_ImplOpenGL3_Init(glslVersion);
    if (!success) {
        spdlog::error("ImGui_ImplOpenGL3_Init failed");
        assert(false);
    }

    // Add font
    std::string rootPath = CURRENT_PROJECT_PATH;
    std::string fontPath = rootPath + "/resources/droid_sans.ttf";
    spdlog::debug("font path: {}", fontPath);

    for (int i = 2; i < 25; i++) {
        gFonts[i] =
            ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str(), (float)i * sDisplayScale);
    }
}

static void UpdateUI() {}

int main() {
    spdlog::set_level(spdlog::level::info);

    gCamera.mWidth  = 1470;
    gCamera.mHeight = 816;

    if (glfwInit() == 0) {
        spdlog::error("Failed to initialize GLFW");
        return -1;
    }

#if __APPLE__
    const char* glslVersion = "#version 150";
#else
    const char* glslVersion = NULL;
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    gMainWindow = glfwCreateWindow(gCamera.mWidth, gCamera.mHeight, "Title", nullptr, nullptr);
    if (!gMainWindow) {
        glfwTerminate();
        return -1;
    }

    glfwGetWindowContentScale(gMainWindow, &sDisplayScale, &sDisplayScale);
    glfwMakeContextCurrent(gMainWindow);

    int version = gladLoadGL(glfwGetProcAddress);
    printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    printf(
        "OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    gDraw.Create();

    sCreateUI(gMainWindow, glslVersion);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glDisable(GL_DEPTH_TEST);


    std::chrono::duration<double> frameTime(0.0);
    std::chrono::duration<double> sleepAdjust(0.0);


    while (!glfwWindowShouldClose(gMainWindow)) {
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        glfwGetWindowSize(gMainWindow, &gCamera.mWidth, &gCamera.mHeight);
        spdlog::debug("width {}  height {}", gCamera.mWidth, gCamera.mHeight);
        int bufferWidth, bufferHeight;
        glfwGetFramebufferSize(gMainWindow, &bufferWidth, &bufferHeight);
        glViewport(0, 0, bufferWidth, bufferHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(float(gCamera.mWidth), float(gCamera.mHeight)));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Overlay",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::End();


        UpdateUI();

        if (true) {
            sDrawRectText({50, 700}, "static void sDrawRectText(const Vec2& p, ...)");
            //            gDraw.DrawPoint({500, 500}, {255, 0, 0, 1}, 100);
            gDraw.Flush();
        }

        if (true) {
            static std::string buffer;
            buffer = std::to_string(1000.0 * frameTime.count()) + " ms.";
            gDraw.DrawString(Vec2{0, 0}, buffer, 5.f);
        }



        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(gMainWindow);
        glfwPollEvents();

        // Throttle to cap at 60Hz. This adaptive using a sleep adjustment. This could be improved
        // by using mm_pause or equivalent for the last millisecond.
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double>         target(1.0 / 60.0);
        std::chrono::duration<double>         timeUsed  = t2 - t1;
        std::chrono::duration<double>         sleepTime = target - timeUsed + sleepAdjust;
        if (sleepTime > std::chrono::duration<double>(0)) {
            std::this_thread::sleep_for(sleepTime);
        }
        std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
        frameTime                                = t3 - t1;
        spdlog::debug("SleepTime: {} ms,  frameTime: {} ms",
                      1000.0 * sleepTime.count(),
                      1000.0 * frameTime.count());


        // Compute the sleep adjustment using a low pass filter
        sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
    }

    gDraw.Destroy();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();

    return 0;
}