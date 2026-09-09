#pragma once

#include <axmol.h>

class EntryScene : public ax::Scene {
public:
    bool init() override;

private:
    void onOpenKgtClicked();
    void onFileSelected(const std::string& fullPath);

    bool rememberChecked = false;
    ax::Label* openBtnLabel = nullptr;
    ax::Label* checkboxLabel = nullptr;
    ax::Label* backBtnLabel = nullptr;  // 返回菜单按钮
};
