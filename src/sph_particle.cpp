//
// Created by ChenhuiWang on 2024/4/21.

// Copyright (c) 2024 Tencent. All rights reserved.
//



#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Draw.h"
#include <spdlog/spdlog.h>
#include <Eigen/Dense>
#include <partio/src/lib/Partio.h>
#include <partio/src/lib/io/readers.h>

using TV = Eigen::Vector2d;
using TM = Eigen::Matrix2d;

std::vector<TV> gParticles;

void sReadParticles() {
    const char*       filename = "/Users/wangchenhui/Downloads/particlesInfos_21.bgeo";
    std::stringstream errStream;
    auto              particlesData = Partio::readBGEO(filename, false, &errStream);

    int                       num = particlesData->numParticles();
    Partio::ParticleAttribute positionAttr;
    bool        positionFound = particlesData->attributeInfo("position", positionAttr);
    const auto* positions     = particlesData->data<float>(positionAttr, 0);
    for (int i = 0; i < 6527; i++) {
        gParticles.push_back(100.0 * TV{positions[3 * i], positions[3 * i + 1] - 6.0});
    }
    particlesData->release();
}


void sDrawSphParticles() {
    // generate particles

    double           h = 25;
    double           r = 2 * h;
    double  radius = 5;

    if (gParticles.empty()) {

#define bunny
#ifdef bunny
        sReadParticles();
#else

            for (int i = 300; i < 1000; i += 20) {
                for (int j = 50; j < 750; j += 20) {
                    gParticles.emplace_back(i, j);
                }
            }
            h = 50;
            r = 2 * h;
            radius = 8;
#endif
    }
    // draw points
    std::for_each(gParticles.begin(), gParticles.end(), [](const TV& v) {
        gDraw.DrawPoint({v.x(), v.y()}, Vec4(174, 107, 129, 255) / 255.f, 3);
    });
//    return;

    int n = gParticles.size();

    // anistropic kernel
    // x_weight


    std::vector<int> Nei(n, 0);
    std::vector<TV>  x_w(n, TV::Zero());
    auto&            X = gParticles;
    for (int i = 0; i < n; i++) {
        spdlog::info("x_weight: {}", i);
        double w_ij_total = 0.0;
        for (int j = 0; j < n; j++) {
            auto x_ij = X[i] - X[j];
            if (x_ij.norm() < r) {
                double w_ij = 1 - (x_ij.norm() / r) * (x_ij.norm() / r) * (x_ij.norm() / r);
                x_w[i] += w_ij * X[j];
                w_ij_total += w_ij;
                Nei[i]++;
            }
        }
        x_w[i] /= w_ij_total;
    }

    // C
    double kr = 4;
    //    double ks    = 1;
    double kn    = 0.5;
    int    N_eps = 20;

    std::vector<double> ks(n);
    std::vector<TM>     C(n, TM::Zero());
    for (int i = 0; i < n; i++) {
        spdlog::info("C: {}", i);
        double w_ij_total = 0.0;
        for (int j = 0; j < n; j++) {
            auto x_ij = X[i] - X[j];
            if (x_ij.norm() < r) {
                double w_ij = 1 - (x_ij.norm() / r) * (x_ij.norm() / r) * (x_ij.norm() / r);
                C[i] += w_ij * (X[j] - x_w[i]) * (X[j] - x_w[i]).transpose();
                w_ij_total += w_ij;
            }
        }
        C[i] /= w_ij_total;

        ks[i] = sqrt(1.0 / C[i].determinant());
        C[i] *= ks[i];

        if (Nei[i] < N_eps) {
            C[i] = 0.5 * TM::Identity();
        }
    }

    // SVD

    std::vector<TM> Rotate(n);
    std::vector<TV> Scale(n);
    for (int i = 0; i < n; i++) {
        spdlog::info("SVD: {}", i);
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(C[i], Eigen::ComputeThinU | Eigen::ComputeThinV);
        TM                                R     = svd.matrixU();
        TM                                Sigma = svd.singularValues().asDiagonal();
        Rotate[i]                               = R;
        double s0                               = Sigma(0, 0);
        double s1                               = Sigma(1, 1);
        spdlog::info("{}  {}", s0, s1);
        //        s1  = std::max(s1, s0 / kr);
        Scale[i] = {s0, s1};
    }






    // draw spheres
    for (int i = 0; i < n; i++) {
        gDraw.DrawCircle({gParticles[i].x(), gParticles[i].y()},
                         radius,
                         Vec4(241, 239, 236, 255) / 255.f,
                         Scale[i],
                         Rotate[i]);
    }

    gDraw.Flush();
};





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

int main(int argc, char* argv[]) {

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

//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




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

        static int hasDrawFirstFrame = 0;
        if(!hasDrawFirstFrame){
            sDrawSphParticles();
            hasDrawFirstFrame = 1;
        }


        if (false) {
            static std::string buffer;
            buffer = std::to_string(1000.0 * frameTime.count()) + " ms.";
            gDraw.DrawString(Vec2{1350, 800}, buffer, 8.f);
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
