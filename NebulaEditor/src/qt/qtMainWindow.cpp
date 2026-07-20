#include "qt/qtMainWindow.h"
#include "qt/qtNodeGraphPanel.h"
#include "qt/qtSceneViewEmbed.h"

#include <QAbstractSpinBox>
#include <QAction>
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>
#include <QWindow>
#include <QMenu>
#include <QMimeData>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

#include "component.h"
#include "collider_fit.h"
#include "tag_component.h"
#include "physics/physics_component.h"
#include "prefabInstance.h"
#include "prefabService.h"

#include <algorithm>

namespace Editor
{
  namespace
  {
    class AssetListWidget final : public QListWidget
    {
    public:
      using QListWidget::QListWidget;

    protected:
      QStringList mimeTypes() const override
      {
        return {"application/x-nebula-asset"};
      }

      QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override
      {
        auto *mime = new QMimeData();
        if (!items.empty())
        {
          const auto *item = items.front();
          const QString payload =
              item->data(Qt::UserRole).toString() + "\n" +
              QString::number(item->data(Qt::UserRole + 1).toInt());
          mime->setData("application/x-nebula-asset", payload.toUtf8());
        }
        return mime;
      }
    };

    class SceneInputFilter final : public QObject, public QAbstractNativeEventFilter
    {
    public:
      SceneInputFilter(EditorApplication &app, QWindow &window, WId nativeHandle,
                       QObject *parent)
          : QObject(parent), m_app(app), m_window(window), m_nativeHandle(nativeHandle)
      {
      }

      bool nativeEventFilter(const QByteArray &, void *message, qintptr *) override
      {
#ifdef _WIN32
        auto *msg = static_cast<MSG *>(message);
        if (reinterpret_cast<WId>(msg->hwnd) != m_nativeHandle)
        {
          return false;
        }
        RECT rect{};
        GetClientRect(msg->hwnd, &rect);
        const int width = std::max(1L, rect.right - rect.left);
        const int height = std::max(1L, rect.bottom - rect.top);
        const float x = static_cast<float>(GET_X_LPARAM(msg->lParam));
        const float y = static_cast<float>(GET_Y_LPARAM(msg->lParam));
        switch (msg->message)
        {
        case WM_LBUTTONDOWN:
          SetFocus(msg->hwnd);
          SetCapture(msg->hwnd);
          m_app.scenePointerPress(x, y, true, false,
                                  (GetKeyState(VK_SHIFT) & 0x8000) != 0,
                                  width, height);
          // Edit mode: consume so GLFW does not also apply mouse edges.
          return !m_app.playing();
        case WM_RBUTTONDOWN:
          SetFocus(msg->hwnd);
          SetCapture(msg->hwnd);
          m_app.scenePointerPress(x, y, false, true,
                                  (GetKeyState(VK_SHIFT) & 0x8000) != 0,
                                  width, height);
          return !m_app.playing();
        case WM_MOUSEMOVE:
          m_app.scenePointerMove(x, y, (msg->wParam & MK_LBUTTON) != 0,
                                 (msg->wParam & MK_RBUTTON) != 0, width, height);
          return !m_app.playing();
        case WM_LBUTTONUP:
          m_app.scenePointerRelease(x, y, true, false, width, height);
          if ((msg->wParam & (MK_LBUTTON | MK_RBUTTON)) == 0)
            ReleaseCapture();
          return !m_app.playing();
        case WM_RBUTTONUP:
          m_app.scenePointerRelease(x, y, false, true, width, height);
          if ((msg->wParam & (MK_LBUTTON | MK_RBUTTON)) == 0)
            ReleaseCapture();
          return !m_app.playing();
        case WM_MOUSEWHEEL:
          if (!m_app.playing())
          {
            m_app.sceneWheel(static_cast<float>(GET_WHEEL_DELTA_WPARAM(msg->wParam)));
            return true;
          }
          return false;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          if ((msg->lParam & (1LL << 30)) == 0)
            m_app.sceneKeyChanged(static_cast<int>(msg->wParam), true);
          return false;
        case WM_KEYUP:
        case WM_SYSKEYUP:
          m_app.sceneKeyChanged(static_cast<int>(msg->wParam), false);
          return false;
        default:
          return false;
        }
#else
        (void)message;
        return false;
#endif
      }

    protected:
      bool eventFilter(QObject *, QEvent *event) override
      {
        const qreal dpr = m_window.devicePixelRatio();
        const int width = std::max(1, static_cast<int>(m_window.width() * dpr));
        const int height = std::max(1, static_cast<int>(m_window.height() * dpr));

#ifdef _WIN32
        // Mouse/key messages for the foreign GLFW HWND are handled by the
        // native filter above. Keep Qt events here for drag/drop only.
        if (event->type() != QEvent::DragEnter && event->type() != QEvent::Drop)
        {
          return false;
        }
#endif
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        {
          auto *mouse = static_cast<QMouseEvent *>(event);
          m_window.requestActivate();
          m_window.setMouseGrabEnabled(true);
          const QPointF p = mouse->position() * dpr;
          m_app.scenePointerPress(
              static_cast<float>(p.x()), static_cast<float>(p.y()),
              mouse->button() == Qt::LeftButton, mouse->button() == Qt::RightButton,
              mouse->modifiers().testFlag(Qt::ShiftModifier), width, height);
          return true;
        }
        case QEvent::MouseMove:
        {
          auto *mouse = static_cast<QMouseEvent *>(event);
          const QPointF p = mouse->position() * dpr;
          m_app.scenePointerMove(
              static_cast<float>(p.x()), static_cast<float>(p.y()),
              mouse->buttons().testFlag(Qt::LeftButton),
              mouse->buttons().testFlag(Qt::RightButton), width, height);
          return true;
        }
        case QEvent::MouseButtonRelease:
        {
          auto *mouse = static_cast<QMouseEvent *>(event);
          const QPointF p = mouse->position() * dpr;
          m_app.scenePointerRelease(
              static_cast<float>(p.x()), static_cast<float>(p.y()),
              mouse->button() == Qt::LeftButton, mouse->button() == Qt::RightButton,
              width, height);
          if (mouse->buttons() == Qt::NoButton)
          {
            m_window.setMouseGrabEnabled(false);
          }
          return true;
        }
        case QEvent::Wheel:
          m_app.sceneWheel(static_cast<QWheelEvent *>(event)->angleDelta().y());
          return true;
        case QEvent::KeyPress:
        {
          auto *key = static_cast<QKeyEvent *>(event);
          if (!key->isAutoRepeat())
          {
            m_app.sceneKeyChanged(key->key(), true);
          }
          return true;
        }
        case QEvent::KeyRelease:
        {
          auto *key = static_cast<QKeyEvent *>(event);
          if (!key->isAutoRepeat())
          {
            m_app.sceneKeyChanged(key->key(), false);
          }
          return true;
        }
        case QEvent::FocusOut:
          // Clear held movement keys if another dock takes focus.
          for (const int key : {'W', 'A', 'S', 'D', 'Q', 'Z'})
          {
            m_app.sceneKeyChanged(key, false);
          }
          return false;
        case QEvent::DragEnter:
        {
          auto *drag = static_cast<QDragEnterEvent *>(event);
          if (drag->mimeData()->hasFormat("application/x-nebula-asset"))
          {
            drag->acceptProposedAction();
            return true;
          }
          return false;
        }
        case QEvent::Drop:
        {
          auto *drop = static_cast<QDropEvent *>(event);
          if (!drop->mimeData()->hasFormat("application/x-nebula-asset"))
          {
            return false;
          }
          const QString payload =
              QString::fromUtf8(drop->mimeData()->data("application/x-nebula-asset"));
          const QStringList parts = payload.split('\n');
          if (parts.size() == 2)
          {
            const QPointF p = drop->position() * dpr;
            m_app.sceneDropAsset(
                parts[0].toStdString(),
                static_cast<AssetEntryKind>(parts[1].toInt()),
                static_cast<float>(p.x()), static_cast<float>(p.y()), width, height);
          }
          drop->acceptProposedAction();
          return true;
        }
        default:
          return false;
        }
      }

    private:
      EditorApplication &m_app;
      QWindow &m_window;
      WId m_nativeHandle = 0;
    };

    QString entityDisplayName(Nebula::Scene &scene, Nebula::Entity e)
    {
      if (scene.hasComponent<Nebula::TagComponent>(e))
      {
        return QString::fromStdString(scene.getComponent<Nebula::TagComponent>(e).tag);
      }
      return QString("Entity %1").arg(e.id);
    }

    std::string entityVectorToJson(const std::vector<Nebula::Entity> &entities)
    {
      std::string json = "[";
      for (std::size_t i = 0; i < entities.size(); ++i)
      {
        if (i != 0)
          json += ",";
        json += "{\"id\":" + std::to_string(entities[i].id) +
                ",\"generation\":" + std::to_string(entities[i].generation) + "}";
      }
      json += "]";
      return json;
    }

