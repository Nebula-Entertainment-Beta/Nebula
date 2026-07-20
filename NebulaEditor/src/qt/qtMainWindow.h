#pragma once

#include "editorApplication.h"

#include <QMainWindow>
#include <QPoint>
#include <QString>
#include <QTimer>
#include <memory>

#include "scriptParams.h"

class QListWidget;
class QPlainTextEdit;
class QDoubleSpinBox;
class QLabel;
class QScrollArea;
class QComboBox;
class QWindow;
class QObject;
class QDockWidget;
class QFormLayout;
class QAbstractNativeEventFilter;

namespace Editor
{
  class QtNodeGraphPanel;

  class QtMainWindow final : public QMainWindow
  {
    Q_OBJECT
  public:
    QtMainWindow(std::unique_ptr<EditorApplication> app, QWidget *parent = nullptr);
    ~QtMainWindow() override;

  private slots:
    void onFrame();
    void onPlay();
    void onStop();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onNewScene();
    void onHierarchySelectionChanged();
    void setTranslateMode();
    void setRotateMode();
    void setScaleMode();

  private:
    void buildUi();
    void buildMenus();
    void applyDarkTheme();
    void embedSceneView();
    void refreshHierarchy();
    void refreshAssets();
    void rebuildInspector();
    void refreshTransformValues();
    void refreshConsole();
    void refreshDebug();
    void rebuildDebugTuning();
    void syncSelectionFromEngine();
    void updateGizmoLabel();
    void showHierarchyContextMenu(const QPoint &position);
    Nebula::Entity selectedEntity() const;

    std::unique_ptr<EditorApplication> m_app;
    QTimer m_timer;
    Nebula::ScriptParams m_scriptParams;

    QListWidget *m_hierarchy = nullptr;
    QListWidget *m_assets = nullptr;
    QComboBox *m_assetFilter = nullptr;
    QPlainTextEdit *m_console = nullptr;
    QPlainTextEdit *m_debugText = nullptr;
    QWidget *m_debugTuningHost = nullptr;
    QFormLayout *m_debugTuningForm = nullptr;
    QtNodeGraphPanel *m_nodes = nullptr;

    QScrollArea *m_inspectorScroll = nullptr;
    QDoubleSpinBox *m_pos[3]{};
    QDoubleSpinBox *m_yaw = nullptr;
    QDoubleSpinBox *m_scale[3]{};

    QLabel *m_gizmoLabel = nullptr;
    QLabel *m_fpsLabel = nullptr;
    QWindow *m_sceneWindow = nullptr;
    QWidget *m_sceneContainer = nullptr;
    QObject *m_sceneInputFilter = nullptr;
    QAbstractNativeEventFilter *m_nativeInputFilter = nullptr;
    QDockWidget *m_inspectorDock = nullptr;

    int m_lastLogCount = 0;
    bool m_blockUiSignals = false;
    float m_fpsEma = 60.f;
    Nebula::Entity m_lastSelected{};
    int m_lastEntityCount = -1;
    int m_lastGizmoMode = -1;
    qint64 m_lastDebugRefreshMs = 0;
    QString m_lastDebugText;
  };

}
