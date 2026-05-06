#pragma once

#include <string>

class NativeFileDialog {
public:
    /// Opens a native file selection dialog.
    /// @param title Dialog window title
    /// @param filterDesc Filter description (e.g. "KGT Files")
    /// @param filterExt Filter extension pattern (e.g. "*.kgt")
    /// @return Full file path selected, or empty string if cancelled.
    static std::string openFileDialog(const std::string& title, const std::string& filterDesc, const std::string& filterExt);
};
