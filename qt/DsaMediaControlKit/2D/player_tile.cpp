#include "player_tile.h"
#include "resources/resources.h"

#include <QGraphicsScene>

namespace TwoD {

PlayerTile::PlayerTile(QGraphicsItem *parent)
    : Tile(parent)
    , player_(0)
    , is_playing_(false)
{
    player_ = new QMediaPlayer(this);
}

PlayerTile::~PlayerTile()
{
}

void PlayerTile::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    scene()->update(scene()->sceneRect());

    setDefaultOpacity();

    QRectF p_rect = getPaintRect();

    // paint
    painter->fillRect(p_rect, getBackgroundBrush());
    if(p_rect.width() > 0 && p_rect.height() > 0) {
        painter->drawPixmap(
            (int) p_rect.x(),
            (int)p_rect.y(),
            (int)p_rect.width(),
            (int)p_rect.height(),
            getPlayStatePixmap()
        );
    }
    if(mode_ == HOVER)
        painter->drawText(QPointF(p_rect.x(), p_rect.y()-5), name_);
}

void PlayerTile::setMedia(const QMediaContent &c)
{
    player_->setMedia(c);
}

void PlayerTile::play()
{
    if(!player_->media().isNull() && !is_playing_) {
        player_->play();
        is_playing_ = true;
    }
}

void PlayerTile::stop()
{
    if(!player_->media().isNull() && is_playing_) {
        player_->stop();
        is_playing_ = false;
    }
}

void PlayerTile::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && mode_ != MOVE) {
        if(is_playing_)
            stop();
        else
            play();
    }

    Tile::mouseReleaseEvent(e);
}

const QPixmap PlayerTile::getPlayStatePixmap() const
{
    if(is_playing_)
        return *Resources::PX_STOP;
    else
        return *Resources::PX_PLAY;
}

} // namespace TwoD
