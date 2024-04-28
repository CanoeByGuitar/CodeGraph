//
// Created by ChenhuiWang on 2024/2/4.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#include "Draw.h"
#include <spdlog/spdlog.h>
#include <imgui/imgui.h>

#define BUFFER_OFFSET(x) ((const void*)(x))

Camera gCamera;
Draw   gDraw;

extern std::vector<ImFont*> gFonts;
Camera::Camera() {
    mWidth  = 1470;
    mHeight = 816;
    mCenter = Vec2{mWidth / 2, mHeight / 2};
    mZoom   = 1.f;
}

// Convert from world coordinates to normalized device coordinates.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
void Camera::BuildProjectionMatrix(Mat4& mat, float zBias) {
    auto  w     = static_cast<float>(mWidth);
    auto  h     = static_cast<float>(mHeight);
    float ratio = w / h;
//    Vec2  extents(ratio * 25.f, 25.f);
    Vec2 extents{w / 2 , h / 2 };
    extents *= mZoom;

    Vec2 lower = mCenter - extents;
    Vec2 upper = mCenter + extents;


    // l = lower.x      r = upper.x
    // b = lower.y      r = upper.y
    // n = 1            f = -1
    float m[16];
    m[0] = 2.0f / (upper.x - lower.x);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = 2.0f / (upper.y - lower.y);
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8]  = 0.0f;
    m[9]  = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;

    m[12] = -(upper.x + lower.x) / (upper.x - lower.x);
    m[13] = -(upper.y + lower.y) / (upper.y - lower.y);
    m[14] = 0;
    m[15] = 1.0f;

    mat = glm::make_mat4(m);
}

Vec2 Camera::ConvertWorldToScreen(const Vec2& pw) {
    auto  w     = float(mWidth);
    auto  h     = float(mHeight);
    float ratio = w / h;
//    Vec2  extents(ratio * 25.0f, 25.0f);
    Vec2  extents(w / 2, h / 2);
    extents *= mZoom;

    Vec2 lower = mCenter - extents;
    Vec2 upper = mCenter + extents;

    float u = (pw.x - lower.x) / (upper.x - lower.x);
    float v = (pw.y - lower.y) / (upper.y - lower.y);

    Vec2 ps;
    ps.x = u * w;
    ps.y = (1.0f - v) * h;
    return ps;
}

static void sPrintLog(GLuint object) {
    GLint logLength = 0;
    if (glIsShader(object)) {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
    } else if (glIsProgram(object)) {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
    } else {
        spdlog::error("printLog: Not a shader or a program\n");
        return;
    }

    char* log = static_cast<char*>(malloc(logLength));

    if (glIsShader(object))
        glGetShaderInfoLog(object, logLength, nullptr, log);
    else if (glIsProgram(object)) {
        glGetProgramInfoLog(object, logLength, nullptr, log);
    }

    spdlog::error("{}", log);
    free(log);
}

static GLuint sCreateShaderFromString(const char* source, GLenum type) {
    GLuint      res       = glCreateShader(type);
    const char* sources[] = {source};
    glShaderSource(res, 1, sources, nullptr);
    glCompileShader(res);
    GLint compileOk = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compileOk);
    if (compileOk == GL_FALSE) {
        spdlog::error("Error compiling shader of type {}!", type);
        sPrintLog(res);
        glDeleteShader(res);
        return 0;
    }
    return res;
}

static GLuint sCreateShaderProgram(const char* vs, const char* fs) {
    GLuint vsId = sCreateShaderFromString(vs, GL_VERTEX_SHADER);
    GLuint fsId = sCreateShaderFromString(fs, GL_FRAGMENT_SHADER);
    assert(vsId != 0 && fsId != 0);

    GLuint programId = glCreateProgram();
    glAttachShader(programId, vsId);
    glAttachShader(programId, fsId);
    glBindFragDataLocation(programId, 0, "color");
    glLinkProgram(programId);

    glDeleteShader(vsId);
    glDeleteShader(fsId);

    GLint status = GL_FALSE;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    assert(status != GL_FALSE);

    return programId;
}

static void sCheckGLError() {
    GLenum errCode = glGetError();
    if (errCode != GLFW_NO_ERROR) {
        spdlog::error("OpenGL error = {}", errCode);
        assert(false);
    }
}

