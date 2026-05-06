#include "engine/NativeFileDialog.hpp"
#import <AppKit/AppKit.h>
#include <string>

std::string NativeFileDialog::openFileDialog(const std::string& title, const std::string& filterDesc, const std::string& filterExt) {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setTitle:[NSString stringWithUTF8String:title.c_str()]];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        // Set allowed file types
        if (@available(macOS 11.0, *)) {
            UTType* kgtType = [UTType typeWithFilenameExtension:@"kgt"];
            if (kgtType) {
                [panel setAllowedContentTypes:@[kgtType]];
            }
        } else {
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [panel setAllowedFileTypes:@[@"kgt"]];
            #pragma clang diagnostic pop
        }

        NSModalResponse response = [panel runModal];
        if (response == NSModalResponseOK) {
            NSURL* url = [[panel URLs] firstObject];
            if (url) {
                return std::string([[url path] UTF8String]);
            }
        }
    }
    return {};
}
