//
// Created by ChenhuiWang on 2024/2/4.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#ifndef CODEGRAPH_DRAW_H
#define CODEGRAPH_DRAW_H

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

using Vec2   = glm::vec2;
using Vec4   = glm::vec4;
using Mat4   = glm::mat4x4;
using Color4 = Vec4;

class Camera {
public:
    Camera();

    void BuildProjectionMatrix(Mat4& m, float zBias);
    Vec2 ConvertWorldToScreen(const Vec2& pw);

public:
    Vec2  mCenter;
    float mZoom;
    int   mWidth;
    int   mHeight;
};

class GLRenderPointsImpl;
class GLRenderLinesImpl;
class GLRenderTrianglesImpl;

class Draw {
    /// use p-impl to reduce build dependency
public:
    Draw();

    void Create();

    void Destroy();

    void DrawPoint(const Vec2& p, const Color4& color, float size);

    void DrawNativeLine(const Vec2& p0, const Vec2& p1, const Vec4& color);

    void DrawPolygon(const std::vector<Vec2>& vertices, const Vec4& color);

    void DrawCircle(const Vec2& center, float radius, const Vec4& color);

    void DrawString(const Vec2& p, const std::string& str, int fontSize = 14,
                    const Color4& color = {230, 153, 153, 255});

    void Flush();



public:
    std::unique_ptr<GLRenderPointsImpl>    mPointsImpl;
    std::unique_ptr<GLRenderLinesImpl>     mLinesImpl;
    std::unique_ptr<GLRenderTrianglesImpl> mTrianglesImpl;
};

extern Camera gCamera;
extern Draw   gDraw;

#endif   // CODEGRAPH_DRAW_H
