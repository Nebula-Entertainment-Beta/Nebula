#import <AppKit/AppKit.h>

#include "platform/fileDialog.h"

#include <string>

namespace Editor
{
  namespace
  {
    std::string runPanel(NSSavePanel *panel)
    {
      if ([panel runModal] != NSModalResponseOK)
      {
        return {};
      }
      NSURL *url = panel.URL;
      if (url == nil)
      {
        return {};
      }
      return std::string(url.path.UTF8String);
    }

    void applyExtensions(NSSavePanel *panel, const std::vector<std::string> &extensions)
    {
      if (extensions.empty())
      {
        return;
      }
      NSMutableArray<NSString *> *types = [NSMutableArray array];
      for (const std::string &ext : extensions)
      {
        [types addObject:[NSString stringWithUTF8String:ext.c_str()]];
      }
      panel.allowedFileTypes = types;
    }
  }

  std::string openFileDialog(const char *title, const std::vector<std::string> &extensions)
  {
    @autoreleasepool
    {
      NSOpenPanel *panel = [NSOpenPanel openPanel];
      panel.title = [NSString stringWithUTF8String:title];
      panel.canChooseFiles = YES;
      panel.canChooseDirectories = NO;
      panel.allowsMultipleSelection = NO;
      applyExtensions(panel, extensions);
      return runPanel(panel);
    }
  }

  std::string saveFileDialog(const char *title, const std::string &defaultName,
                             const std::vector<std::string> &extensions)
  {
    @autoreleasepool
    {
      NSSavePanel *panel = [NSSavePanel savePanel];
      panel.title = [NSString stringWithUTF8String:title];
      if (!defaultName.empty())
      {
        panel.nameFieldStringValue = [NSString stringWithUTF8String:defaultName.c_str()];
      }
      applyExtensions(panel, extensions);
      return runPanel(panel);
    }
  }

} // namespace Editor
