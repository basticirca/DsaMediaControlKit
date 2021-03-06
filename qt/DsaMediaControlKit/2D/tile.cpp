#include "tile.h"

#include <QDebug>
#include <QMimeData>
#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <QJsonArray>

#include "resources/resources.h"
#include "misc/char_input_dialog.h"

#define OFFSET 10

namespace TwoD {

Tile::Tile(QGraphicsItem* parent)
    : QObject(0)
    , QGraphicsItem(parent)
    , name_()
    , long_click_timer_()
    , long_click_duration_(300)
    , mode_(IDLE)
    , size_(1)
    , context_menu_(0)
    , activate_action_(0)
    , activate_key_(' ')
{    
    long_click_timer_ = new QTimer(this);
    connect(long_click_timer_, SIGNAL(timeout()),
            this, SLOT(onLongClick()));

    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    context_menu_ = new QMenu;

    activate_action_ = new QAction("Activate", this);

    connect(activate_action_, SIGNAL(triggered()),
            this, SLOT(onActivate()));
}

Tile::~Tile()
{
    context_menu_->deleteLater();
}

void Tile::init()
{
    createContextMenu();
}

QRectF Tile::boundingRect() const
{
    return QRectF(0,0,100*size_,100*size_);
}

void Tile::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    scene()->update(scene()->sceneRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    setDefaultOpacity();

    QRectF p_rect(getPaintRect());

    painter->fillRect(p_rect, getBackgroundBrush());
    if(p_rect.width() > 0 && p_rect.height() > 0) {
        painter->setOpacity(0.6);
        painter->drawPixmap((int) p_rect.x(), (int)p_rect.y(), (int) p_rect.width(), (int) p_rect.height(), getOverlayPixmap());
        painter->setOpacity(1.0);
        QPixmap act_px = getActivatePixmap();
        if(!act_px.isNull())
            painter->drawPixmap((int) p_rect.x()+5, (int) p_rect.y()+5, (int) p_rect.width() / 4, (int) p_rect.height() / 4, act_px);
    }
}

void Tile::setActivateKey(const QChar &c)
{
    activate_key_ = c;
}

const QChar &Tile::getActivateKey() const
{
    return activate_key_;
}

void Tile::setSize(qreal size)
{
    size_ = size;
}

qreal Tile::getSize() const
{
    return size_;
}

void Tile::setSizeAnimated(qreal size)
{
    qreal prev_size = size_;

    setSizeLayoutAware(size);

    QPropertyAnimation* anim = new QPropertyAnimation(this, "size");
    anim->setStartValue(prev_size);
    anim->setEndValue(size);
    anim->setDuration(300);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
}

void Tile::setSizeLayoutAware(qreal size)
{
    qreal prev_size = size_;
    size_ = size;
    fixOverlapsAfterResize(prev_size);

    scene()->update(scene()->sceneRect());
}

void Tile::setName(const QString &str)
{
    name_ = str;
}

const QString &Tile::getName() const
{
    return name_;
}

const QMenu *Tile::getContextMenu() const
{
    return context_menu_;
}

bool Tile::hasActivateKey() const
{
    return activate_key_ != ' ';
}

void Tile::receiveExternalData(const QMimeData *data)
{
    qDebug() << "Tile " << name_ <<" : Received Data "<< data->text();
}

void Tile::receiveWheelEvent(QWheelEvent *event)
{
    //do something
    qDebug() << "receiving wheel event";
}

const QJsonObject Tile::toJsonObject() const
{
    QJsonObject obj;

    obj["name"] = name_;
    obj["size"] = size_;
    QJsonArray arr_pos;
    arr_pos.append(pos().x());
    arr_pos.append(pos().y());
    obj["position"] = arr_pos;
    if(hasActivateKey())
        obj["activate_key"] = QString(activate_key_);

    return obj;
}

bool Tile::setFromJsonObject(const QJsonObject &obj)
{
    // check format
    if(obj.isEmpty())
        return false;
    if(!(obj.contains("name") && obj.contains("size") && obj.contains("position")))
        return false;
    if(!(obj["name"].isString() && obj["size"].isDouble() && obj["position"].isArray()))
        return false;

    QJsonArray arr_pos = obj["position"].toArray();

    if(obj["position"].toArray().size() != 2)
        return false;

    // set name, size and pos
    setName(obj["name"].toString());
    setSize((qreal) obj["size"].toDouble());
    setPos(QPointF(arr_pos[0].toDouble(), arr_pos[1].toDouble()));

    // set activate key
    if(obj.contains("activate_key") && obj["activate_key"].isString()) {
        QString k = obj["activate_key"].toString();
        if(k.size() == 1)
            setActivateKey(k.at(0));
    }

    return true;
}

void Tile::onActivate()
{
    emit activated();
}

void Tile::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button() == Qt::LeftButton) {
        setMode(SELECTED);
        long_click_timer_->start(long_click_duration_);
    }
    else if(e->button() == Qt::RightButton) {
        // qDebug() << "right button";
    }
    else if(e->button() == Qt::MidButton) {
        // qDebug() << "mid button";
    }

    QGraphicsItem::mousePressEvent(e);
    emit mousePressed(e);
}

