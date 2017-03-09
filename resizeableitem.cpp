/****************************************************************************
** Copyright (C) 2016 Olaf Japp
**
** This file is part of AnimationMaker.
**
**  AnimationMaker is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  AnimationMaker is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with AnimationMaker.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "resizeableitem.h"
#include "animationscene.h"
#include "keyframe.h"
#include <QStyleOptionGraphicsItem>
#include <QGraphicsItem>
#include <QGuiApplication>

#include <QTest>
#include <QGraphicsScene>
#include <QMenu>

ResizeableItem::ResizeableItem()
{
    m_hasHandles = false;
    m_xscale = 1;
    m_yscale = 1;
    m_animations = new QList<QPropertyAnimation *>();
    m_keyframes = new QList<KeyFrame *>();

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    delAct = new QAction(tr("Delete"), this);
    delAct->setShortcut(tr("Delete"));
    connect(delAct, SIGNAL(triggered()), this, SLOT(deleteItem()));

    bringToFrontAct = new QAction("Bring to front");
    connect(bringToFrontAct, SIGNAL(triggered()), this, SLOT(bringToFront()));

    sendToBackAct = new QAction("Send to back");
    connect(sendToBackAct, SIGNAL(triggered()), this, SLOT(sendToBack()));

    raiseAct = new QAction("Raise");
    connect(raiseAct, SIGNAL(triggered()), this, SLOT(raise()));

    lowerAct = new QAction("Lower");
    connect(lowerAct, SIGNAL(triggered()), this, SLOT(lower()));

    m_opacityAct = new QAction("Opacity");
    connect(m_opacityAct, SIGNAL(triggered()), this, SLOT(addOpacityAnimation()));

    m_leftAct = new QAction("Left");
    connect(m_leftAct, SIGNAL(triggered()), this, SLOT(addLeftAnimation()));

    m_topAct = new QAction("Top");
    connect(m_topAct, SIGNAL(triggered()), this, SLOT(addTopAnimation()));

    m_animationMenu = new QMenu("Animate");
    m_animationMenu->addAction(m_opacityAct);
    m_animationMenu->addAction(m_leftAct);
    m_animationMenu->addAction(m_topAct);
    m_contextMenu = new QMenu();
    m_contextMenu->addAction(delAct);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(bringToFrontAct);
    m_contextMenu->addAction(raiseAct);
    m_contextMenu->addAction(lowerAct);
    m_contextMenu->addAction(sendToBackAct);
    m_contextMenu->addSeparator();
    m_contextMenu->addMenu(m_animationMenu);
}

void ResizeableItem::addKeyframe(KeyFrame *frame)
{
    m_keyframes->append(frame);
}

void ResizeableItem::addAnimation(QPropertyAnimation *anim)
{
    m_animations->append(anim);
}

int ResizeableItem::getAnimationCount() const
{
    return m_animations->count();
}

QPropertyAnimation *ResizeableItem::getAnimation(int row)
{
    return m_animations->at(row);
}

void ResizeableItem::drawHighlightSelected(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    qreal itemPenWidth = m_pen.widthF();
    const qreal pad = itemPenWidth / 2;
    const qreal penWidth = 0;
    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( fgcolor.red()   > 127 ? 0 : 255, fgcolor.green() > 127 ? 0 : 255, fgcolor.blue()  > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(pad, pad, -pad, -pad));

    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(pad, pad, -pad, -pad));
}

QRectF ResizeableItem::rect() const
{
    return m_rect;
}

void ResizeableItem::scaleObjects() {}
void ResizeableItem::setScale(qreal x, qreal y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
}

qreal ResizeableItem::xscale()
{
    return m_xscale;
}

qreal ResizeableItem::yscale()
{
    return m_yscale;
}

void ResizeableItem::setRect(qreal x, qreal y, qreal w, qreal h)
{
    prepareGeometryChange();
    m_rect = QRectF(x, y, w, h);
    update();
    emit sizeChanged(w, h);
}

QString ResizeableItem::id() const
{
    return m_id;
}

void ResizeableItem::setId(const QString value)
{
    m_id = value;
    emit idChanged(this, value);
}

void ResizeableItem::setWidth(qreal value)
{
    prepareGeometryChange();
    m_rect.setWidth(value);
    update();
    setHandlePositions();
}

void ResizeableItem::setHeight(qreal value)
{
    prepareGeometryChange();
    m_rect.setHeight(value);
    update();
    setHandlePositions();
}

QPen ResizeableItem::pen() const
{
    return m_pen;
}

void ResizeableItem::setPen(const QPen &pen)
{
    m_pen = pen;
    update();
}

QBrush ResizeableItem::brush() const
{
    return m_brush;
}
void ResizeableItem::setBrush(const QBrush &brush)
{
    m_brush = brush;
    update();
}

void ResizeableItem::paint( QPainter *paint, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(paint);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QRectF ResizeableItem::boundingRect() const
{
    return rect();
}

bool ResizeableItem::sceneEventFilter(QGraphicsItem * watched, QEvent * event)
{
    ItemHandle * handle = dynamic_cast<ItemHandle *>(watched);
    if ( handle == NULL)
    {
        return false;
    }
    QGraphicsSceneMouseEvent * mevent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if ( mevent == NULL)
    {
        return false;
    }

    switch (event->type() )
    {
        case QEvent::GraphicsSceneMousePress:
        {
            handle->setMouseState(ItemHandle::kMouseDown);
            handle->mouseDownX = mevent->pos().x();
            handle->mouseDownY = mevent->pos().y();
        }
            break;

        case QEvent::GraphicsSceneMouseRelease:
            handle->setMouseState(ItemHandle::kMouseReleased);
            break;

        case QEvent::GraphicsSceneMouseMove:
            handle->setMouseState(ItemHandle::kMouseMoving );
            break;

        default:
            return false;
    }

    if ( handle->getMouseState() == ItemHandle::kMouseMoving )
    {
        qreal x = mevent->pos().x(), y = mevent->pos().y();

        int XaxisSign = 0;
        int YaxisSign = 0;
        switch(handle->getCorner())
        {
            case 0:
            {
                XaxisSign = +1;
                YaxisSign = +1;
                break;
            }
            case 1:
            {
                XaxisSign = -1;
                YaxisSign = +1;
                break;
            }
            case 2:
            {
                XaxisSign = -1;
                YaxisSign = -1;
                break;
            }
            case 3:
            {
                XaxisSign = +1;
                YaxisSign = -1;
                break;
            }
            case 4:
            {
                YaxisSign = +1;
                break;
            }
            case 5:
            {
                XaxisSign = -1;
                break;
            }
            case 6:
            {
                YaxisSign = -1;
                break;
            }
            case 7:
            {
                XaxisSign = +1;
                break;
            }
        }

        int xMoved = handle->mouseDownX - x;
        int yMoved = handle->mouseDownY - y;

        int newWidth = rect().width() + ( XaxisSign * xMoved);
        if ( newWidth < 40 ) newWidth  = 40;

        int newHeight = rect().height() + (YaxisSign * yMoved);
        if ( newHeight < 40 ) newHeight = 40;

        qreal deltaWidth = newWidth - rect().width();
        qreal deltaHeight = newHeight - rect().height();

        bool controlPressed = QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);
        if(controlPressed)
        {
            qreal ratio = rect().width() / rect().height();
            if(handle->getCorner() < 4) // corners
            {
                if(newWidth > newHeight)
                    deltaWidth = (int)(deltaHeight * ratio);
                else
                    deltaHeight = (int)(deltaWidth / ratio);
            }
            else
            {
                if(handle->getCorner() == 4 || handle->getCorner() == 6) // top | bottom
                    deltaWidth = deltaHeight * ratio;
                else // left | right
                    deltaHeight = deltaWidth / ratio;
            }
        }

        setRect(0,0,rect().width() + deltaWidth, rect().height() + deltaHeight);

        scaleObjects();

        deltaWidth *= (-1);
        deltaHeight *= (-1);

        switch(handle->getCorner())
        {
            case 0:
            {
                int newXpos = this->pos().x() + deltaWidth;
                int newYpos = this->pos().y() + deltaHeight;
                this->setPos(newXpos, newYpos);
                posChanged(newXpos, newYpos);
                break;
            }
            case 1:
            {
                int newYpos = this->pos().y() + deltaHeight;
                this->setPos(this->pos().x(), newYpos);
                posChanged(this->pos().x(), newYpos);
                break;
            }
            case 3:
            {
                int newXpos = this->pos().x() + deltaWidth;
                this->setPos(newXpos, this->pos().y());
                posChanged(newXpos, this->pos().y());
                break;
            }
            case 4: // top
            {
                if(controlPressed)
                {
                    int newYpos = this->pos().y() + deltaHeight;
                    qreal newXpos = this->pos().x() + deltaWidth / 2;
                    this->setPos(newXpos, newYpos);
                    posChanged(newXpos, newYpos);
                }
                else
                {
                    int newYpos = this->pos().y() + deltaHeight;
                    this->setPos(this->pos().x(), newYpos);
                    posChanged(this->pos().x(), newYpos);
                }
                break;
            }
            case 5: // right
            {
                if(controlPressed)
                {
                    qreal newYpos = this->pos().y() + deltaHeight / 2;
                    this->setPos(this->pos().x(), newYpos);
                    posChanged(this->pos().x(), newYpos);
                }
                break;
            }
            case 6: // bottom
            {
                if(controlPressed)
                {
                    qreal newXpos = this->pos().x() + deltaWidth / 2;
                    this->setPos(newXpos, this->pos().y());
                    posChanged(newXpos, this->pos().y());
                }
                break;
            }
            case 7: // left
            {
                if(controlPressed)
                {
                    int newXpos = this->pos().x() + deltaWidth;
                    qreal newYpos = this->pos().y() + deltaHeight / 2;
                    this->setPos(newXpos, newYpos);
                    posChanged(newXpos, newYpos);
                }
                else
                {
                    int newXpos = this->pos().x() + deltaWidth;
                    this->setPos(newXpos, this->pos().y());
                    posChanged(newXpos, this->pos().y());
                }
                break;
            }
        }

        setHandlePositions();

        this->update();
    }
    return true;
}

void ResizeableItem::setHandlePositions()
{
    m_handles[0]->setPos(-4, -4);
    m_handles[1]->setPos(rect().width() - 4,  -4);
    m_handles[2]->setPos(rect().width() - 4, rect().height() - 4);
    m_handles[3]->setPos(-4,  rect().height() - 4);
    m_handles[4]->setPos(rect().width() / 2 - 4, -4);
    m_handles[5]->setPos(rect().width() - 4,  rect().height() / 2 - 4);
    m_handles[6]->setPos(rect().width() /2 - 4, rect().height() - 4);
    m_handles[7]->setPos(-4,  rect().height() / 2 - 4);
}

QVariant ResizeableItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (value == true)
        {
            if(!m_hasHandles)
            {
                for(int i = 0; i < 8; i++)
                {
                    m_handles[i] = new ItemHandle(this,i);
                    m_handles[i]->installSceneEventFilter(this);
                }
                setHandlePositions();
                m_hasHandles = true;
            }
        }
        else
        {
            for(int i = 0; i < 8; i++)
            {
                m_handles[i]->setParentItem(NULL);
                delete m_handles[i];
            }
            m_hasHandles = false;
        }
    }
    else if(change == QGraphicsItem::ItemPositionHasChanged)
    {
        if(isSelected())
        {
            QPointF newPos = value.toPointF();
            posChanged(newPos.x(), newPos.y());
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void ResizeableItem::posChanged(qreal x, qreal y)
{
    adjustKeyframes("left", QVariant(x));
    adjustKeyframes("top", QVariant(y));
    emit positionChanged(x, y);
}

/*
 * Looking for the keyframe which occures in front of the playhead position
 * and adjust its value
 */
