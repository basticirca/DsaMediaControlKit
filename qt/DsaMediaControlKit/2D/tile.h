#ifndef TWO_D_TILE_H
#define TWO_D_TILE_H

#include <QObject>
#include <QGraphicsItem>
#include <QElapsedTimer>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include <QLineF>
#include <QMouseEvent>
#include <QMediaPlayer>

namespace TwoD {

class Tile : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem) // so instances can be casted using qobject_cast
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal size MEMBER size_ READ getSize WRITE setSize)

protected:
    /*
     * state of the graphics item
    */
    enum ItemMode {
        IDLE,
        HOVER,
        SELECTED,
        MOVE
    };

    /*
     *  side of a 2d box
    */
    enum BOX_SIDE {
        LEFT,
        RIGHT,
        UPPER,
        LOWER
    };

public:
    Tile(QGraphicsItem* parent = 0);
    virtual ~Tile();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    // does not take overlap into account
    virtual void setSize(qreal size);
    virtual qreal getSize() const;

    virtual void setSizeLayoutAware(qreal size);

    virtual void setName(const QString& str);
    virtual const QString& getName() const;

    virtual const QMenu* getContextMenu() const;

signals:
    void mousePressed(QGraphicsSceneMouseEvent* e);
    void mouseReleased(QGraphicsSceneMouseEvent* e);
    void mouseMoved(QGraphicsSceneMouseEvent* e);
    void hoverEntered(QGraphicsSceneHoverEvent *e);
    void hoverLeft(QGraphicsSceneHoverEvent *e);

protected slots:
    /* slot to enable move mode after timer */
    virtual void onLongClick();

    /* sets small size for tile */
    virtual void setSmallSize();

    /* sets medium size for tile */
    virtual void setMediumSize();

    /* sets large size for tile */
    virtual void setLargeSize();

    /* removes this item from the scene and schedules deletion (see deleteLater) */
    virtual void onDelete();

protected:
    /*
     * BC overrides
    */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* e);

    /*
    * relayouts all other tiles based on overlaps created by resize operation
    */
    virtual void fixOverlapsAfterResize(qreal prev_size);

    /*
    * Returns QRectF definition for draw area
    */
    virtual const QRectF getPaintRect() const;

    /*
    * Returns default background color brush based on ItemMode
    */
    virtual const QBrush getBackgroundBrush() const;

    /*
    * Sets default opacity value based on ItemState
    */
    virtual void setDefaultOpacity();

    /*
     * setter
    */
    void setMode(ItemMode mode);

    /*
     * distance between given point and line.
    */
    static qreal distance(const QPointF& p, const QLineF& l);

    /*
     * closest side of given box to given point.
    */
    static BOX_SIDE closestSide(const QPointF& p, const QRectF& rect);

    /*
     * creates context menu
    */
    virtual void createContextMenu();

    QString name_;
    QTimer* long_click_timer_;
    int long_click_duration_;
    ItemMode mode_;
    qreal size_;
    QMenu* context_menu_;
};

} // namespace TwoD

#endif // TWO_D_TILE_ITEM_H
