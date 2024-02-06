//
// Created by ChenhuiWang on 2024/2/6.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#ifndef CODEGRAPH_HIERARCHYCALLSTACK_H
#define CODEGRAPH_HIERARCHYCALLSTACK_H

#include "HybridDraw.h"
#include <fstream>
#include <unordered_map>

std::vector<Vec4> gColorPlateDefault{
    Red3, DarkRed, LightRed,Blue6, Blue5, Blue4};
std::vector<Vec4> gColorPlateRed{Red1, Red2, Red3, Red4, Red5};
std::vector<Vec4> gColorPlateBlue{Blue1, Blue2, Blue3, Blue4, Blue5, Blue6};
std::vector<Vec4> gColorPlate = gColorPlateDefault;

static int sCountTabsInLine(const std::string& line) {
    int count = 0;
    for (char c : line) {
        if (c == ' ') {
            count++;
        } else {
            break;
        }
    }
    return count;
}

struct Func {
    Func(const std::string& name, int level)
        : name(name)
        , level(level) {}

    std::string name;
    int         level;
};

class HierarchyCallStack {
public:
    HierarchyCallStack() = default;

    void ReadTxt(const std::string& file, int version = 1);

    void Draw() const;

private:
    std::vector<Func> mFuncs;
};

void HierarchyCallStack::ReadTxt(const std::string& file, int version) {
    std::string   absolutePath = file;
    std::ifstream inputFile(absolutePath);
    if (!inputFile.is_open()) {
        spdlog::error("File not open: {}", absolutePath);
        return;
    }
    std::string line;
    while (std::getline(inputFile, line)) {
        if (version == 1) {
            if (line.empty())
                continue;
            auto level = sCountTabsInLine(line) / 4;
            auto start = std::find_if(line.begin(), line.end(), [](char c) { return c != ' '; });
            mFuncs.emplace_back(line.substr(std::distance(line.begin(), start)), level);
        }
    }
    inputFile.close();
}



void HierarchyCallStack::Draw() const {
    Vec2  startPos = {50, 750};
    Vec2  p        = startPos;
    float width    = 0;
    for (int i = 0; i < mFuncs.size(); i++) {
        int level = mFuncs[i].level;
        width     = sDrawRectText({p.x + (float)level * 25.f, p.y - width * (float)i},
                              mFuncs[i].name,
                              gColorPlate[level % gColorPlate.size()]);
    }
    gDraw.Flush();
}
#endif   // CODEGRAPH_HIERARCHYCALLSTACK_H