template<typename T> static void sSetUniform(GLint location, const T& value) {
    if constexpr (std::is_same_v<T, Mat4>) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, Vec2>) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, float>) {
        glUniform1f(location, value);
    }
}
template void sSetUniform<Mat4>(GLint, const Mat4&);
template void sSetUniform<Vec2>(GLint, const Vec2&);
template void sSetUniform<float>(GLint, const float&);


class GLRenderPointsImpl {
    /// Use a buffer size = mMaxVertices to avoid of too many draw call
public:
    void Create() {
        mVertices.resize(mMaxVertices);
        mColors.resize(mMaxVertices);
        mSizes.resize(mMaxVertices);

        const char* vs = "#version 330\n"
                         "uniform mat4 projectionMatrix;\n"
                         "layout(location = 0) in vec2 v_position;\n"
                         "layout(location = 1) in vec4 v_color;\n"
                         "layout(location = 2) in float v_size;\n"
                         "out vec4 f_color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	f_color = v_color;\n"
                         "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
                         "   gl_PointSize = v_size;\n"
                         "}\n";

        const char* fs = "#version 330\n"
                         "in vec4 f_color;\n"
                         "out vec4 color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	color = f_color;\n"
                         "}\n";

        mProgramId         = sCreateShaderProgram(vs, fs);
        mProjectionUniform = glGetUniformLocation(mProgramId, "projectionMatrix");
        mVertexAttribute   = 0;
        mColorAttribute    = 1;
        mSizeAttribute     = 2;

        // Generate
        glGenVertexArrays(1, &mVaoId);
        glGenBuffers(3, mVboIds);

        glBindVertexArray(mVaoId);
        glEnableVertexAttribArray(mVertexAttribute);
        glEnableVertexAttribArray(mColorAttribute);
        glEnableVertexAttribArray(mSizeAttribute);

        // Vertex buffer
        // allocate the max size of VBO when created
        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glVertexAttribPointer(mVertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Vec2) * mMaxVertices, mVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glVertexAttribPointer(mColorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Color4) * mMaxVertices, mColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[2]);
        glVertexAttribPointer(mSizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(float) * mMaxVertices, mSizes.data(), GL_DYNAMIC_DRAW);

        sCheckGLError();

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        mCount = 0;
    }

    void Destroy() {
        if (mVaoId) {
            glDeleteVertexArrays(1, &mVaoId);
            glDeleteBuffers(3, mVboIds);
            mVaoId = 0;
        }

        if (mProgramId) {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }
    }

    void AddVertex(const Vec2& v, const Color4& c, float size) {
        if (mCount == mMaxVertices) {
            Flush();
        }
        mVertices[mCount] = v;
        mColors[mCount]   = c;
        mSizes[mCount]    = size;
        mCount++;
    }

    void Flush() {
        if (mCount == 0)
            return;

        glUseProgram(mProgramId);

        Mat4 proj;
        gCamera.BuildProjectionMatrix(proj, 0.f);

        sSetUniform(mProjectionUniform, proj);

        glBindVertexArray(mVaoId);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Vec2), mVertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Vec4), mColors.data());

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(float), mSizes.data());

        glEnable(GL_PROGRAM_POINT_SIZE);
        glDrawArrays(GL_POINTS, 0, mCount);
        glDisable(GL_PROGRAM_POINT_SIZE);

        sCheckGLError();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        mCount = 0;
    }


public:
    std::vector<Vec2>   mVertices;
    std::vector<Color4> mColors;
    std::vector<float>  mSizes;
    int                 mCount;
    int                 mMaxVertices = 512;
    GLuint              mVaoId;
    GLuint              mVboIds[3];
    GLuint              mProgramId;
    GLint               mProjectionUniform;
    GLint               mVertexAttribute;
    GLint               mColorAttribute;
    GLint               mSizeAttribute;
};

