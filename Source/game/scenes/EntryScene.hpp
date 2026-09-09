#pragma once

#include <axmol.h>

class EntryScene : public ax::Scene {
public:
    bool init() override;

private:
    void onOpenKgtClicked();
    void onFileSelected(const std::string &fullPath);
    void onConfirmClicked();

    bool rememberChecked = false;
    bool hasPendingSelection = false;
    std::string pendingBasePath;
    std::string pendingKgtName;

    ax::Label *openBtnLabel = nullptr;
    ax::Label *checkboxLabel = nullptr;
    ax::Label *backBtnLabel = nullptr;     // 返回菜单按钮
    ax::Label *pathLabel = nullptr;        // 已选择文件的路径显示
    ax::Label *confirmBtnLabel = nullptr;  // 确认加载按钮
};
