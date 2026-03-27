#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <Arduino.h>

enum LayoutMode {
    LAYOUT_MODE_STANDARD,
    LAYOUT_MODE_COMPACT,
    LAYOUT_MODE_EXPANDED
};

struct LayoutConfig {
    LayoutMode screenSize;
    float leftPanelRatio;
    float rightPanelRatio;
    int baseFontSize;
    int elementSpacing;
    bool showBorders;
};

class LayoutManager {
private:
    LayoutConfig currentLayout;

public:
    LayoutManager();
    void setLayout(LayoutMode mode);
    LayoutConfig getLayout() const;
    void calculateLayout(int width, int height, int& leftPanelWidth, int& rightPanelWidth);
};

#endif // LAYOUT_MANAGER_H