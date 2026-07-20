#include "qt/qtMainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication qtApp(argc, argv);

  Nebula::ApplicationSpec spec;
  spec.title = "Nebula Editor";
  auto editor = std::make_unique<Editor::EditorApplication>(spec);
  Editor::QtMainWindow window(std::move(editor));
  window.show();
  return qtApp.exec();
}