void Tile::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    setMode(IDLE);
    long_click_timer_->stop();
    QGraphicsItem::mouseReleaseEvent(e);
    emit mouseReleased(e);
}

void Tile::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if(mode_ == MOVE) {
        QPointF p = pos();
        QGraphicsItem::mouseMoveEvent(e);
        QPointF p_new = pos();

        QList<QGraphicsItem*> col_it = collidingItems(Qt::IntersectsItemBoundingRect);
        if(col_it.size() > 0) {
            qreal x_min = col_it[0]->pos().x() - boundingRect().width();
            qreal x_max = col_it[0]->pos().x() + col_it[0]->boundingRect().width();
            qreal y_min = col_it[0]->pos().y() - boundingRect().height();
            qreal y_max = col_it[0]->pos().y() + col_it[0]->boundingRect().height();

            QRectF rect(QPointF(x_min, y_min), QPointF(x_max, y_max));

            switch(closestSide(p_new, rect)) {
                case LEFT:
                    p_new.setX(x_min);
                    break;
                case RIGHT:
                    p_new.setX(x_max);
                    break;
                case UPPER:
                    p_new.setY(y_min);
                    break;
                case LOWER:
                    p_new.setY(y_max);
                    break;
            }
            setPos(p_new);
        }

        // check if item in scene bounds
        qreal max_x = scene()->width() - boundingRect().width();
        qreal max_y = scene()->height() - boundingRect().height();
        if (x() < 0)
            setPos(0, y());
        else if (x() > max_x)
            setPos(max_x, y());

        if (y() < 0)
            setPos(x(), 0);
        else if (y() > max_y)
            setPos(x(), max_y);

        // if still colliding set pos back to start
        col_it = collidingItems(Qt::IntersectsItemBoundingRect);
        if(col_it.size() > 0)
            setPos(p);
    }

    emit mouseMoved(e);
}

void Tile::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    if(mode_ == IDLE)
        setMode(HOVER);
    emit hoverEntered(e);
    e->accept();
}

void Tile::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    if(mode_ == HOVER)
        setMode(IDLE);
    emit hoverLeft(e);
    e->accept();
}

void Tile::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsItem::dragEnterEvent(event);
}

void Tile::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsItem::dragMoveEvent(event);
}

void Tile::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsItem::dropEvent(event);
}

void Tile::contextMenuEvent(QGraphicsSceneContextMenuEvent *e)
{
    context_menu_->popup(e->screenPos());
}