void ResizeableItem::adjustKeyframes(QString propertyName, QVariant value)
{
    AnimationScene *as = dynamic_cast<AnimationScene *>(scene());
    if(as)
    {
        int time = as->playheadPosition();
        std::sort(m_keyframes->begin(), m_keyframes->end(), compareKeyframes);
        KeyFrame *found = NULL;
        for(int i=0; i < m_keyframes->count(); i++)
        {
            KeyFrame *key = m_keyframes->at(i);
            if(key->propertyName() == propertyName && key->time() <= time)
            {
                found = key;
            }
        }
        if(found)
            found->setValue(value);
    }
}

void ResizeableItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    m_contextMenu->exec(event->screenPos());
}

void ResizeableItem::lower()
{
    int pos = scene()->items().indexOf(this);
    for(int i = pos + 1; i < scene()->items().count(); i++)
    {
        QGraphicsItem *item = scene()->items().at(i);
        if(isAnimationMakerItem(item))
        {
            this->stackBefore(item);
            break;
        }
    }
    // trick to repaint item
    this->setSelected(false);
    this->setSelected(true);
}

void ResizeableItem::raise()
{
    int pos = scene()->items().indexOf(this);
    for(int i = pos - 1; i >= 0; i--)
    {
        QGraphicsItem *item = scene()->items().at(i);
        if(isAnimationMakerItem(item))
        {
            item->stackBefore(this);
            break;
        }
    }
    // trick to repaint item
    this->setSelected(false);
    this->setSelected(true);
}

