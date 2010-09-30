#ifndef MIXINREMEMBER_H
#define MIXINREMEMBER_H

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

#include "neurogui_global.h"
#include "neurolinkitem.h"

namespace NeuroGui
{

    /// Provides functionality for remembering the positions of linked nodes.
    class NEUROGUISHARED_EXPORT MixinRemember
    {
        NeuroItem *_self;

    protected:
        QMap<NeuroLinkItem *, QVector2D> _incomingAttachments, _outgoingAttachments;

    public:
        MixinRemember(NeuroItem *self);
        virtual ~MixinRemember();

    protected:
        void onAttachedBy(NeuroLinkItem *link);
        void adjustLinks();

        virtual void adjustLink(NeuroLinkItem *link, QList<NeuroLinkItem *> & alreadyAdjusted);
        virtual QVector2D getAttachPos(const QVector2D & dirTo) = 0;
    };

} // namespace NeuroGui

#endif // MIXINREMEMBER_H
