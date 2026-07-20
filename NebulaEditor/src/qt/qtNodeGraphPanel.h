#pragma once

#include <QWidget>
#include <string>

namespace Editor
{

  /** Minimal Qt node-graph authoring panel (events / math / actions). */
  class QtNodeGraphPanel final : public QWidget
  {
    Q_OBJECT
  public:
    explicit QtNodeGraphPanel(QWidget *parent = nullptr);

    bool loadGraph(const std::string &logicalPath);
    bool saveGraph(const std::string &logicalPath) const;
    const std::string &graphJson() const { return m_graphJson; }

  signals:
    void graphChanged();

  private:
    std::string m_graphJson;
  };

}
