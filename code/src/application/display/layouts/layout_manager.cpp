#include "layout_manager.h"

LayoutManager::LayoutManager() {
    currentLayout = {
        LAYOUT_MODE_STANDARD,
        0.45f,  // 左侧面板比例 45%（较小）
        0.55f,  // 右侧面板比例 55%（较大）
        12,     // 基础字体大小
        8,      // 元素间距
        false   // 默认不显示边框
    };
}

void LayoutManager::setLayout(LayoutMode mode) {
    currentLayout.screenSize = mode;
}

LayoutConfig LayoutManager::getLayout() const {
    return currentLayout;
}

void LayoutManager::calculateLayout(int width, int height, int& leftPanelWidth, int& rightPanelWidth) {
    // 初始化分屏布局参数 - 根据屏幕宽度动态调整
    // 小屏幕（< 600像素）：左侧面板宽度约为总宽度的1/2
    // 大屏幕（>= 600像素）：左侧面板宽度约为总宽度的1/3
    if (width < 600) {
        leftPanelWidth = width / 2;
    } else {
        leftPanelWidth = width / 3;
    }
    rightPanelWidth = width - leftPanelWidth;
}