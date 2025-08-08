#include "tile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QFile>
#include <QString>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QEvent>
#include <QSize>
#include <QDebug>

namespace {
// Empty namespace - UI constants moved to header
}

namespace LongView {
namespace Tiles {

Tile::Tile(Kind kind, QWidget* parent)
    : QWidget(parent)
    , m_kind(kind)
{
    setObjectName("LongViewTile");
    setupUI();
    updateUI();
}

Tile::~Tile()
{
    // Default destructor - Qt will handle cleanup
}

void Tile::setupUI()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    m_mainLayout->setSpacing(kSpacing);

    // Header widget
    m_headerWidget = new QWidget(this);
    auto headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(kHeaderSpacing);

    // Expand button
    m_expandButton = new QPushButton(m_headerWidget);
    m_expandButton->setFixedSize(kButtonSize, kButtonSize);
    m_expandButton->setFlat(true);
    m_expandButton->setToolTip(tr("Expand/Collapse"));
    m_expandButton->setFocusPolicy(Qt::StrongFocus);
    m_expandButton->setAccessibleName("tile-expand");
    headerLayout->addWidget(m_expandButton);

    // Completion checkbox
    m_completionCheckBox = new QCheckBox(m_headerWidget);
    m_completionCheckBox->setToolTip(tr("Mark as completed"));
    m_completionCheckBox->setFocusPolicy(Qt::NoFocus); // Avoid stealing focus
    m_completionCheckBox->setAccessibleName("tile-completed");
    headerLayout->addWidget(m_completionCheckBox);

    // Title label
    m_titleLabel = new QLabel(m_headerWidget);
    m_titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleLabel->setAccessibleName("tile-title");
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    m_mainLayout->addWidget(m_headerWidget, 0); // No stretch

    // Content widget
    m_contentWidget = new QWidget(this);
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainLayout->addWidget(m_contentWidget, 1); // Take remaining space

    // Connect signals
    connect(m_expandButton, &QPushButton::clicked, this, &Tile::onExpandButtonClicked);
    connect(m_completionCheckBox, &QCheckBox::stateChanged, this, &Tile::onCompletionCheckBoxChanged);

    // Load stylesheet
    loadStyleSheet();
}

void Tile::updateUI()
{
    // Defensive assertions
    Q_ASSERT(m_expandButton);
    Q_ASSERT(m_completionCheckBox);
    Q_ASSERT(m_contentWidget);

    // Update expand button icon using QStyle
    m_expandButton->setIcon(style()->standardIcon(
        m_expanded ? QStyle::SP_ArrowDown : QStyle::SP_ArrowRight
    ));
    m_expandButton->setText({}); // Clear button text

    // Update content visibility
    m_contentWidget->setVisible(m_expanded);

    // Update completion checkbox (block signals to avoid loops)
    {
        QSignalBlocker blocker(m_completionCheckBox);
        m_completionCheckBox->setChecked(m_completed);
    }
}

void Tile::setExpanded(bool expanded)
{
    if (m_expanded != expanded) {
        m_expanded = expanded;
        updateUI();
        emit expandedChanged(expanded);
    }
}

void Tile::setCompleted(bool completed)
{
    if (m_completed != completed) {
        m_completed = completed;
        updateUI();
        emit completedChanged(completed);
    }
}

void Tile::setTitle(const QString& title)
{
    if (m_titleLabel->text() != title) {
        m_titleLabel->setText(title);
        emit titleChanged(title);
    }
}

QString Tile::title() const
{
    return m_titleLabel->text();
}

void Tile::setContentWidget(QWidget* widget)
{
    if (m_contentWidget != widget) {
        // Remove old content widget
        if (m_contentWidget) {
            m_mainLayout->removeWidget(m_contentWidget);
            m_contentWidget->deleteLater(); // deleteLater() automatically reparents to nullptr
        }
        
        // Ensure we always have a valid widget (create placeholder if nullptr)
        if (!widget) {
            widget = new QWidget(this); // Placeholder widget
        }
        
        // Set new content widget
        m_contentWidget = widget;
        m_contentWidget->setParent(this);
        m_mainLayout->insertWidget(1, m_contentWidget, 1); // Ensure position after header
        m_contentWidget->setVisible(m_expanded);
    }
}

QWidget* Tile::contentWidget() const
{
    return m_contentWidget;
}

void Tile::onExpandButtonClicked()
{
    toggleExpanded();
}

void Tile::toggleExpanded()
{
    setExpanded(!m_expanded);
}

void Tile::loadStyleSheet()
{
    // Static cache for stylesheet to avoid repeated file reads
    static QString s_tileQss;
    if (s_tileQss.isEmpty()) {
        QFile f(":/assets/styles/tile.qss");
        if (f.open(QIODevice::ReadOnly)) {
            s_tileQss = QString::fromUtf8(f.readAll());
        } else {
            qWarning() << "Tile QSS not found, using fallback";
            s_tileQss = QString(); // keep empty; we'll set fallback below
        }
    }
    
    if (!s_tileQss.isEmpty()) {
        setStyleSheet(s_tileQss);
    } else {
        setStyleSheet(
            "#LongViewTile{background:#f5f5f5;border:1px solid #ddd;border-radius:4px}"
            "#LongViewTile QPushButton{background:transparent;border:none}"
            "#LongViewTile QCheckBox{background:transparent}"
        );
    }
}

void Tile::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::PaletteChange) {
        updateUI();
    }
}

QSize Tile::sizeHint() const
{
    return QSize(kDefaultWidth, kDefaultHeight);
}

QSize Tile::minimumSizeHint() const
{
    return QSize(kMinWidth, kMinHeight);
}

void Tile::onCompletionCheckBoxChanged(int state)
{
    setCompleted(state == Qt::Checked);
}

} // namespace Tiles
} // namespace LongView
