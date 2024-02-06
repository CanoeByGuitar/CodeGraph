//
// Created by ChenhuiWang on 2024/2/5.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#ifndef CODEGRAPH_HYBRIDDRAW_H
#define CODEGRAPH_HYBRIDDRAW_H

#include "Draw.h"
#include <spdlog/spdlog.h>

#define DarkRed     Vec4(230, 153, 153, 255) / 255.f
#define DarkOrange   Vec4(204, 204, 0, 255) / 255.f
#define DarkGreen   Vec4(102, 204, 0, 255) / 255.f
#define DarkBlue    Vec4(0.0, 153, 153, 255) / 255.f
#define LightRed    Vec4(255, 204, 204, 255) / 255.f
#define LightOrange    Vec4(255, 204, 153, 255) / 255.f


#define Red1    Vec4(255, 0, 0, 255) / 255.f
#define Red2    Vec4(255, 51, 51, 255) / 255.f
#define Red3    Vec4(255, 102, 102, 255) / 255.f
#define Red4    Vec4(255, 153, 153, 255) / 255.f
#define Red5    Vec4(255, 204, 204, 255) / 255.f


#define Blue1    Vec4(174, 107, 129, 255) / 255.f
#define Blue2    Vec4(130, 45, 74, 255) / 255.f
#define Blue3    Vec4(71, 85, 142, 255) / 255.f
#define Blue4    Vec4(105, 130, 185, 255) / 255.f
#define Blue5    Vec4(188, 202, 224, 255) / 255.f
#define Blue6    Vec4(241, 239, 236, 255) / 255.f


static float sDrawRectText(const Vec2& p, const std::string& text, const Color4& color = DarkRed,
                           int fontSize = 10) {
    Vec2 lower = {p.x - 5, p.y - 2.2 * (float)fontSize};
    Vec2 upper = {p.x + (float)fontSize * (float)text.size() * 1, p.y + (float)fontSize * 0.1};
    static int flag = 0;
    if (!flag) {
        spdlog::debug("{} {} {} {}", lower.x, lower.y, upper.x, upper.y);
        flag = 1;
    }
    std::vector<Vec2> points = {lower, {upper.x, lower.y}, upper, {lower.x, upper.y}};
    gDraw.DrawPolygon(points, color);
    //    int fontSize = static_cast<int>((upper.y - lower.y) * 0.8);

    gDraw.DrawString(p, text, fontSize, color);

    return upper.y - lower.y;
}


#endif   // CODEGRAPH_HYBRIDDRAW_H