    QLabel *sectionLabel(const QString &text, QWidget *parent)
    {
      auto *label = new QLabel(QString("<b>%1</b>").arg(text), parent);
      label->setStyleSheet("color: #7fb2e5; padding-top: 8px;");
      return label;
    }
  }

  QtMainWindow::QtMainWindow(std::unique_ptr<EditorApplication> app, QWidget *parent)
      : QMainWindow(parent), m_app(std::move(app))
  {
    setWindowTitle("Nebula Editor");
    resize(1600, 900);
    applyDarkTheme();
    buildUi();
#ifdef _WIN32
    QTimer::singleShot(0, this, [this]()
                       {
                         BOOL dark = TRUE;
                         DwmSetWindowAttribute(
                             reinterpret_cast<HWND>(winId()), 20, &dark, sizeof(dark));
                       });
#endif

    m_app->startFrameLoop();
    embedSceneView();

    connect(&m_timer, &QTimer::timeout, this, &QtMainWindow::onFrame);
    m_timer.start(16);
    refreshHierarchy();
    refreshAssets();
    rebuildInspector();
    rebuildDebugTuning();
    m_app->editorLog().info(
        "Editor ready — click Scene View: LMB pick/drag gizmo, RMB look, scroll dolly, W/E/R tools");
  }

  QtMainWindow::~QtMainWindow()
  {
    m_timer.stop();
    if (m_nativeInputFilter != nullptr)
    {
      qApp->removeNativeEventFilter(m_nativeInputFilter);
      m_nativeInputFilter = nullptr;
    }
    if (m_app)
    {
      m_app->stopFrameLoop();
    }
  }

  void QtMainWindow::applyDarkTheme()
  {
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette p;
    const QColor window(37, 39, 43);
    const QColor base(28, 30, 33);
    const QColor alt(45, 48, 53);
    const QColor text(220, 222, 226);
    const QColor disabled(120, 122, 126);
    const QColor accent(53, 132, 228);
    p.setColor(QPalette::Window, window);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, base);
    p.setColor(QPalette::AlternateBase, alt);
    p.setColor(QPalette::ToolTipBase, alt);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Button, window);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Link, accent);
    qApp->setPalette(p);
    qApp->setStyleSheet(R"(
      QMainWindow::separator { background: #1a1c1f; width: 3px; height: 3px; }
      QDockWidget { titlebar-close-icon: none; font-weight: 600; }
      QDockWidget::title { background: #2d3035; padding: 5px 8px; }
      QToolBar { background: #2d3035; border: none; padding: 3px; spacing: 4px; }
      QToolButton { padding: 4px 10px; border-radius: 4px; }
      QToolButton:hover { background: #3a3e44; }
      QToolButton:pressed { background: #3584e4; }
      QListWidget, QPlainTextEdit, QScrollArea { border: 1px solid #1a1c1f; }
      QListWidget::item { padding: 3px 6px; }
      QListWidget::item:selected { background: #3584e4; }
      /* Do not put padding on QSpinBox/QDoubleSpinBox — it breaks the embedded
         line edit (arrows still work, but you cannot click/type a value). */
      QDoubleSpinBox, QSpinBox {
        border: 1px solid #1a1c1f; border-radius: 3px; background: #1c1e21; color: #dcdce2;
        padding-right: 2px;
      }
      QDoubleSpinBox::up-button, QSpinBox::up-button,
      QDoubleSpinBox::down-button, QSpinBox::down-button { width: 16px; border: none; }
      QLineEdit, QComboBox {
        padding: 2px 4px; border: 1px solid #1a1c1f; border-radius: 3px;
        background: #1c1e21; color: #dcdce2; selection-background-color: #3584e4;
      }
      QMenuBar { background: #2d3035; }
      QMenuBar::item:selected { background: #3a3e44; }
      QMenu { background: #2d3035; border: 1px solid #1a1c1f; }
      QMenu::item:selected { background: #3584e4; }
      QLabel { background: transparent; }
    )");
  }

  void QtMainWindow::embedSceneView()
  {
    const WId handle = sceneViewNativeHandle(m_app->getWindow());
    if (handle == 0)
    {
      auto *fallback = new QLabel("Scene View unavailable (native embed failed)", this);
      fallback->setAlignment(Qt::AlignCenter);
      setCentralWidget(fallback);
      return;
    }
    m_sceneWindow = QWindow::fromWinId(handle);
    m_sceneWindow->setFlag(Qt::FramelessWindowHint, true);
    auto *inputFilter =
        new SceneInputFilter(*m_app, *m_sceneWindow, handle, this);
    m_sceneInputFilter = inputFilter;
    m_nativeInputFilter = inputFilter;
    qApp->installNativeEventFilter(inputFilter);
    m_sceneWindow->installEventFilter(m_sceneInputFilter);
    m_sceneContainer = QWidget::createWindowContainer(m_sceneWindow, this);
    m_sceneContainer->setMinimumSize(480, 320);
    m_sceneContainer->setFocusPolicy(Qt::StrongFocus);
    m_sceneContainer->setMouseTracking(true);
    m_sceneContainer->setAcceptDrops(true);
    m_sceneContainer->installEventFilter(m_sceneInputFilter);
    setCentralWidget(m_sceneContainer);
  }

  void QtMainWindow::buildUi()
  {
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->addAction("▶ Play", this, &QtMainWindow::onPlay);
    toolbar->addAction("■ Stop", this, &QtMainWindow::onStop);
    toolbar->addSeparator();
    toolbar->addAction("Translate (W)", this, &QtMainWindow::setTranslateMode);
    toolbar->addAction("Rotate (E)", this, &QtMainWindow::setRotateMode);
    toolbar->addAction("Scale (R)", this, &QtMainWindow::setScaleMode);
    toolbar->addSeparator();
    m_gizmoLabel = new QLabel("  Gizmo: Translate  ", this);
    toolbar->addWidget(m_gizmoLabel);
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);
    m_fpsLabel = new QLabel("FPS: --  ", this);
    toolbar->addWidget(m_fpsLabel);

    buildMenus();

    m_hierarchy = new QListWidget(this);
    connect(m_hierarchy, &QListWidget::currentRowChanged, this,
            &QtMainWindow::onHierarchySelectionChanged);
    m_hierarchy->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_hierarchy, &QListWidget::customContextMenuRequested, this,
            &QtMainWindow::showHierarchyContextMenu);
    auto *hierDock = new QDockWidget("Hierarchy", this);
    hierDock->setWidget(m_hierarchy);
    addDockWidget(Qt::LeftDockWidgetArea, hierDock);

    auto *assetsHost = new QWidget(this);
    auto *assetsLayout = new QVBoxLayout(assetsHost);
    assetsLayout->setContentsMargins(4, 4, 4, 4);
    auto *assetControls = new QHBoxLayout();
    m_assetFilter = new QComboBox(assetsHost);
    m_assetFilter->addItems({"All", "Meshes", "Materials", "Prefabs"});
    auto *refreshAssetsButton = new QPushButton("Refresh", assetsHost);
    assetControls->addWidget(m_assetFilter, 1);
    assetControls->addWidget(refreshAssetsButton);
    assetsLayout->addLayout(assetControls);
    m_assets = new AssetListWidget(assetsHost);
    m_assets->setDragEnabled(true);
    assetsLayout->addWidget(m_assets, 1);
    connect(m_assetFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { refreshAssets(); });
    connect(refreshAssetsButton, &QPushButton::clicked, this, [this]()
            {
              m_app->refreshAssetsPublic();
              refreshAssets();
            });
    connect(m_assets, &QListWidget::itemSelectionChanged, this, [this]()
            {
              if (auto *item = m_assets->currentItem())
              {
                m_app->editorState().selectedAssetPath =
                    item->data(Qt::UserRole).toString().toStdString();
                m_app->editorState().selectedAssetKind =
                    static_cast<AssetEntryKind>(item->data(Qt::UserRole + 1).toInt());
              }
            });
    connect(m_assets, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *item)
            {
              const auto kind =
                  static_cast<AssetEntryKind>(item->data(Qt::UserRole + 1).toInt());
              const std::string path = item->data(Qt::UserRole).toString().toStdString();
              if (kind == AssetEntryKind::Prefab)
              {
                m_app->spawnPrefabPublic(path, {0.f, 0.5f, 0.f});
              }
              else if (kind == AssetEntryKind::Mesh)
              {
                m_app->spawnMeshPublic(path, "materials/solid_cube.mat", {0.f, 0.5f, 0.f});
              }
              refreshHierarchy();
              rebuildInspector();
            });
    auto *assetsDock = new QDockWidget("Assets", this);
    assetsDock->setWidget(assetsHost);
    addDockWidget(Qt::LeftDockWidgetArea, assetsDock);

    m_inspectorScroll = new QScrollArea(this);
    m_inspectorScroll->setWidgetResizable(true);
    auto *inspDock = new QDockWidget("Inspector", this);
    inspDock->setWidget(m_inspectorScroll);
    inspDock->setMinimumWidth(300);
    addDockWidget(Qt::RightDockWidgetArea, inspDock);

    m_nodes = new QtNodeGraphPanel(this);
    m_nodes->loadGraph("assets/graphs/objective_complete.json");
    auto *nodesDock = new QDockWidget("Node Graph", this);
    nodesDock->setWidget(m_nodes);
    addDockWidget(Qt::RightDockWidgetArea, nodesDock);
    tabifyDockWidget(inspDock, nodesDock);
    inspDock->raise();

    m_console = new QPlainTextEdit(this);
    m_console->setReadOnly(true);
    m_console->setMaximumBlockCount(2000);
    m_console->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_console->setMinimumHeight(0);
    auto *consoleHost = new QWidget(this);
    consoleHost->setMinimumHeight(0);
    consoleHost->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto *consoleLayout = new QVBoxLayout(consoleHost);
    consoleLayout->setContentsMargins(3, 3, 3, 3);
    auto *clearConsole = new QPushButton("Clear", consoleHost);
    clearConsole->setMaximumWidth(80);
    consoleLayout->addWidget(clearConsole, 0, Qt::AlignLeft);
    consoleLayout->addWidget(m_console, 1);
    connect(clearConsole, &QPushButton::clicked, this, [this]()
            {
              m_app->editorLog().clear();
              m_lastLogCount = -1;
              refreshConsole();
            });
    auto *consoleDock = new QDockWidget("Console", this);
    consoleDock->setWidget(consoleHost);
    consoleDock->setMinimumHeight(64);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock);

    auto *debugHost = new QWidget(this);
    debugHost->setMinimumHeight(0);
    auto *debugLayout = new QVBoxLayout(debugHost);
    debugLayout->setContentsMargins(4, 4, 4, 4);
    m_debugText = new QPlainTextEdit(debugHost);
    m_debugText->setReadOnly(true);
    m_debugText->setMinimumHeight(0);
    m_debugText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_debugText->setMaximumHeight(120);
    debugLayout->addWidget(m_debugText, 1);
    auto *tuningScroll = new QScrollArea(debugHost);
    tuningScroll->setWidgetResizable(true);
    tuningScroll->setFrameShape(QFrame::NoFrame);
    tuningScroll->setMinimumHeight(0);
    tuningScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_debugTuningHost = new QWidget(tuningScroll);
    m_debugTuningHost->setMinimumHeight(0);
    m_debugTuningForm = new QFormLayout(m_debugTuningHost);
    tuningScroll->setWidget(m_debugTuningHost);
    debugLayout->addWidget(tuningScroll, 2);
    auto *debugDock = new QDockWidget("Debug", this);
    debugDock->setWidget(debugHost);
    debugDock->setMinimumHeight(64);
    addDockWidget(Qt::BottomDockWidgetArea, debugDock);
    tabifyDockWidget(consoleDock, debugDock);
    consoleDock->raise();

    m_inspectorDock = inspDock;
    QTimer::singleShot(0, this,
                       [this, hierDock, assetsDock, inspDock, consoleDock]()
                       {
                         resizeDocks({hierDock, assetsDock}, {420, 260},
                                     Qt::Vertical);
                         resizeDocks({hierDock, inspDock}, {260, 360},
                                     Qt::Horizontal);
                         resizeDocks({consoleDock}, {140}, Qt::Vertical);
                       });

    refreshAssets();
    rebuildDebugTuning();
  }

  void QtMainWindow::buildMenus()
  {
    auto *fileMenu = menuBar()->addMenu("&File");
    if (m_app->scenePresets().empty())
    {
      fileMenu->addAction("New Scene", this, &QtMainWindow::onNewScene);
    }
    else
    {
      auto *newMenu = fileMenu->addMenu("New Scene");
      for (const ScenePreset &preset : m_app->scenePresets())
      {
        const QString label = QString::fromUtf8(preset.label);
        const std::string loadPath = preset.loadPath != nullptr ? preset.loadPath : "";
        const NewSceneBuilder builder = preset.build;
        newMenu->addAction(label, this, [this, loadPath, builder]()
                           {
                             if (!loadPath.empty())
                             {
                               m_app->newSceneFromPathPublic(loadPath);
                             }
                             else
                             {
                               m_app->newScenePublic(builder);
                             }
                             refreshHierarchy();
                             rebuildInspector();
                             rebuildDebugTuning();
                           });
      }
    }
    fileMenu->addSeparator();
    fileMenu->addAction("Open Scene...", this, &QtMainWindow::onOpen,
                        QKeySequence::Open);
    fileMenu->addAction("Save", this, &QtMainWindow::onSave,
                        QKeySequence::Save);
    fileMenu->addAction("Save Scene As...", this, &QtMainWindow::onSaveAs,
                        QKeySequence::SaveAs);

    auto *editMenu = menuBar()->addMenu("&Edit");
    const struct
    {
      const char *label;
      const char *id;
    } templates[] = {
        {"Create Empty", "empty"},
        {"Create Mesh Cube", "cube"},
        {"Create Enemy Placeholder", "enemy"},
        {"Create Platform", "platform"},
        {"Create Bounce Pad", "bouncePad"},
        {"Create Wind Volume", "windVolume"},
    };
    for (const auto &entry : templates)
    {
      editMenu->addAction(entry.label, this, [this, id = std::string(entry.id)]()
                          {
                            m_app->createEntityPublic(id.c_str());
                            refreshHierarchy();
                            rebuildInspector();
                          });
    }
    editMenu->addSeparator();
    editMenu->addAction("Delete Selected", this, [this]()
                        {
                          m_app->deleteSelectedPublic();
                          refreshHierarchy();
                          rebuildInspector();
                        });
    // Bind Delete only when not editing a text field (shortcut would steal Backspace/Delete).
    {
      auto *delShortcut = new QShortcut(QKeySequence::Delete, this);
      delShortcut->setContext(Qt::WindowShortcut);
      connect(delShortcut, &QShortcut::activated, this, [this]()
              {
                QWidget *focus = QApplication::focusWidget();
                if (qobject_cast<QLineEdit *>(focus) != nullptr ||
                    qobject_cast<QAbstractSpinBox *>(focus) != nullptr ||
                    qobject_cast<QPlainTextEdit *>(focus) != nullptr ||
                    (focus != nullptr && focus->inherits("QSpinBox")) ||
                    (focus != nullptr && focus->parentWidget() != nullptr &&
                     qobject_cast<QAbstractSpinBox *>(focus->parentWidget()) != nullptr))
                {
                  return;
                }
                m_app->deleteSelectedPublic();
                refreshHierarchy();
                rebuildInspector();
              });
    }
    editMenu->addAction("Save Selected as Prefab", this, [this]()
                        {
                          m_app->saveSelectedAsPrefabPublic();
                          refreshAssets();
                        });
    editMenu->addAction("Revert Selected to Prefab", this, [this]()
                        {
                          m_app->revertSelectedPrefabPublic();
                          rebuildInspector();
                        });
    editMenu->addAction("Create Variant from Selected", this, [this]()
                        {
                          m_app->createVariantPublic();
                          refreshAssets();
                        });

    auto *prefabMenu = editMenu->addMenu("Instantiate Prefab");
    constexpr const char *kKnownPrefabs[] = {
        "prefabs/enemy.prefab", "prefabs/enemy_fast.prefab",
        "prefabs/platform.prefab", "prefabs/bounce_pad.prefab",
        "prefabs/wind_volume.prefab"};
    for (const char *path : kKnownPrefabs)
    {
      prefabMenu->addAction(path, this, [this, path]()
                            {
                              m_app->instantiatePrefab(path);
                              refreshHierarchy();
                              rebuildInspector();
                            });
    }

    auto *sceneMenu = menuBar()->addMenu("&Scene View");
    sceneMenu->addAction("Select Tool", this, [this]()
                         {
                           m_app->editorState().toolMode = EditorToolMode::Select;
                           m_app->editorState().colliderEditMode = false;
                         });
    sceneMenu->addAction("Translate Tool", this, &QtMainWindow::setTranslateMode);
    sceneMenu->addSeparator();
    auto *showColliders = sceneMenu->addAction("Show Colliders");
    showColliders->setCheckable(true);
    showColliders->setChecked(m_app->editorState().showColliderGizmos);
    connect(showColliders, &QAction::toggled, this, [this](bool checked)
            { m_app->editorState().showColliderGizmos = checked; });
    auto *showGizmo = sceneMenu->addAction("Show Transform Gizmo");
    showGizmo->setCheckable(true);
    showGizmo->setChecked(m_app->editorState().showTransformGizmo);
    connect(showGizmo, &QAction::toggled, this, [this](bool checked)
            { m_app->editorState().showTransformGizmo = checked; });
    auto *colliderEdit = sceneMenu->addAction("Collider Edit Mode");
    colliderEdit->setCheckable(true);
    colliderEdit->setChecked(m_app->editorState().colliderEditMode);
    connect(colliderEdit, &QAction::toggled, this, [this](bool checked)
            {
              m_app->editorState().colliderEditMode = checked;
              m_app->editorState().toolMode =
                  checked ? EditorToolMode::ColliderEdit : EditorToolMode::Select;
            });
  }

  void QtMainWindow::showHierarchyContextMenu(const QPoint &position)
  {
    QMenu menu(this);
    auto *create = menu.addMenu("Create");
    const struct
    {
      const char *label;
      const char *id;
    } templates[] = {
        {"Empty", "empty"}, {"Mesh Cube", "cube"}, {"Enemy", "enemy"},
        {"Platform", "platform"}, {"Bounce Pad", "bouncePad"},
        {"Wind Volume", "windVolume"}};
    for (const auto &entry : templates)
    {
      create->addAction(entry.label, this, [this, id = std::string(entry.id)]()
                        {
                          m_app->createEntityPublic(id.c_str());
                          refreshHierarchy();
                          rebuildInspector();
                        });
    }
    auto *instantiate = menu.addMenu("Instantiate Prefab");
    constexpr const char *kKnownPrefabs[] = {
        "prefabs/enemy.prefab", "prefabs/enemy_fast.prefab",
        "prefabs/platform.prefab", "prefabs/bounce_pad.prefab",
        "prefabs/wind_volume.prefab"};
    for (const char *path : kKnownPrefabs)
    {
      instantiate->addAction(path, this, [this, path]()
                             {
                               m_app->instantiatePrefab(path);
                               refreshHierarchy();
                               rebuildInspector();
                             });
    }
    menu.addSeparator();
    QAction *savePrefab = menu.addAction("Save Selected as Prefab");
    QAction *deleteEntity = menu.addAction("Delete Entity");
    const bool valid = m_app->scene().isValidEntity(selectedEntity());
    savePrefab->setEnabled(valid);
    deleteEntity->setEnabled(valid);
    connect(savePrefab, &QAction::triggered, this, [this]()
            {
              m_app->saveSelectedAsPrefabPublic();
              refreshAssets();
            });
    connect(deleteEntity, &QAction::triggered, this, [this]()
            {
              m_app->deleteSelectedPublic();
              refreshHierarchy();
              rebuildInspector();
            });
    menu.exec(m_hierarchy->viewport()->mapToGlobal(position));
  }

  void QtMainWindow::refreshAssets()
  {
    if (m_assets == nullptr)
    {
      return;
    }
    m_assets->clear();
    const int filter = m_assetFilter != nullptr ? m_assetFilter->currentIndex() : 0;
    for (const AssetEntry &entry : m_app->assetCatalog().entries())
    {
      if ((filter == 1 && entry.kind != AssetEntryKind::Mesh) ||
          (filter == 2 && entry.kind != AssetEntryKind::Material) ||
          (filter == 3 && entry.kind != AssetEntryKind::Prefab))
      {
        continue;
      }
      auto *item = new QListWidgetItem(QString::fromStdString(entry.label), m_assets);
      item->setData(Qt::UserRole, QString::fromStdString(entry.logicalPath));
      item->setData(Qt::UserRole + 1, static_cast<int>(entry.kind));
      if (entry.logicalPath == m_app->editorState().selectedAssetPath)
      {
        m_assets->setCurrentItem(item);
      }
    }
  }

  Nebula::Entity QtMainWindow::selectedEntity() const
  {
    return m_app->editorState().selectedEntity;
  }

  void QtMainWindow::onFrame()
  {
    if (!m_app->pumpFrame())
    {
      close();
      return;
    }
    const float dt = m_app->lastFrameDt();
    if (dt > 0.0001f)
    {
      m_fpsEma = m_fpsEma * 0.9f + (1.f / dt) * 0.1f;
      m_fpsLabel->setText(QString("FPS: %1  ").arg(static_cast<int>(m_fpsEma + 0.5f)));
    }
    refreshConsole();
    refreshDebug();
    syncSelectionFromEngine();
    updateGizmoLabel();

    // Track structural changes (spawn/delete/play/stop rebuilds).
    int count = 0;
    for (const Nebula::Entity e : m_app->scene().getAllEntities())
    {
      (void)e;
      ++count;
    }
    if (count != m_lastEntityCount)
    {
      m_lastEntityCount = count;
      refreshHierarchy();
      rebuildDebugTuning();
    }

    // Keep transform fields live while the gizmo drags entities around.
    if (m_app->qtGizmo().isDragging())
    {
      refreshTransformValues();
    }
  }

  void QtMainWindow::updateGizmoLabel()
  {
    const int mode = m_app->gizmoMode();
    if (mode == m_lastGizmoMode)
    {
      return;
    }
    m_lastGizmoMode = mode;
    const char *names[] = {"Translate (W)", "Rotate (E)", "Scale (R)"};
    m_gizmoLabel->setText(QString("  Gizmo: %1  ").arg(names[std::clamp(mode, 0, 2)]));
  }

  void QtMainWindow::syncSelectionFromEngine()
  {
    const Nebula::Entity current = selectedEntity();
    if (current == m_lastSelected)
    {
      return;
    }
    m_lastSelected = current;

    m_blockUiSignals = true;
    m_hierarchy->clearSelection();
    for (int i = 0; i < m_hierarchy->count(); ++i)
    {
      auto *item = m_hierarchy->item(i);
      if (item->data(Qt::UserRole).toULongLong() == current.id)
      {
        m_hierarchy->setCurrentItem(item);
        break;
      }
    }
    m_blockUiSignals = false;
    rebuildInspector();
  }

  void QtMainWindow::onPlay()
  {
    m_app->enterPlayPublic();
    refreshHierarchy();
    rebuildDebugTuning();
  }
  void QtMainWindow::onStop()
  {
    m_app->exitPlayPublic();
    refreshHierarchy();
    rebuildInspector();
    rebuildDebugTuning();
  }
  void QtMainWindow::onOpen()
  {
    const QString path =
        QFileDialog::getOpenFileName(this, "Open Scene", "assets/scenes",
                                     "Nebula Scenes (*.json)");
    if (path.isEmpty())
    {
      return;
    }
    m_app->loadSceneAbsolutePublic(path.toStdString());
    refreshHierarchy();
    rebuildInspector();
    rebuildDebugTuning();
  }
  void QtMainWindow::onSave() { m_app->saveScenePublic(); }
  void QtMainWindow::onSaveAs()
  {
    const QString path =
        QFileDialog::getSaveFileName(this, "Save Scene As", "scene.json", "Scenes (*.json)");
    if (path.isEmpty())
    {
      return;
    }
    m_app->saveSceneAsAbsolutePublic(path.toStdString());
  }
  void QtMainWindow::onNewScene()
  {
    m_app->newScenePublic();
    refreshHierarchy();
    rebuildInspector();
    rebuildDebugTuning();
  }

  void QtMainWindow::setTranslateMode() { m_app->setGizmoMode(0); }
  void QtMainWindow::setRotateMode() { m_app->setGizmoMode(1); }
  void QtMainWindow::setScaleMode() { m_app->setGizmoMode(2); }

  void QtMainWindow::refreshHierarchy()
  {
    m_blockUiSignals = true;
    m_hierarchy->clear();
    Nebula::Scene &scene = m_app->scene();
    for (const Nebula::Entity e : scene.getAllEntities())
    {
      auto *item = new QListWidgetItem(entityDisplayName(scene, e), m_hierarchy);
      item->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(e.id));
      item->setData(Qt::UserRole + 1, QVariant::fromValue<qulonglong>(e.generation));
      if (e == selectedEntity())
      {
        m_hierarchy->setCurrentItem(item);
      }
    }
    m_blockUiSignals = false;
  }

  void QtMainWindow::onHierarchySelectionChanged()
  {
    if (m_blockUiSignals)
    {
      return;
    }
    auto *item = m_hierarchy->currentItem();
    if (item == nullptr)
    {
      m_app->editorState().selectedEntity = {};
      m_lastSelected = {};
      rebuildInspector();
      return;
    }
    Nebula::Entity e{};
    e.id = static_cast<Nebula::EntityID>(item->data(Qt::UserRole).toULongLong());
    e.generation = static_cast<uint32_t>(item->data(Qt::UserRole + 1).toULongLong());
    m_app->editorState().selectedEntity = e;
    m_lastSelected = e;
    rebuildInspector();
  }

  void QtMainWindow::refreshTransformValues()
  {
    const Nebula::Entity e = selectedEntity();
    Nebula::Scene &scene = m_app->scene();
    if (m_pos[0] == nullptr || !scene.isValidEntity(e) ||
        !scene.hasComponent<Nebula::TransformComponent>(e))
    {
      return;
    }
    const auto &tf = scene.getComponent<Nebula::TransformComponent>(e).transform;
    m_blockUiSignals = true;
    m_pos[0]->setValue(tf.getPosition().x);
    m_pos[1]->setValue(tf.getPosition().y);
    m_pos[2]->setValue(tf.getPosition().z);
    m_yaw->setValue(tf.getYaw());
    m_scale[0]->setValue(tf.getScale().x);
    m_scale[1]->setValue(tf.getScale().y);
    m_scale[2]->setValue(tf.getScale().z);
    m_blockUiSignals = false;
  }

  void QtMainWindow::rebuildInspector()
  {
    for (auto *&box : m_pos)
    {
      box = nullptr;
    }
    for (auto *&box : m_scale)
    {
      box = nullptr;
    }
    m_yaw = nullptr;

    auto *host = new QWidget(m_inspectorScroll);
    auto *form = new QFormLayout(host);
    form->setLabelAlignment(Qt::AlignRight);
    form->setVerticalSpacing(4);

    Nebula::Scene &scene = m_app->scene();
    const Nebula::Entity entity = selectedEntity();
    if (!scene.isValidEntity(entity))
    {
      form->addRow(new QLabel("No entity selected.\nClick an object in the Scene View\nor pick one in the Hierarchy.", host));
      m_inspectorScroll->setWidget(host);
      return;
    }

    const auto markDirty = [this]()
    { m_app->editorState().sceneDirty = true; };
    const auto addComponentSection =
        [this, host, form](const QString &title, const std::function<void()> &remove)
    {
      auto *row = new QWidget(host);
      auto *layout = new QHBoxLayout(row);
      layout->setContentsMargins(0, 6, 0, 0);
      layout->addWidget(sectionLabel(title, row), 1);
      if (remove)
      {
        auto *button = new QPushButton("Remove", row);
        button->setMaximumWidth(70);
        connect(button, &QPushButton::clicked, this, [this, remove]()
                {
                  remove();
                  QTimer::singleShot(0, this, [this]() { rebuildInspector(); });
                });
        layout->addWidget(button);
      }
      form->addRow(row);
    };
    const bool isPrefabInstance =
        scene.hasComponent<Nebula::PrefabInstanceComponent>(entity);
    const auto addResetButton =
        [this, host, form, isPrefabInstance, &scene, entity](
            const QString &label, QWidget *fieldWidget,
            std::string component, std::string field)
    {
      const bool overridden =
          isPrefabInstance &&
          Nebula::PrefabService::overridesContainPath(
              scene.getComponent<Nebula::PrefabInstanceComponent>(entity).overridesJson,
              component.c_str(), field.c_str());
      if (!overridden)
      {
        form->addRow(label, fieldWidget);
        return;
      }
      auto *row = new QWidget(host);
      auto *layout = new QHBoxLayout(row);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(fieldWidget, 1);
      auto *reset = new QPushButton("Reset", row);
      reset->setMaximumWidth(55);
      connect(reset, &QPushButton::clicked, this,
              [this, component = std::move(component), field = std::move(field)]()
              {
                if (m_app->resetSelectedOverridePublic(component.c_str(), field.c_str()))
                {
                  rebuildInspector();
                }
              });
      layout->addWidget(reset);
      form->addRow(label, row);
    };

    // --- Header / Tag ---
    form->addRow(sectionLabel(entityDisplayName(scene, entity), host));
    if (scene.hasComponent<Nebula::TagComponent>(entity))
    {
      addComponentSection("Tag", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::TagComponent>(entity))
                            {
                              s.removeComponent<Nebula::TagComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                              refreshHierarchy();
                            }
                          });
      auto *tagEdit = new QLineEdit(
          QString::fromStdString(scene.getComponent<Nebula::TagComponent>(entity).tag), host);
      connect(tagEdit, &QLineEdit::editingFinished, this, [this, tagEdit, entity]()
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::TagComponent>(entity))
                {
                  s.getComponent<Nebula::TagComponent>(entity).tag = tagEdit->text().toStdString();
                  m_app->editorState().sceneDirty = true;
                  refreshHierarchy();
                } });
      form->addRow("Tag", tagEdit);
    }
    else
    {
      auto *addTag = new QPushButton("Add Tag", host);
      connect(addTag, &QPushButton::clicked, this, [this, entity]()
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) &&
                    !s.hasComponent<Nebula::TagComponent>(entity))
                {
                  s.addComponent<Nebula::TagComponent>(entity).tag = "Entity";
                  m_app->editorState().sceneDirty = true;
                  refreshHierarchy();
                  rebuildInspector();
                }
              });
      form->addRow(addTag);
    }

    // --- Prefab instance ---
    if (scene.hasComponent<Nebula::PrefabInstanceComponent>(entity))
    {
      const auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
      form->addRow(sectionLabel("Prefab Instance", host));
      form->addRow("Source", new QLabel(QString::fromStdString(inst.prefabPath), host));
      auto *actions = new QWidget(host);
      auto *layout = new QHBoxLayout(actions);
      layout->setContentsMargins(0, 0, 0, 0);
      auto *revert = new QPushButton("Revert", actions);
      auto *variant = new QPushButton("Create Variant", actions);
      layout->addWidget(revert);
      layout->addWidget(variant);
      connect(revert, &QPushButton::clicked, this, [this]()
              {
                m_app->revertSelectedPrefabPublic();
                rebuildInspector();
              });
      connect(variant, &QPushButton::clicked, this, [this]()
              {
                m_app->createVariantPublic();
                refreshAssets();
              });
      form->addRow(actions);
    }

    // --- Transform ---
    if (scene.hasComponent<Nebula::TransformComponent>(entity))
    {
      addComponentSection("Transform", {});
      const auto makeSpin = [&](double min, double max) -> QDoubleSpinBox *
      {
        auto *box = new QDoubleSpinBox(host);
        box->setRange(min, max);
        box->setDecimals(3);
        box->setSingleStep(0.1);
        box->setKeyboardTracking(false);
        return box;
      };
      const char *posNames[] = {"Pos X", "Pos Y", "Pos Z"};
      const char *scaleNames[] = {"Scale X", "Scale Y", "Scale Z"};
      for (int i = 0; i < 3; ++i)
      {
        m_pos[i] = makeSpin(-10000.0, 10000.0);
        const char *fieldNames[] = {"position", "position", "position"};
        addResetButton(posNames[i], m_pos[i], "TransformComponent", fieldNames[i]);
      }
      m_yaw = makeSpin(-6.28318, 6.28318);
      m_yaw->setSingleStep(0.05);
      addResetButton("Yaw", m_yaw, "TransformComponent", "yaw");
      for (int i = 0; i < 3; ++i)
      {
        m_scale[i] = makeSpin(0.01, 1000.0);
        addResetButton(scaleNames[i], m_scale[i], "TransformComponent", "scale");
      }
      refreshTransformValues();

      const auto applyTransform = [this, entity]()
      {
        if (m_blockUiSignals)
        {
          return;
        }
        Nebula::Scene &s = m_app->scene();
        if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::TransformComponent>(entity))
        {
          return;
        }
        auto &tf = s.getComponent<Nebula::TransformComponent>(entity).transform;
        tf.setPosition(Nebula::Vec3{static_cast<float>(m_pos[0]->value()),
                                    static_cast<float>(m_pos[1]->value()),
                                    static_cast<float>(m_pos[2]->value())});
        tf.setYaw(static_cast<float>(m_yaw->value()));
        tf.setScale(Nebula::Vec3{static_cast<float>(m_scale[0]->value()),
                                 static_cast<float>(m_scale[1]->value()),
                                 static_cast<float>(m_scale[2]->value())});
        m_app->editorState().sceneDirty = true;
      };
      for (int i = 0; i < 3; ++i)
      {
        connect(m_pos[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, applyTransform);
        connect(m_scale[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, applyTransform);
      }
      connect(m_yaw, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, applyTransform);
    }

    // --- Mesh Renderer ---
    if (scene.hasComponent<Nebula::MeshRendererComponent>(entity))
    {
      auto &mesh = scene.getComponent<Nebula::MeshRendererComponent>(entity);
      addComponentSection("Mesh Renderer", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::MeshRendererComponent>(entity))
                            {
                              s.removeComponent<Nebula::MeshRendererComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                            }
                          });
      auto *preset = new QComboBox(host);
      preset->addItems({"Cube", "Ground", "Enemy", "Player"});
      int presetIndex = 0;
      if (mesh.m_materialPath == "builtin/materials/ground")
        presetIndex = 1;
      else if (mesh.m_materialPath == "builtin/materials/enemy")
        presetIndex = 2;
      else if (mesh.m_materialPath == "builtin/materials/player")
        presetIndex = 3;
      preset->setCurrentIndex(presetIndex);
      auto *meshEdit = new QLineEdit(QString::fromStdString(mesh.m_meshPath), host);
      auto *matEdit = new QLineEdit(QString::fromStdString(mesh.m_materialPath), host);
      const auto applyMesh = [this, entity, meshEdit, matEdit]()
      {
        Nebula::Scene &s = m_app->scene();
        if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::MeshRendererComponent>(entity))
        {
          return;
        }
        auto &m = s.getComponent<Nebula::MeshRendererComponent>(entity);
        m.m_meshPath = meshEdit->text().toStdString();
        m.m_materialPath = matEdit->text().toStdString();
        m_app->editorState().sceneDirty = true;
        m_app->resolveSceneAssetsNow();
      };
      connect(meshEdit, &QLineEdit::editingFinished, this, applyMesh);
      connect(matEdit, &QLineEdit::editingFinished, this, applyMesh);
      connect(preset, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              [this, entity, meshEdit, matEdit](int index)
              {
                Nebula::Scene &s = m_app->scene();
                if (!s.isValidEntity(entity) ||
                    !s.hasComponent<Nebula::MeshRendererComponent>(entity))
                {
                  return;
                }
                auto &m = s.getComponent<Nebula::MeshRendererComponent>(entity);
                m.m_meshPath =
                    index == 1 ? "builtin/meshes/ground" : "builtin/meshes/cube";
                const char *materials[] = {
                    "builtin/materials/cube", "builtin/materials/ground",
                    "builtin/materials/enemy", "builtin/materials/player"};
                m.m_materialPath = materials[std::clamp(index, 0, 3)];
                meshEdit->setText(QString::fromStdString(m.m_meshPath));
                matEdit->setText(QString::fromStdString(m.m_materialPath));
                m_app->editorState().sceneDirty = true;
                m_app->resolveSceneAssetsNow();
              });
      form->addRow("Preset", preset);
      form->addRow("Mesh", meshEdit);
      form->addRow("Material", matEdit);
    }

    // --- Script + fields ---
    if (scene.hasComponent<Nebula::ScriptComponent>(entity))
    {
      auto &script = scene.getComponent<Nebula::ScriptComponent>(entity);
      addComponentSection("Script", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::ScriptComponent>(entity))
                            {
                              s.removeComponent<Nebula::ScriptComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                            }
                          });

      auto *combo = new QComboBox(host);
      const std::vector<std::string> ids = m_app->scriptRegistryRef().registeredScriptIds();
      int currentIndex = -1;
      for (const std::string &id : ids)
      {
        combo->addItem(QString::fromStdString(id));
        if (id == script.scriptName)
        {
          currentIndex = combo->count() - 1;
        }
      }
      if (currentIndex >= 0)
      {
        combo->setCurrentIndex(currentIndex);
      }
      connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              [this, entity, combo](int)
              {
                Nebula::Scene &s = m_app->scene();
                if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::ScriptComponent>(entity))
                {
                  return;
                }
                auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                sc.scriptName = combo->currentText().toStdString();
                sc.paramsJson = m_scriptParams.mergeScriptParamDefaults(
                    sc.paramsJson, m_app->fieldRegistryRef(), sc.scriptName);
                m_app->editorState().sceneDirty = true;
                // Queued: rebuilding now would delete the combo emitting this signal.
                QTimer::singleShot(0, this, [this]() { rebuildInspector(); });
              });
      form->addRow("Type", combo);

      if (const auto *fields = m_app->fieldRegistryRef().getFields(script.scriptName))
      {
        script.paramsJson = m_scriptParams.mergeScriptParamDefaults(
            script.paramsJson, m_app->fieldRegistryRef(), script.scriptName);
        const bool isPrefabInstance = scene.hasComponent<Nebula::PrefabInstanceComponent>(entity);

        const auto writeOverride = [this, entity, isPrefabInstance](const std::string &fieldName,
                                                                    auto value)
        {
          if (!isPrefabInstance)
          {
            return;
          }
          Nebula::Scene &s = m_app->scene();
          auto &inst = s.getComponent<Nebula::PrefabInstanceComponent>(entity);
          Nebula::PrefabService::setInstanceOverrideField(
              inst, "ScriptComponent", ("paramsJson." + fieldName).c_str(), value);
        };

        for (const auto &field : *fields)
        {
          if (!field.visibleInEditor)
          {
            continue;
          }
          const std::string fieldName = field.name;
          switch (field.type)
          {
          case Nebula::ScriptFieldType::Float:
          {
            auto *box = new QDoubleSpinBox(host);
            box->setRange(-100000.0, 100000.0);
            box->setDecimals(3);
            box->setSingleStep(0.1);
            box->setKeyboardTracking(false);
            box->setValue(m_scriptParams.readScriptParamFloat(script.paramsJson, field));
            connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                    [this, entity, fieldName, writeOverride](double v)
                    {
                      Nebula::Scene &s = m_app->scene();
                      if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::ScriptComponent>(entity))
                      {
                        return;
                      }
                      auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                      sc.paramsJson = m_scriptParams.setScriptParamFloat(sc.paramsJson, fieldName,
                                                                         static_cast<float>(v));
                      writeOverride(fieldName, static_cast<float>(v));
                      m_app->editorState().sceneDirty = true;
                    });
            addResetButton(QString::fromStdString(fieldName), box,
                           "ScriptComponent",
                           ("paramsJson." + fieldName).c_str());
            break;
          }
          case Nebula::ScriptFieldType::Int:
          {
            auto *box = new QSpinBox(host);
            box->setRange(-1000000, 1000000);
            box->setKeyboardTracking(false);
            box->setValue(m_scriptParams.readScriptParamInt(script.paramsJson, field));
            connect(box, QOverload<int>::of(&QSpinBox::valueChanged), this,
                    [this, entity, fieldName, writeOverride](int v)
                    {
                      Nebula::Scene &s = m_app->scene();
                      if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::ScriptComponent>(entity))
                      {
                        return;
                      }
                      auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                      sc.paramsJson = m_scriptParams.setScriptParamInt(sc.paramsJson, fieldName, v);
                      writeOverride(fieldName, v);
                      m_app->editorState().sceneDirty = true;
                    });
            addResetButton(QString::fromStdString(fieldName), box,
                           "ScriptComponent",
                           ("paramsJson." + fieldName).c_str());
            break;
          }
          case Nebula::ScriptFieldType::Bool:
          {
            auto *box = new QCheckBox(host);
            box->setChecked(m_scriptParams.readScriptParamBool(script.paramsJson, field));
            connect(box, &QCheckBox::toggled, this,
                    [this, entity, fieldName, writeOverride](bool v)
                    {
                      Nebula::Scene &s = m_app->scene();
                      if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::ScriptComponent>(entity))
                      {
                        return;
                      }
                      auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                      sc.paramsJson = m_scriptParams.setScriptParamBool(sc.paramsJson, fieldName, v);
                      writeOverride(fieldName, v);
                      m_app->editorState().sceneDirty = true;
                    });
            addResetButton(QString::fromStdString(fieldName), box,
                           "ScriptComponent",
                           ("paramsJson." + fieldName).c_str());
            break;
          }
          case Nebula::ScriptFieldType::EntityVector:
          {
            auto entities =
                m_scriptParams.readScriptParamEntityVector(script.paramsJson, field);
            auto *listHost = new QWidget(host);
            auto *listLayout = new QVBoxLayout(listHost);
            listLayout->setContentsMargins(0, 0, 0, 0);
            for (std::size_t refIndex = 0; refIndex < entities.size(); ++refIndex)
            {
              auto *row = new QWidget(listHost);
              auto *rowLayout = new QHBoxLayout(row);
              rowLayout->setContentsMargins(0, 0, 0, 0);
              auto *entityCombo = new QComboBox(row);
              int selectedIndex = -1;
              for (const Nebula::Entity candidate : scene.getAllEntities())
              {
                if (candidate == entity)
                  continue;
                entityCombo->addItem(entityDisplayName(scene, candidate),
                                     QVariant::fromValue<qulonglong>(candidate.id));
                entityCombo->setItemData(
                    entityCombo->count() - 1,
                    QVariant::fromValue<qulonglong>(candidate.generation),
                    Qt::UserRole + 1);
                if (candidate == entities[refIndex])
                {
                  selectedIndex = entityCombo->count() - 1;
                }
              }
              entityCombo->setCurrentIndex(selectedIndex);
              auto *remove = new QPushButton("Remove", row);
              rowLayout->addWidget(entityCombo, 1);
              rowLayout->addWidget(remove);
              listLayout->addWidget(row);

              connect(entityCombo,
                      QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                      [this, entity, fieldName, refIndex, entityCombo](int index)
                      {
                        if (index < 0)
                          return;
                        Nebula::Scene &s = m_app->scene();
                        if (!s.isValidEntity(entity) ||
                            !s.hasComponent<Nebula::ScriptComponent>(entity))
                          return;
                        auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                        const auto *fields =
                            m_app->fieldRegistryRef().getFields(sc.scriptName);
                        if (fields == nullptr)
                          return;
                        auto it = std::find_if(
                            fields->begin(), fields->end(),
                            [&fieldName](const auto &f)
                            { return f.name == fieldName; });
                        if (it == fields->end())
                          return;
                        auto values =
                            m_scriptParams.readScriptParamEntityVector(sc.paramsJson, *it);
                        if (refIndex >= values.size())
                          return;
                        values[refIndex] = {
                            static_cast<Nebula::EntityID>(
                                entityCombo->itemData(index).toULongLong()),
                            static_cast<uint32_t>(
                                entityCombo->itemData(index, Qt::UserRole + 1)
                                    .toULongLong())};
                        sc.paramsJson = m_scriptParams.setScriptParamEntityVector(
                            sc.paramsJson, fieldName, values);
                        if (s.hasComponent<Nebula::PrefabInstanceComponent>(entity))
                        {
                          auto &inst =
                              s.getComponent<Nebula::PrefabInstanceComponent>(entity);
                          Nebula::PrefabService::setInstanceOverrideFieldJson(
                              inst, "ScriptComponent",
                              ("paramsJson." + fieldName).c_str(),
                              entityVectorToJson(values));
                        }
                        m_app->editorState().sceneDirty = true;
                      });
              connect(remove, &QPushButton::clicked, this,
                      [this, entity, fieldName, refIndex]()
                      {
                        Nebula::Scene &s = m_app->scene();
                        if (!s.isValidEntity(entity) ||
                            !s.hasComponent<Nebula::ScriptComponent>(entity))
                          return;
                        auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                        const auto *fields =
                            m_app->fieldRegistryRef().getFields(sc.scriptName);
                        if (fields == nullptr)
                          return;
                        auto it = std::find_if(
                            fields->begin(), fields->end(),
                            [&fieldName](const auto &f)
                            { return f.name == fieldName; });
                        if (it == fields->end())
                          return;
                        auto values =
                            m_scriptParams.readScriptParamEntityVector(sc.paramsJson, *it);
                        if (refIndex < values.size())
                          values.erase(values.begin() +
                                       static_cast<std::ptrdiff_t>(refIndex));
                        sc.paramsJson = m_scriptParams.setScriptParamEntityVector(
                            sc.paramsJson, fieldName, values);
                        if (s.hasComponent<Nebula::PrefabInstanceComponent>(entity))
                        {
                          auto &inst =
                              s.getComponent<Nebula::PrefabInstanceComponent>(entity);
                          Nebula::PrefabService::setInstanceOverrideFieldJson(
                              inst, "ScriptComponent",
                              ("paramsJson." + fieldName).c_str(),
                              entityVectorToJson(values));
                        }
                        m_app->editorState().sceneDirty = true;
                        rebuildInspector();
                      });
            }
            auto *add = new QPushButton("Add Entity", listHost);
            connect(add, &QPushButton::clicked, this,
                    [this, entity, fieldName]()
                    {
                      Nebula::Scene &s = m_app->scene();
                      if (!s.isValidEntity(entity) ||
                          !s.hasComponent<Nebula::ScriptComponent>(entity))
                        return;
                      auto &sc = s.getComponent<Nebula::ScriptComponent>(entity);
                      const auto *fields =
                          m_app->fieldRegistryRef().getFields(sc.scriptName);
                      if (fields == nullptr)
                        return;
                      auto it = std::find_if(
                          fields->begin(), fields->end(),
                          [&fieldName](const auto &f)
                          { return f.name == fieldName; });
                      if (it == fields->end())
                        return;
                      auto values =
                          m_scriptParams.readScriptParamEntityVector(sc.paramsJson, *it);
                      for (const Nebula::Entity candidate : s.getAllEntities())
                      {
                        if (candidate != entity)
                        {
                          values.push_back(candidate);
                          break;
                        }
                      }
                      sc.paramsJson = m_scriptParams.setScriptParamEntityVector(
                          sc.paramsJson, fieldName, values);
                      if (s.hasComponent<Nebula::PrefabInstanceComponent>(entity))
                      {
                        auto &inst =
                            s.getComponent<Nebula::PrefabInstanceComponent>(entity);
                        Nebula::PrefabService::setInstanceOverrideFieldJson(
                            inst, "ScriptComponent",
                            ("paramsJson." + fieldName).c_str(),
                            entityVectorToJson(values));
                      }
                      m_app->editorState().sceneDirty = true;
                      rebuildInspector();
                    });
            listLayout->addWidget(add);
            form->addRow(QString::fromStdString(fieldName), listHost);
            break;
          }
          }
        }
      }
    }

    // --- Camera ---
    if (scene.hasComponent<Nebula::CameraComponent>(entity))
    {
      auto &cam = scene.getComponent<Nebula::CameraComponent>(entity);
      addComponentSection("Camera", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::CameraComponent>(entity))
                            {
                              s.removeComponent<Nebula::CameraComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                            }
                          });
      const auto addCameraFloat =
          [this, host, form, entity, markDirty](const char *label, float value,
                                                double min, double max,
                                                const std::function<void(Nebula::CameraComponent &, float)> &set)
      {
        auto *box = new QDoubleSpinBox(host);
        box->setRange(min, max);
        box->setDecimals(3);
        box->setSingleStep(0.05);
        box->setValue(value);
        box->setKeyboardTracking(false);
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, entity, markDirty, set](double v)
                {
                  Nebula::Scene &s = m_app->scene();
                  if (s.isValidEntity(entity) &&
                      s.hasComponent<Nebula::CameraComponent>(entity))
                  {
                    set(s.getComponent<Nebula::CameraComponent>(entity),
                        static_cast<float>(v));
                    markDirty();
                  }
                });
        form->addRow(label, box);
      };
      addCameraFloat("Distance", cam.distance, 0.5, 64.0,
                     [](auto &c, float v) { c.distance = v; });
      addCameraFloat("Pitch", cam.pitch, -1.5, 1.5,
                     [](auto &c, float v) { c.pitch = v; });
      addCameraFloat("Yaw", cam.yaw, -1000.0, 1000.0,
                     [](auto &c, float v) { c.yaw = v; });
      addCameraFloat("FOV", cam.fov, 10.0, 150.0,
                     [](auto &c, float v) { c.fov = v; });
      addCameraFloat("Near Clip", cam.nearClip, 0.001, 100.0,
                     [](auto &c, float v) { c.nearClip = v; });
      addCameraFloat("Far Clip", cam.farClip, 1.0, 100000.0,
                     [](auto &c, float v) { c.farClip = v; });
      const char *pivotLabels[] = {"Pivot X", "Pivot Y", "Pivot Z"};
      const float pivotValues[] = {cam.pivotOffset.x, cam.pivotOffset.y,
                                   cam.pivotOffset.z};
      for (int axis = 0; axis < 3; ++axis)
      {
        addCameraFloat(
            pivotLabels[axis], pivotValues[axis], -10000.0, 10000.0,
            [axis](auto &c, float v)
            {
              float *values[] = {&c.pivotOffset.x, &c.pivotOffset.y,
                                 &c.pivotOffset.z};
              *values[axis] = v;
            });
      }
      auto *targetTag = new QLineEdit(QString::fromStdString(cam.targetTag), host);
      connect(targetTag, &QLineEdit::editingFinished, this,
              [this, entity, targetTag, markDirty]()
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) &&
                    s.hasComponent<Nebula::CameraComponent>(entity))
                {
                  s.getComponent<Nebula::CameraComponent>(entity).targetTag =
                      targetTag->text().toStdString();
                  markDirty();
                }
              });
      form->addRow("Target Tag", targetTag);
    }

    // --- Collider ---
    if (scene.hasComponent<Nebula::ColliderComponent>(entity))
    {
      auto &col = scene.getComponent<Nebula::ColliderComponent>(entity);
      addComponentSection("Collider", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::ColliderComponent>(entity))
                            {
                              s.removeComponent<Nebula::ColliderComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                            }
                          });
      auto *shape = new QComboBox(host);
      shape->addItems({"Box", "Sphere"});
      shape->setCurrentIndex(
          col.shape == Nebula::ColliderComponent::Shape::Sphere ? 1 : 0);
      connect(shape, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              [this, entity, markDirty](int index)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) &&
                    s.hasComponent<Nebula::ColliderComponent>(entity))
                {
                  s.getComponent<Nebula::ColliderComponent>(entity).shape =
                      index == 1 ? Nebula::ColliderComponent::Shape::Sphere
                                 : Nebula::ColliderComponent::Shape::Box;
                  markDirty();
                }
              });
      form->addRow("Shape", shape);
      const char *axisNames[] = {"Half X", "Half Y", "Half Z"};
      float *axisValues[] = {&col.halfExtents.x, &col.halfExtents.y, &col.halfExtents.z};
      for (int i = 0; i < 3; ++i)
      {
        auto *box = new QDoubleSpinBox(host);
        box->setRange(0.01, 100.0);
        box->setDecimals(3);
        box->setValue(*axisValues[i]);
        box->setKeyboardTracking(false);
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, entity, i, markDirty](double v)
                {
                  Nebula::Scene &s = m_app->scene();
                  if (!s.isValidEntity(entity) || !s.hasComponent<Nebula::ColliderComponent>(entity))
                  {
                    return;
                  }
                  auto &c = s.getComponent<Nebula::ColliderComponent>(entity);
                  float *axes[] = {&c.halfExtents.x, &c.halfExtents.y, &c.halfExtents.z};
                  *axes[i] = static_cast<float>(v);
                  markDirty();
                });
        form->addRow(axisNames[i], box);
      }
      auto *trigger = new QCheckBox("Is Trigger", host);
      trigger->setChecked(col.isTrigger);
      auto *isStatic = new QCheckBox("Is Static", host);
      isStatic->setChecked(col.isStatic);
      connect(trigger, &QCheckBox::toggled, this, [this, entity, markDirty](bool v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::ColliderComponent>(entity))
                {
                  s.getComponent<Nebula::ColliderComponent>(entity).isTrigger = v;
                  markDirty();
                } });
      connect(isStatic, &QCheckBox::toggled, this, [this, entity, markDirty](bool v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::ColliderComponent>(entity))
                {
                  s.getComponent<Nebula::ColliderComponent>(entity).isStatic = v;
                  markDirty();
                } });
      form->addRow(QString(), trigger);
      form->addRow(QString(), isStatic);
      if (scene.hasComponent<Nebula::MeshRendererComponent>(entity))
      {
        auto *fit = new QPushButton("Fit to Mesh", host);
        connect(fit, &QPushButton::clicked, this, [this, entity]()
                {
                  Nebula::Scene &s = m_app->scene();
                  if (s.isValidEntity(entity) &&
                      s.hasComponent<Nebula::ColliderComponent>(entity) &&
                      s.hasComponent<Nebula::MeshRendererComponent>(entity) &&
                      Nebula::fitBoxColliderToMeshRenderer(
                          s.getComponent<Nebula::ColliderComponent>(entity),
                          m_app->assetManagerRef(),
                          s.getComponent<Nebula::MeshRendererComponent>(entity)))
                  {
                    m_app->editorState().sceneDirty = true;
                    rebuildInspector();
                  }
                });
        form->addRow(QString(), fit);
      }
    }

    // --- Rigid Body ---
    if (scene.hasComponent<Nebula::RigidBodyComponent>(entity))
    {
      auto &body = scene.getComponent<Nebula::RigidBodyComponent>(entity);
      addComponentSection("Rigid Body", [this, entity]()
                          {
                            Nebula::Scene &s = m_app->scene();
                            if (s.isValidEntity(entity) &&
                                s.hasComponent<Nebula::RigidBodyComponent>(entity))
                            {
                              s.removeComponent<Nebula::RigidBodyComponent>(entity);
                              m_app->editorState().sceneDirty = true;
                            }
                          });
      auto *mass = new QDoubleSpinBox(host);
      mass->setRange(0.01, 1000.0);
      mass->setValue(body.mass);
      mass->setKeyboardTracking(false);
      connect(mass, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
              [this, entity, markDirty](double v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::RigidBodyComponent>(entity))
                {
                  s.getComponent<Nebula::RigidBodyComponent>(entity).mass = static_cast<float>(v);
                  markDirty();
                }
              });
      form->addRow("Mass", mass);
      auto *kinematic = new QCheckBox("Kinematic", host);
      kinematic->setChecked(body.kinematic);
      connect(kinematic, &QCheckBox::toggled, this, [this, entity, markDirty](bool v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::RigidBodyComponent>(entity))
                {
                  s.getComponent<Nebula::RigidBodyComponent>(entity).kinematic = v;
                  markDirty();
                } });
      form->addRow(QString(), kinematic);
    }

    // --- Environment ---
    if (scene.hasComponent<Nebula::EnvironmentComponent>(entity))
    {
      auto &env = scene.getComponent<Nebula::EnvironmentComponent>(entity);
      form->addRow(sectionLabel("Environment", host));

      auto *fog = new QDoubleSpinBox(host);
      fog->setRange(0.0, 1.0);
      fog->setDecimals(4);
      fog->setSingleStep(0.005);
      fog->setValue(env.fogDensity);
      fog->setKeyboardTracking(false);
      connect(fog, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
              [this, entity, markDirty](double v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::EnvironmentComponent>(entity))
                {
                  s.getComponent<Nebula::EnvironmentComponent>(entity).fogDensity = static_cast<float>(v);
                  markDirty();
                }
              });
      form->addRow("Fog density", fog);

      auto *light = new QDoubleSpinBox(host);
      light->setRange(0.0, 5.0);
      light->setDecimals(3);
      light->setValue(env.lightIntensity);
      light->setKeyboardTracking(false);
      connect(light, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
              [this, entity, markDirty](double v)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::EnvironmentComponent>(entity))
                {
                  s.getComponent<Nebula::EnvironmentComponent>(entity).lightIntensity =
                      static_cast<float>(v);
                  markDirty();
                }
              });
      form->addRow("Light intensity", light);

      auto *tod = new QComboBox(host);
      tod->addItems({"Day", "Dusk", "Night"});
      tod->setCurrentIndex(std::clamp(env.timeOfDayPreset, 0, 2));
      connect(tod, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              [this, entity, markDirty](int idx)
              {
                Nebula::Scene &s = m_app->scene();
                if (s.isValidEntity(entity) && s.hasComponent<Nebula::EnvironmentComponent>(entity))
                {
                  s.getComponent<Nebula::EnvironmentComponent>(entity).timeOfDayPreset = idx;
                  markDirty();
                }
              });
      form->addRow("Time of day", tod);
    }

    auto *addComponent = new QPushButton("Add Component", host);
    auto *componentMenu = new QMenu(addComponent);
    addComponent->setMenu(componentMenu);
    const auto finishAdd = [this]()
    {
      m_app->editorState().sceneDirty = true;
      QTimer::singleShot(0, this, [this]() { rebuildInspector(); });
    };
    if (!scene.hasComponent<Nebula::MeshRendererComponent>(entity))
    {
      componentMenu->addAction("Mesh Renderer", this, [this, entity, finishAdd]()
                               {
                                 auto &m = m_app->scene().addComponent<
                                     Nebula::MeshRendererComponent>(entity);
                                 m.m_meshPath = "builtin/meshes/cube";
                                 m.m_materialPath = "builtin/materials/cube";
                                 m_app->resolveSceneAssetsNow();
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::TagComponent>(entity))
    {
      componentMenu->addAction("Tag", this, [this, entity, finishAdd]()
                               {
                                 m_app->scene().addComponent<Nebula::TagComponent>(entity).tag =
                                     "Entity";
                                 refreshHierarchy();
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::ScriptComponent>(entity))
    {
      componentMenu->addAction("Script", this, [this, entity, finishAdd]()
                               {
                                 auto &sc = m_app->scene().addComponent<
                                     Nebula::ScriptComponent>(entity);
                                 const auto ids =
                                     m_app->scriptRegistryRef().registeredScriptIds();
                                 if (!ids.empty())
                                 {
                                   sc.scriptName = ids.front();
                                   sc.paramsJson =
                                       m_scriptParams.mergeScriptParamDefaults(
                                           "{}", m_app->fieldRegistryRef(), sc.scriptName);
                                 }
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::CameraComponent>(entity))
    {
      componentMenu->addAction("Camera", this, [this, entity, finishAdd]()
                               {
                                 m_app->scene().addComponent<Nebula::CameraComponent>(entity);
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::ColliderComponent>(entity))
    {
      componentMenu->addAction("Collider", this, [this, entity, finishAdd]()
                               {
                                 m_app->scene().addComponent<Nebula::ColliderComponent>(entity);
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::RigidBodyComponent>(entity))
    {
      componentMenu->addAction("Rigid Body", this, [this, entity, finishAdd]()
                               {
                                 m_app->scene().addComponent<Nebula::RigidBodyComponent>(entity);
                                 finishAdd();
                               });
    }
    if (!scene.hasComponent<Nebula::EnvironmentComponent>(entity))
    {
      componentMenu->addAction("Environment", this, [this, entity, finishAdd]()
                               {
                                 m_app->scene().addComponent<
                                     Nebula::EnvironmentComponent>(entity);
                                 finishAdd();
                               });
    }
    form->addRow(addComponent);

    m_inspectorScroll->setWidget(host);
  }

  void QtMainWindow::refreshConsole()
  {
    const auto &lines = m_app->editorLog().getLines();
    const int count = static_cast<int>(lines.size());
    if (count == m_lastLogCount)
    {
      return;
    }
    const bool wasAtBottom =
        m_console->verticalScrollBar()->value() >=
        m_console->verticalScrollBar()->maximum() - 2;

    // Append only new lines — rebuilding the full document every frame kills FPS.
    if (count < m_lastLogCount || m_lastLogCount < 0)
    {
      m_console->clear();
      m_lastLogCount = 0;
    }
    QTextCursor cursor(m_console->document());
    cursor.movePosition(QTextCursor::End);
    for (int i = m_lastLogCount; i < count; ++i)
    {
      const auto &line = lines[static_cast<std::size_t>(i)];
      const char *prefix = "[INFO] ";
      QColor color(220, 222, 226);
      if (line.level == Nebula::LogLevel::Warn)
      {
        prefix = "[WARN] ";
        color = QColor(245, 200, 70);
      }
      else if (line.level == Nebula::LogLevel::Error)
      {
        prefix = "[ERROR] ";
        color = QColor(245, 90, 80);
      }
      QTextCharFormat format;
      format.setForeground(color);
      cursor.insertText(QString::fromUtf8(prefix) +
                            QString::fromStdString(line.text) + '\n',
                        format);
    }
    m_lastLogCount = count;
    if (wasAtBottom)
    {
      m_console->moveCursor(QTextCursor::End);
    }
  }

  void QtMainWindow::refreshDebug()
  {
    if (m_debugText == nullptr)
    {
      return;
    }
    // Debug panel is diagnostic-only; updating QPlainTextEdit every frame is expensive.
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (nowMs - m_lastDebugRefreshMs < 100)
    {
      return;
    }
    m_lastDebugRefreshMs = nowMs;

    Nebula::Input &input = m_app->inputRef();
    Nebula::ActionMapping &actions = m_app->actionsRef();
    const auto keyState = [&input](Nebula::Tasto key)
    {
      QString state;
      if (input.isKeyDown(key))
        state += "Held ";
      if (input.wasKeyPressed(key))
        state += "Pressed ";
      if (input.wasKeyReleased(key))
        state += "Released";
      return state.isEmpty() ? QString("-") : state.trimmed();
    };
    const auto actionState = [&input, &actions](Nebula::Action action)
    {
      QString state;
      if (actions.isActionDown(action, input))
        state += "Down ";
      if (actions.wasActionPressed(action, input))
        state += "Pressed";
      return state.isEmpty() ? QString("-") : state.trimmed();
    };

    float moveX = 0.f;
    float moveY = 0.f;
    float lookX = 0.f;
    float lookY = 0.f;
    actions.getAxisValue(Nebula::Axis::MoveX, input, moveX, moveY);
    actions.getAxisValue(Nebula::Axis::MoveY, input, moveX, moveY);
    actions.getAxisValue(Nebula::Axis::LookX, input, lookX, lookY);
    actions.getAxisValue(Nebula::Axis::LookY, input, lookX, lookY);

    const QString text = QString(
        "INPUT STATE\n"
        "Mouse delta: %1, %2   Left: %3   Right: %4\n"
        "W: %5  A: %6  S: %7  D: %8  Space: %9  Shift: %10\n"
        "MoveForward: %11  MoveBackward: %12  MoveLeft: %13  MoveRight: %14\n"
        "Jump: %15  FastFall: %16  LightAttack: %17  HeavyAttack: %18\n"
        "Axes move(%19, %20) look(%21, %22)")
            .arg(input.mouseDeltaX(), 0, 'f', 2)
            .arg(input.mouseDeltaY(), 0, 'f', 2)
            .arg(input.isMouseButtonDown(Nebula::TastoDelMouse::left) ? "Down" : "-")
            .arg(input.isMouseButtonDown(Nebula::TastoDelMouse::right) ? "Down" : "-")
            .arg(keyState(Nebula::Tasto::w))
            .arg(keyState(Nebula::Tasto::a))
            .arg(keyState(Nebula::Tasto::s))
            .arg(keyState(Nebula::Tasto::d))
            .arg(keyState(Nebula::Tasto::space))
            .arg(keyState(Nebula::Tasto::left_shift))
            .arg(actionState(Nebula::Action::MoveForward))
            .arg(actionState(Nebula::Action::MoveBackward))
            .arg(actionState(Nebula::Action::MoveLeft))
            .arg(actionState(Nebula::Action::MoveRight))
            .arg(actionState(Nebula::Action::Jump))
            .arg(actionState(Nebula::Action::FastFall))
            .arg(actionState(Nebula::Action::LightAttack))
            .arg(actionState(Nebula::Action::HeavyAttack))
            .arg(moveX, 0, 'f', 2)
            .arg(moveY, 0, 'f', 2)
            .arg(lookX, 0, 'f', 2)
            .arg(lookY, 0, 'f', 2);
    if (text != m_lastDebugText)
    {
      m_lastDebugText = text;
      m_debugText->setPlainText(text);
    }
  }

  void QtMainWindow::rebuildDebugTuning()
  {
    if (m_debugTuningForm == nullptr)
    {
      return;
    }
    while (QLayoutItem *item = m_debugTuningForm->takeAt(0))
    {
      delete item->widget();
      delete item;
    }
    m_debugTuningForm->addRow(sectionLabel("Combat Tuning", m_debugTuningHost));

    Nebula::Scene &scene = m_app->scene();
    Nebula::Entity director{};
    for (const Nebula::Entity entity : scene.getAllEntities())
    {
      if (scene.hasComponent<Nebula::ScriptComponent>(entity) &&
          scene.getComponent<Nebula::ScriptComponent>(entity).scriptName ==
              "CombatDirector")
      {
        director = entity;
        break;
      }
    }
    if (!scene.isValidEntity(director))
    {
      m_debugTuningForm->addRow(new QLabel("No CombatDirector entity in scene.",
                                           m_debugTuningHost));
      return;
    }
    if (m_app->playing())
    {
      auto *live = new QLabel("Live tuning active", m_debugTuningHost);
      live->setStyleSheet("color: #66d17a;");
      m_debugTuningForm->addRow(live);
    }

    auto &script = scene.getComponent<Nebula::ScriptComponent>(director);
    script.paramsJson = m_scriptParams.mergeScriptParamDefaults(
        script.paramsJson, m_app->fieldRegistryRef(), script.scriptName);
    const auto *fields = m_app->fieldRegistryRef().getFields(script.scriptName);
    if (fields == nullptr)
    {
      return;
    }
    for (const auto &field : *fields)
    {
      if (!field.visibleInEditor)
      {
        continue;
      }
      const std::string fieldName = field.name;
      if (field.type == Nebula::ScriptFieldType::Float)
      {
        auto *box = new QDoubleSpinBox(m_debugTuningHost);
        box->setRange(-100000.0, 100000.0);
        box->setDecimals(3);
        box->setSingleStep(0.1);
        box->setKeyboardTracking(false);
        box->setValue(m_scriptParams.readScriptParamFloat(script.paramsJson, field));
        connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, director, fieldName](double value)
                {
                  Nebula::Scene &s = m_app->scene();
                  if (!s.isValidEntity(director) ||
                      !s.hasComponent<Nebula::ScriptComponent>(director))
                  {
                    return;
                  }
                  auto &sc = s.getComponent<Nebula::ScriptComponent>(director);
                  sc.paramsJson = m_scriptParams.setScriptParamFloat(
                      sc.paramsJson, fieldName, static_cast<float>(value));
                  m_app->editorState().sceneDirty = true;
                });
        m_debugTuningForm->addRow(QString::fromStdString(fieldName), box);
      }
      else if (field.type == Nebula::ScriptFieldType::Int)
      {
        auto *box = new QSpinBox(m_debugTuningHost);
        box->setRange(-1000000, 1000000);
        box->setKeyboardTracking(false);
        box->setValue(m_scriptParams.readScriptParamInt(script.paramsJson, field));
        connect(box, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, director, fieldName](int value)
                {
                  Nebula::Scene &s = m_app->scene();
                  if (!s.isValidEntity(director) ||
                      !s.hasComponent<Nebula::ScriptComponent>(director))
                  {
                    return;
                  }
                  auto &sc = s.getComponent<Nebula::ScriptComponent>(director);
                  sc.paramsJson =
                      m_scriptParams.setScriptParamInt(sc.paramsJson, fieldName, value);
                  m_app->editorState().sceneDirty = true;
                });
        m_debugTuningForm->addRow(QString::fromStdString(fieldName), box);
      }
    }
  }

}
