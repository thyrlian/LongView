#pragma once
#include <QWidget>

// Forward declarations
class QVBoxLayout;
class QCheckBox;
class QPushButton;
class QLabel;
class QString;
class QSize;
class QEvent;

namespace LongView {
namespace Tiles {

/**
 * @brief Abstract base class for all Tile components
 * 
 * A Tile acts as a visual container that can be expanded/collapsed
 * and marked as completed. This is the foundation for both ItemTile
 * and GroupTile implementations.
 */
class Tile : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(Tile)
    
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
    Q_PROPERTY(bool completed READ isCompleted WRITE setCompleted NOTIFY completedChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

protected:
    // UI constants - accessible to subclasses
    static constexpr int kMargin = 8;
    static constexpr int kSpacing = 4;
    static constexpr int kHeaderSpacing = 8;
    static constexpr int kButtonSize = 20;
    static constexpr int kDefaultWidth = 360;
    static constexpr int kDefaultHeight = 240;
    static constexpr int kMinWidth = 200;
    static constexpr int kMinHeight = 120;

public:
    enum class Kind { Item, Group };
    
    explicit Tile(Kind kind, QWidget* parent = nullptr);
    virtual ~Tile() override;

    // Core tile properties
    bool isExpanded() const { return m_expanded; }
    bool isCompleted() const { return m_completed; }
    Kind kind() const { return m_kind; }
    
    // Core tile methods
    void setExpanded(bool expanded);
    void setCompleted(bool completed);
    void toggleExpanded();
    
    // Title interface
    void setTitle(const QString& title);
    QString title() const;
    
    // Content widget interface
    /**
     * @brief Set the content widget for this tile
     * 
     * The widget will be reparented to this tile and added to the layout.
     * Any existing content widget will be removed and deleted.
     * 
     * @param widget The widget to set as content (can be nullptr)
     */
    void setContentWidget(QWidget* widget);
    QWidget* contentWidget() const;
    
    // Virtual interface for subclasses
    virtual void refresh() = 0;
    
    // Event handling
    void changeEvent(QEvent* e) override;
    
    // Size hints for better layout prediction
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void expandedChanged(bool expanded);
    void completedChanged(bool completed);
    void titleChanged(const QString& title);

protected:
    // Protected methods for subclasses
    virtual void setupUI();
    virtual void updateUI();
    void loadStyleSheet();
    
    // Core tile state
    Kind m_kind;
    bool m_expanded = false;  // Default collapsed
    bool m_completed = false;
    
    // UI components
    QVBoxLayout* m_mainLayout = nullptr;
    QWidget* m_headerWidget = nullptr;
    QWidget* m_contentWidget = nullptr;
    QCheckBox* m_completionCheckBox = nullptr;
    QPushButton* m_expandButton = nullptr;
    QLabel* m_titleLabel = nullptr;

protected slots:
    void onExpandButtonClicked();
    void onCompletionCheckBoxChanged(int state);
};

} // namespace Tiles
} // namespace LongView
