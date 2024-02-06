//
// Created by ChenhuiWang on 2024/2/6.

// Copyright (c) 2024 Tencent. All rights reserved.
//

#ifndef CODEGRAPH_HIERARCHYCALLSTACK_H
#define CODEGRAPH_HIERARCHYCALLSTACK_H

#include "HybridDraw.h"
#include <fstream>

std::vector<Vec4> gColorPlate{DarkRed, DarkOrange, DarkGreen, DarkBlue, LightRed, LightOrange};

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

class HierarchyCallStack {
public:
    HierarchyCallStack() { mFuncs.resize(mMaxStack); }

    void ReadTxt(const std::string& file, int version = 1);

    void Add(const std::string& func);

    void In();

    void Out();

    void Draw() const;

private:
    int                                   mMaxStack     = 64;
    int                                   mCurrentLevel = 0;
    int                                   mMaxLevel     = -1;
    std::vector<std::vector<std::string>> mFuncs;
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
        if (version == 0) {
            if (line == "in") {
                In();
            } else if (line == "out") {
                Out();
            } else {
                Add(line);
            }
        } else if (version == 1) {
            auto level = sCountTabsInLine(line) / 4;
            auto start = std::find_if(line.begin(), line.end(), [](char c) { return c != ' '; });
            mFuncs[level].push_back(line.substr(std::distance(line.begin(), start)));
            mMaxLevel = std::max(mMaxLevel, level);
        }
    }
    inputFile.close();
}

void HierarchyCallStack::Add(const std::string& func) {
    mFuncs[mCurrentLevel].push_back(func);
    mMaxLevel = std::max(mMaxLevel, mCurrentLevel);
}

void HierarchyCallStack::In() {
    mCurrentLevel++;
}

void HierarchyCallStack::Out() {
    mCurrentLevel--;
    if (mCurrentLevel == -1) {
        spdlog::error("mCurrentLevel == -1");
        assert(false);
    }
}

void HierarchyCallStack::Draw() const {
    Vec2  startPos = {50, 750};
    Vec2  p        = startPos;
    float width    = 0;
    int   totalNum = 0;
    for (int level = 0; level < mMaxLevel; level++) {
        for (const auto& func : mFuncs[level]) {
            width = sDrawRectText({p.x + (float)level * 25.f, p.y - width * (float)totalNum},
                                  func,
                                  gColorPlate[level % gColorPlate.size()]);

            totalNum++;
        }
    }
    gDraw.Flush();
}
#endif   // CODEGRAPH_HIERARCHYCALLSTACK_H