void ResizeableItem::bringToFront()
{
    int pos = scene()->items().indexOf(this);
    for(int i = pos - 1; i >= 0; i--)
    {
        QGraphicsItem *item = scene()->items().at(i);
        if(isAnimationMakerItem(item))
        {
            item->stackBefore(this);
        }
    }
    // trick to repaint item
    this->setSelected(false);
    this->setSelected(true);
}

void ResizeableItem::sendToBack()
{
    int pos = scene()->items().indexOf(this);
    for(int i = pos + 1; i < scene()->items().count(); i++)
    {
        QGraphicsItem *item = scene()->items().at(i);
        if(isAnimationMakerItem(item))
        {
            this->stackBefore(item);
        }
    }
    // trick to repaint item
    this->setSelected(false);
    this->setSelected(true);
}

void ResizeableItem::deleteItem()
{
    scene()->removeItem(this);
}

void ResizeableItem::addOpacityAnimation()
{
    emit addPropertyAnimation(this, "opacity", opacity(), 0, 1);
}

void ResizeableItem::addLeftAnimation()
{
    emit addPropertyAnimation(this, "left", x(), -10000, 10000);
}

void ResizeableItem::addTopAnimation()
{
    emit addPropertyAnimation(this, "top", y(), -10000, 10000);
}
