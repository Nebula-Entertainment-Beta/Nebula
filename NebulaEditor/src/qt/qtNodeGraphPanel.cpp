#include "qt/qtNodeGraphPanel.h"

#include <QLabel>
#include <QVBoxLayout>
#include <fstream>
#include <sstream>

namespace Editor
{

  QtNodeGraphPanel::QtNodeGraphPanel(QWidget *parent) : QWidget(parent)
  {
    m_graphJson = R"({
  "version": 1,
  "nodes": [
    {"id": 1, "type": "Event", "name": "OnObjectiveReady"},
    {"id": 2, "type": "Action", "name": "CompleteObjective"}
  ],
  "links": [{"from": 1, "to": 2}]
})";
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        "Node Graph MVP\n"
        "Default graph: OnObjectiveReady -> CompleteObjective\n"
        "Edit/save via graph JSON assets under assets/graphs/",
        this));
  }

  bool QtNodeGraphPanel::loadGraph(const std::string &logicalPath)
  {
    std::ifstream in(logicalPath);
    if (!in)
    {
      return false;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    m_graphJson = ss.str();
    emit graphChanged();
    return true;
  }

  bool QtNodeGraphPanel::saveGraph(const std::string &logicalPath) const
  {
    std::ofstream out(logicalPath);
    if (!out)
    {
      return false;
    }
    out << m_graphJson;
    return true;
  }

}