void Tile::fixOverlapsAfterResize(qreal prev_size)
{
    if(prev_size >= size_)
        return;

    QList<QGraphicsItem*> cols = collidingItems(Qt::IntersectsItemShape);
    // no collision, keep layout
    if(cols.size() == 0)
        return;

    // all movement animations will have to be perfromed
    // after seting the psoitions statically for quick intersect computation
    QMap<QGraphicsItem*,QPropertyAnimation*> animations;

    // fix overlapping
    qreal dist = abs(size_ * 100 - prev_size * 100);
    bool collision = true;
    QSet<QGraphicsItem*> items = scene()->items().toSet();
    items.remove(this);
    while(collision) {
        collision = false;
        QSet<QGraphicsItem*> remove_items;
        foreach(QGraphicsItem* it, items) {
            // cast necessary for property animation
            Tile* c_it = dynamic_cast<Tile*>(it);

            // skip non GraphicsItems
            if(!c_it)
                continue;

            /*if(c_it->x() < x()-c_it->boundingRect().width() || c_it->y() < y()-c_it->boundingRect().height()) {
                remove_items.insert(it);
                continue;
            }*/

            // do not move non colliding
            if(c_it->collidingItems(Qt::IntersectsItemShape).size() == 0)
                continue;

            // move item along extension of sized item
            QPointF start = c_it->pos();
            QPointF end(c_it->pos().x()+dist, c_it->pos().y()+dist);

            // start and end might be same for started animations
            //if((start - end).manhattanLength() < 0.0001)
            //    continue;

            // actual new collsion found
            collision = true;

            if(!animations.contains(it)) {
                // store positional change animation
                QPropertyAnimation* anim = new QPropertyAnimation(c_it, "pos");
                anim->setDuration(300);
                anim->setStartValue(start);
                anim->setEndValue(end);
                anim->setEasingCurve(QEasingCurve::InOutQuad);
                animations[it] = anim;
            }

            // set final pos statically for now
            // overlap computation will be easier that way
            // after all final positions are reached
            // position change will be animated again
            c_it->setPos(end);

            // add to list of moved items
            // (use 'it' since 'c_it' is casted)
            remove_items.insert(it);

            // extend scene bounds if item gets pushed out of screen
            // x
            qreal max_x = scene()->width() - c_it->boundingRect().width();
            if (end.x() > max_x) {
                qreal screen_ext = max_x - end.x();
                QRectF rect = scene()->sceneRect();
                rect.setWidth(rect.width() + abs(screen_ext));
                scene()->setSceneRect(rect);
            }
            // y
            qreal max_y = scene()->height() - c_it->boundingRect().height();
            if (end.y() > max_y) {
                qreal screen_ext = max_y - end.y();
                QRectF rect = scene()->sceneRect();
                rect.setHeight(rect.height() + abs(screen_ext));
                scene()->setSceneRect(rect);
            }
        }

        // limit items to the ones, which have not been moved
        items.subtract(remove_items);
    }

    // trigger transitions
    foreach(QPropertyAnimation* anim, animations.values())
        anim->start(QAbstractAnimation::DeleteWhenStopped);

}

const QRectF Tile::getPaintRect() const
{
    QRectF paint_rect = boundingRect();
    paint_rect.setX(paint_rect.x()+OFFSET);
    paint_rect.setY(paint_rect.y()+OFFSET);
    paint_rect.setWidth(paint_rect.width()-OFFSET);
    paint_rect.setHeight(paint_rect.height()-OFFSET);
    return paint_rect;
}

const QBrush Tile::getBackgroundBrush() const
{
    QBrush b(Qt::gray);

    switch(mode_) {
        case SELECTED:
            b.setColor(Qt::green);
            break;

        default:
            break;
    }

    return b;
}

const QPixmap Tile::getOverlayPixmap() const
{
    if(mode_ == SELECTED)
        return *Resources::PX_CRACKED_STONE_INV;
    else
        return *Resources::PX_CRACKED_STONE;
}

const QPixmap Tile::getActivatePixmap() const
{
    QPixmap* px = Resources::getKeyPixmap(activate_key_);
    if(px == 0)
        return QPixmap();
    return *px;
}

void Tile::setDefaultOpacity()
{
    switch(mode_) {
        case MOVE:
            setOpacity(0.2);
            break;

        default:
            setOpacity(1.0);
            break;
    }

}

void Tile::setMode(Tile::ItemMode mode)
{
    mode_ = mode;
    update(boundingRect());
}

