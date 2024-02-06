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




static float sDrawRectText(const Vec2& p, const std::string& text, const Color4& color = DarkRed,
                           int fontSize = 10) {
    Vec2 lower = {p.x - 5, p.y - 2 * (float)fontSize};
    Vec2 upper = {p.x + (float)fontSize * (float)text.size() * 1, p.y + (float)text.size() * 0.1};
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