class GLRenderLinesImpl {
    /// Use a buffer size = mMaxVertices to avoid of too many draw call
public:
    void Create() {
        mVertices.resize(mMaxVertices);
        mColors.resize(mMaxVertices);

        const char* vs = "#version 330\n"
                         "uniform mat4 projectionMatrix;\n"
                         "layout(location = 0) in vec2 v_position;\n"
                         "layout(location = 1) in vec4 v_color;\n"
                         "out vec4 f_color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	f_color = v_color;\n"
                         "	gl_Position =  projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
                         "}\n";

        const char* fs = "#version 330\n"
                         "in vec4 f_color;\n"
                         "out vec4 color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	color = f_color;\n"
                         "}\n";

        mProgramId         = sCreateShaderProgram(vs, fs);
        mVertexAttribute   = 0;
        mColorAttribute    = 1;

        // Generate
        glGenVertexArrays(1, &mVaoId);
        glGenBuffers(2, mVboIds);

        glBindVertexArray(mVaoId);
        glEnableVertexAttribArray(mVertexAttribute);
        glEnableVertexAttribArray(mColorAttribute);

        // Vertex buffer
        // allocate the max size of VBO when created
        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glVertexAttribPointer(mVertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Vec2) * mMaxVertices, mVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glVertexAttribPointer(mColorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Color4) * mMaxVertices, mColors.data(), GL_DYNAMIC_DRAW);

        sCheckGLError();

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        mCount = 0;
    }

    void Destroy() {
        if (mVaoId) {
            glDeleteVertexArrays(1, &mVaoId);
            glDeleteBuffers(2, mVboIds);
            mVaoId = 0;
        }

        if (mProgramId) {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }
    }

    void AddVertex(const Vec2& v, const Color4& c) {
        if (mCount == mMaxVertices) {
            Flush();
        }
        mVertices[mCount] = v;
        mColors[mCount]   = c;
        mCount++;
    }

    void Flush() {
        if (mCount == 0)
            return;

        glUseProgram(mProgramId);

        Mat4 proj;
        gCamera.BuildProjectionMatrix(proj, 0.f);

        sSetUniform(mProjectionUniform, proj);

        glBindVertexArray(mVaoId);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Vec2), mVertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Color4), mColors.data());

        spdlog::debug("color: {} {} {} {}", mColors[1].x,  mColors[1].y,  mColors[1].z,  mColors[1].w);
        glDrawArrays(GL_LINES, 0, mCount);
        sCheckGLError();

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        mCount = 0;
    }


public:
    std::vector<Vec2>   mVertices;
    std::vector<Color4> mColors;
    int                 mCount;
    int                 mMaxVertices = 2 * 512;
    GLuint              mVaoId;
    GLuint              mVboIds[2];
    GLuint              mProgramId;
    GLint               mProjectionUniform;
    GLint               mVertexAttribute;
    GLint               mColorAttribute;
};

class GLRenderTrianglesImpl {
    /// Use a buffer size = mMaxVertices to avoid of too many draw call
public:
    void Create() {
        const char* vs = "#version 330\n"
                         "uniform mat4 projectionMatrix;\n"
                         "layout(location = 0) in vec2 v_position;\n"
                         "layout(location = 1) in vec4 v_color;\n"
                         "out vec4 f_color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	f_color = v_color;\n"
                         "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
                         "}\n";

        const char* fs = "#version 330\n"
                         "in vec4 f_color;\n"
                         "out vec4 color;\n"
                         "void main(void)\n"
                         "{\n"
                         "	color = f_color;\n"
                         "}\n";

        mProgramId         = sCreateShaderProgram(vs, fs);
        mProjectionUniform = glGetUniformLocation(mProgramId, "projectionMatrix");
        mVertexAttribute   = 0;
        mColorAttribute    = 1;

        // Generate
        glGenVertexArrays(1, &mVaoId);
        glGenBuffers(2, mVboIds);

        glBindVertexArray(mVaoId);
        glEnableVertexAttribArray(mVertexAttribute);
        glEnableVertexAttribArray(mColorAttribute);

        // Vertex buffer
        // allocate the max size of VBO when created
        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glVertexAttribPointer(mVertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Vec2) * mMaxVertices, mVertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glVertexAttribPointer(mVertexAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
        glBufferData(
            GL_ARRAY_BUFFER, (int)sizeof(Color4) * mMaxVertices, mColors.data(), GL_DYNAMIC_DRAW);

        sCheckGLError();

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        mCount = 0;

        mVertices.resize(mMaxVertices);
        mColors.resize(mMaxVertices);
    }

    void Destroy() {
        if (mVaoId) {
            glDeleteVertexArrays(1, &mVaoId);
            glDeleteBuffers(3, mVboIds);
            mVaoId = 0;
        }

        if (mProgramId) {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }
    }

    void AddVertex(const Vec2& v, const Color4& c) {
        if (mCount == mMaxVertices) {
            Flush();
        }
        mVertices[mCount] = v;
        mColors[mCount]   = c;
        mCount++;
    }

    void Flush() {
        if (mCount == 0)
            return;

        glUseProgram(mProgramId);

        Mat4 proj;
        gCamera.BuildProjectionMatrix(proj, 0.f);

        sSetUniform(mProjectionUniform, proj);

        glBindVertexArray(mVaoId);

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Vec2), mVertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * (int)sizeof(Color4), mColors.data());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES, 0, mCount);
        glDisable(GL_BLEND);

        sCheckGLError();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        mCount = 0;
    }


