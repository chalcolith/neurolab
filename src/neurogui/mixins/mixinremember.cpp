/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "mixinremember.h"
#include "mixinarrow.h"
#include "../labscene.h"

namespace NeuroGui
{

    MixinRemember::MixinRemember(NeuroItem *self)
        : _self(self)
    {
    }

    MixinRemember::~MixinRemember()
    {
    }

    void MixinRemember::rememberItems(const QSet<NeuroItem *> &items, const QVector2D &center)
    {
        foreach (NeuroItem *ni, items)
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(ni);
            if (link)
            {
                if (link->frontLinkTarget() == _self)
                    _incomingAttachments[link] = QVector2D(link->line().p2()) - center;
                if (link->backLinkTarget() == _self)
                    _outgoingAttachments[link] = QVector2D(link->line().p1()) - center;
            }
        }
    }

    void MixinRemember::onAttachedBy(MixinArrow *link)
    {
        if (link->frontLinkTarget() == _self && link->dragFront())
            _incomingAttachments.remove(link);

        if (link->backLinkTarget() == _self && !link->dragFront())
            _outgoingAttachments.remove(link);

        // snap to node
        QSet<MixinArrow *> already;
        adjustLink(link, already);
    }

    void MixinRemember::onDetach(MixinArrow *link)
    {
        if (link->frontLinkTarget() == _self && link->dragFront())
            _incomingAttachments.remove(link);

        if (link->backLinkTarget() == _self && !link->dragFront())
            _outgoingAttachments.remove(link);
    }

    void MixinRemember::adjustLinks()
    {
        QSet<MixinArrow *> alreadyAdjusted;

        foreach (NeuroItem *ni, _self->connections())
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(ni);
            if (link && !alreadyAdjusted.contains(link))
                adjustLink(link, alreadyAdjusted);
        }
    }

    void MixinRemember::adjustLink(MixinArrow *link, QSet<MixinArrow *> & alreadyAdjusted)
    {
        LabScene *lab_scene = dynamic_cast<LabScene *>(_self->scene());
        Q_ASSERT(lab_scene);

        QVector2D center(_self->scenePos()); // in the scene

        bool frontLink = link->frontLinkTarget() == _self;
        bool backLink = link->backLinkTarget() == _self;

        bool rememberFront = _incomingAttachments.contains(link);
        bool rememberBack = _outgoingAttachments.contains(link);

        QLineF link_line = link->line();
        QVector2D front(link_line.p2()); front -= center; // link's front in my frame
        QVector2D back(link_line.p1()); back -= center; // link's back in my frame

        QVector2D mouse_pos(lab_scene->lastMousePos()); // in the scene
        QVector2D toPos = mouse_pos - center; // in my frame

        if (frontLink)
        {
            if (rememberFront)
            {
                front = _incomingAttachments[link];
            }
            else
            {
                front = getAttachPos(toPos);
                _incomingAttachments[link] = front;
            }
        }

        if (backLink)
        {
            if (rememberBack)
            {
                back = _outgoingAttachments[link];
            }
            else
            {
                back = getAttachPos(toPos);
                _outgoingAttachments[link] = back;
            }
        }

        if (frontLink && backLink)
        {
            QVector2D avg_dir = ((front + back) * 0.5).normalized();
            QPointF new_center = (center + avg_dir * (NeuroNarrowItem::NODE_WIDTH*3)).toPointF();
            QLineF new_line((back + center).toPointF(), (front + center).toPointF());

            link->setLine(new_line, &new_center);
        }
        else
        {
            link->setLine((back + center).toPointF(), (front + center).toPointF());
        }

        alreadyAdjusted.insert(link);
    }

    void MixinRemember::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        writeClipboard(_incomingAttachments, ds, id_map);
        writeClipboard(_outgoingAttachments, ds, id_map);
    }

    void MixinRemember::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        readClipboard(_incomingAttachments, ds, id_map);
        readClipboard(_outgoingAttachments, ds, id_map);
    }

    void MixinRemember::writeClipboard(const QMap<MixinArrow *, QVector2D> &attachments, QDataStream &ds, const QMap<int, int> &id_map) const
    {
        ds << static_cast<qint32>(attachments.size());
        foreach (MixinArrow *link, attachments.keys())
        {
            qint32 id = link->self()->id();

            if (id && id_map.contains(id))
            {
                ds << static_cast<qint32>(id_map[id]);
                ds << attachments[link];
            }
            else
            {
                ds << static_cast<qint32>(0);
            }
        }
    }

    void MixinRemember::readClipboard(QMap<MixinArrow *, QVector2D> &attachments, QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        qint32 num;
        ds >> num;

        attachments.clear();
        for (qint32 i = 0; i < num; ++i)
        {
            qint32 id;
            ds >> id;

            if (id)
            {
                QVector2D vec;
                ds >> vec;

                if (id_map.contains(id))
                {
                    MixinArrow *link = dynamic_cast<MixinArrow *>(id_map[id]);

                    if (link)
                        attachments.insert(link, vec);
                }
            }
        }
    }

} // namespace NeuroGui