void Tile::setSmallSize()
{
    setSizeAnimated(1);
}

void Tile::setMediumSize()
{
    setSizeAnimated(2);
}

void Tile::setLargeSize()
{
    setSizeAnimated(3);
}

void Tile::onLongClick()
{
    setMode(MOVE);
}

void Tile::onDelete()
{
    scene()->removeItem(this);
    deleteLater();
}

void Tile::onSetKey()
{
    Misc::CharInputDialog d;
    if(d.exec()) {
        setActivateKey(d.getChar());
        activate_action_->setShortcut(QKeySequence(QString(d.getChar())));
    }
}

qreal Tile::distance(const QPointF &p, const QLineF &l)
{
    // transform to loocal coordinates system (0,0) - (lx, ly)
    QPointF p1 = l.p1();
    QPointF p2 = l.p2();
    qreal x = p.x() - p1.x();
    qreal y = p.y() - p1.y();
    qreal x2 = p2.x() - p1.x();
    qreal y2 = p2.y() - p1.y();

    // if line is a point (nodes are the same) =>
    // just return distance between point and one line node
    qreal norm = sqrt(x2*x2 + y2*y2);
    if (norm <= std::numeric_limits<int>::epsilon())
      return sqrt(x*x + y*y);

    // distance
    return fabs(x*y2 - y*x2) / norm;
}

Tile::BOX_SIDE Tile::closestSide(const QPointF &p, const QRectF &rect)
{
    qreal x_min = rect.x();
    qreal x_max = rect.x() + rect.width();
    qreal y_min = rect.y();
    qreal y_max = rect.y() + rect.height();
    qreal temp_dist = 0;

    // left
    QLineF l(QPointF(x_min, y_min), QPointF(x_min, y_max));
    qreal min_dist = distance(p,l);
    BOX_SIDE side = LEFT;

    // right
    l.setPoints(QPointF(x_max,y_min), QPointF(x_max, y_max));
    temp_dist = distance(p,l);
    if(temp_dist < min_dist) {
        min_dist = temp_dist;
        side = RIGHT;
    }

    // upper
    l.setPoints(QPointF(x_min, y_min), QPointF(x_max, y_min));
    temp_dist = distance(p,l);
    if(temp_dist < min_dist) {
        min_dist = temp_dist;
        side = UPPER;
    }

    // lower
    l.setPoints(QPointF(x_min, y_max), QPointF(x_max, y_max));
    temp_dist = distance(p,l);
    if(temp_dist < min_dist) {
        min_dist = temp_dist;
        side = LOWER;
    }

    return side;
}

void Tile::createContextMenu()
{
    // create size actions
    QAction* small_size_action = new QAction(tr("Small"), this);
    QAction* medium_size_action = new QAction(tr("Medium"), this);
    QAction* large_size_action = new QAction(tr("Large"), this);

    connect(small_size_action, SIGNAL(triggered()),
            this, SLOT(setSmallSize()));
    connect(medium_size_action, SIGNAL(triggered()),
            this, SLOT(setMediumSize()));
    connect(large_size_action, SIGNAL(triggered()),
            this, SLOT(setLargeSize()));

    // create size menu
    QMenu* size_menu = new QMenu(tr("Size"));
    size_menu->addAction(small_size_action);
    size_menu->addAction(medium_size_action);
    size_menu->addAction(large_size_action);

    // create delete action
    QAction* delete_action = new QAction(tr("Delete"), this);

    connect(delete_action, SIGNAL(triggered()),
            this, SLOT(onDelete()));

    // change activate button
    QAction* activate_button_action = new QAction(tr("Set Key..."), this);

    connect(activate_button_action, SIGNAL(triggered()),
            this, SLOT(onSetKey()));

    // create context menu
    //context_menu_->addAction(activate_action_);
    context_menu_->addAction(activate_button_action);
    context_menu_->addMenu(size_menu);
    context_menu_->addSeparator();
    context_menu_->addAction(delete_action);
}

} // namespace TwoD