public:
    std::vector<Vec2>   mVertices;
    std::vector<Color4> mColors;
    int                 mCount;
    int                 mMaxVertices = 3 * 512;
    GLuint              mVaoId;
    GLuint              mVboIds[2];
    GLuint              mProgramId;
    GLint               mProjectionUniform;
    GLint               mVertexAttribute;
    GLint               mColorAttribute;
};


Draw::Draw() {
    mPointsImpl    = nullptr;
    mLinesImpl     = nullptr;
    mTrianglesImpl = nullptr;
}

void Draw::Create() {
    mPointsImpl = std::make_unique<GLRenderPointsImpl>();
    mPointsImpl->Create();

    mLinesImpl = std::make_unique<GLRenderLinesImpl>();
    mLinesImpl->Create();

    mTrianglesImpl = std::make_unique<GLRenderTrianglesImpl>();
    mTrianglesImpl->Create();
}

void Draw::Destroy() {
    mPointsImpl->Destroy();
    mLinesImpl->Destroy();
    mTrianglesImpl->Destroy();
}

void Draw::DrawPoint(const Vec2& p, const Color4& color, float size){
    mPointsImpl->AddVertex(p, color, size);
}

void Draw::DrawNativeLine(const Vec2& p0, const Vec2& p1, const Vec4& color) {
    mLinesImpl->AddVertex(p0, color);
    mLinesImpl->AddVertex(p1, color);
}

void Draw::DrawPolygon(const std::vector<Vec2>& vertices, const Vec4& color) {
    int n = static_cast<int>(vertices.size());
    for (int i = 0; i < vertices.size(); i++) {
        mLinesImpl->AddVertex(vertices[i], color);
        mLinesImpl->AddVertex(vertices[(i + 1) % n], color);
    }
}

void Draw::DrawCircle(const Vec2& center, float radius, const Vec4& color, const TV& scale, const TM& rotate) {
    int segment = 36;
    for(int i = 0; i < segment; i++){
        double alpha_0 = (360.0 / segment * i) * 3.14 / 180.0;
        double alpha_1 = (360.0 / segment * ((i + 1) % segment)) * 3.14 / 180.0;

        Vec2 x0 = Vec2{radius * cos(alpha_0), radius * sin(alpha_0)} ;
        Vec2 x1 = Vec2{radius * cos(alpha_1), radius * sin(alpha_1)};

        glm::mat2 scaleMatrix = {scale.x(), 0, 0, scale.y()};

        Eigen::Rotation2Dd rotation(rotate);
        double theta = -rotation.angle();
        glm::mat2 rotationMatrix = {cos(theta), -sin(theta), sin(theta), cos(theta)};


        mLinesImpl->AddVertex(rotationMatrix * scaleMatrix * x0 + center, color);
        mLinesImpl->AddVertex(rotationMatrix * scaleMatrix * x1 + center, color);
    }
}

void Draw::DrawString(const Vec2& p,
                      const std::string& str,
                      int fontSize,
                      const Color4& color) {
    auto ps = gCamera.ConvertWorldToScreen(p);
    if(!gFonts[fontSize]){
        spdlog::info("No fontSize = {}. Use default fontSize = 14!", fontSize);
        ImGui::PushFont(gFonts[14]);
    }else{
        ImGui::PushFont(gFonts[fontSize]);
    }
    ImGui::Begin("Overlay",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(ps.x, ps.y));
    ImGui::TextColored(ImColor(color.x , color.y, color.z, color.w),
                       "%s", str.c_str());
    ImGui::End();
    ImGui::PopFont();
}


void Draw::Flush() {
    mTrianglesImpl->Flush();
    mLinesImpl->Flush();
    mPointsImpl->Flush();
}
