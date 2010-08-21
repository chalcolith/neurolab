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

#include "neurocell.h"
#include "neuronet.h"
#include "../automata/exception.h"
#include <cmath>

namespace NeuroLib
{

    NeuroCell::NeuroCell(const KindOfCell & k,
                         const NeuroValue & weight,
                         const NeuroValue & run,
                         const NeuroValue & current_value)
        : _kind(k), _frozen(false),
        _weight(weight), _run(run),
        _output_value(current_value),
        _running_average(current_value)
    {
    }

    void NeuroCell::addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->addEdge(my_index, input_index);
    }

    void NeuroCell::removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->removeEdge(my_index, input_index);
    }

    static const NeuroCell::NeuroValue ZERO = static_cast<NeuroCell::NeuroValue>(0);
    static const NeuroCell::NeuroValue ONE = static_cast<NeuroCell::NeuroValue>(1.0f);
    static const NeuroCell::NeuroValue SLOPE_Y = static_cast<NeuroCell::NeuroValue>(0.99f);
    static const NeuroCell::NeuroValue SLOPE_OFFSET = static_cast<NeuroCell::NeuroValue>(6.0f);
    static const NeuroCell::NeuroValue EPSILON = static_cast<NeuroCell::NeuroValue>(0.000001f);
    static const NeuroCell::NeuroValue MAX_LINK = static_cast<NeuroCell::NeuroValue>(1.1f);

    void NeuroCell::update(NEURONET_BASE *neuronet, const NeuroIndex &, NeuroCell & next, 
                           const QVector<int> & neighbor_indices, const NeuroCell *const neighbors) const
    {
        const NeuroCell & prev = *this;
        NeuroNet *network = dynamic_cast<NeuroNet *>(neuronet);

        next._frozen = prev._frozen;

        if (prev._frozen)
        {
            next._output_value = prev._output_value;
            return;
        }

        NeuroValue input_sum = 0;
        for (int i = 0; i < neighbor_indices.size(); ++i)
            input_sum += neighbors[i]._output_value;

        NeuroValue next_value = 0;
        NeuroValue diff, delta;
        int num_neighbors;

        switch (prev._kind)
        {
        case NODE:
            // node output
            if (input_sum > 0)
            {
                NeuroValue slope = (SLOPE_OFFSET - ::log(ONE/SLOPE_Y - ONE)) / prev._run;
                next_value = ONE / (ONE + ::exp(SLOPE_OFFSET - slope * (input_sum - (prev._weight - prev._run))));
            }
            else
            {
                next_value = 0;
            }

            next_value = qMax(next_value, prev._output_value * (ONE - network->decay()));

            // hebbian learning (link learning)
            num_neighbors = neighbor_indices.size();
            for (int i = 0; i < num_neighbors; ++i)
            {
                const NeuroCell & incoming = neighbors[i];

                if (incoming._kind == EXCITORY_LINK)
                {
                    NeuroValue delta_weight = network->linkLearnRate() * (incoming._output_value - incoming._running_average) * (next_value - prev._running_average);

                    if (qAbs(delta_weight) > EPSILON)
                    {
                        NeuroValue new_weight = qBound(ZERO, incoming.weight() + delta_weight, MAX_LINK);
                        network->addPostUpdate(NeuroNet::PostUpdateRec(neighbor_indices[i], new_weight));
                    }
                }
            }

            // node raising/lowering
            diff = qBound(ZERO, next_value - prev._running_average, ONE);
            delta = network->nodeLearnRate() * (diff * diff * diff - network->nodeForgetRate());
            next._weight = qBound(ZERO, next._weight + delta, next._weight + delta);
            
            break;
        case OSCILLATOR:
            {
                NeuroStep phase = prev._phase_step[0];
                NeuroStep step = prev._phase_step[1];

                NeuroStep gap = prev._gap_peak[0];
                NeuroStep peak = prev._gap_peak[1];

                // increment step
                NeuroStep max = static_cast<NeuroStep>(-1);
                while ((max - (step+1)) < phase)
                    step += gap + peak; // make step overflow, and inc it beyond phase, so it doesn't pause
                step += 1;

                if (step >= phase)
                {
                    // calculate next output
                    if ((gap+peak > 0) && ((phase+step) % (gap+peak)) < peak)
                        next_value = ONE;
                    else
                        next_value = ZERO;
                }
                else
                {
                    next_value = ZERO;
                }

                // save new step value
                next._phase_step[1] = step;
            }

            break;
        case EXCITORY_LINK:
            // we allow the weight to be 1.1 so as to maintain activation
            next_value = qBound(ZERO, input_sum * prev._weight, MAX_LINK);
            break;
        case INHIBITORY_LINK:
            // the weight should be negative, so only clip the inputs
            next_value = qBound(ZERO, input_sum, ONE) * prev._weight;
            break;
        default:
            break;
        }

        next._output_value = next_value;
        next._running_average = (next._output_value + (network->learnTime() - ONE)*prev._running_average) / network->learnTime();
    }

    void NeuroCell::writeBinary(QDataStream & ds, const Automata::AutomataFileVersion &) const
    {
        ds << static_cast<quint8>(_kind);
        ds << static_cast<bool>(_frozen);

        switch (_kind)
        {
        case NeuroCell::NODE:
        case NeuroCell::EXCITORY_LINK:
        case NeuroCell::INHIBITORY_LINK:
            ds << static_cast<float>(_weight);
            ds << static_cast<float>(_run);
            break;
        case NeuroCell::OSCILLATOR:
            ds << static_cast<quint16>(_gap_peak[0]);
            ds << static_cast<quint16>(_gap_peak[1]);
            ds << static_cast<quint16>(_phase_step[0]);
            ds << static_cast<quint16>(_phase_step[1]);
            break;
        default:
            break;
        }

        ds << static_cast<float>(_output_value);
        ds << static_cast<float>(_running_average);
    }

    void NeuroCell::readBinary(QDataStream & ds, const Automata::AutomataFileVersion &)
    {
        // if (file_version.client_version >= NeuroLib::NEUROLIB_FILE_VERSION_OLD)
        {
            quint8 k;
            quint16 s;
            float n;
            bool f;

            ds >> k; _kind = static_cast<NeuroCell::KindOfCell>(k);
            ds >> f; _frozen = static_cast<bool>(f);

            switch (_kind)
            {
            case NeuroCell::NODE:
            case NeuroCell::EXCITORY_LINK:
            case NeuroCell::INHIBITORY_LINK:
                ds >> n; _weight = static_cast<NeuroCell::NeuroValue>(n);
                ds >> n; _run = static_cast<NeuroCell::NeuroValue>(n);
                break;
            case NeuroCell::OSCILLATOR:
                ds >> s; _gap_peak[0] = static_cast<NeuroCell::NeuroStep>(s);
                ds >> s; _gap_peak[1] = static_cast<NeuroCell::NeuroStep>(s);
                ds >> s; _phase_step[0] = static_cast<NeuroCell::NeuroStep>(s);
                ds >> s; _phase_step[1] = static_cast<NeuroCell::NeuroStep>(s);
                break;
            default:
                break;
            }

            ds >> n; _output_value = static_cast<NeuroCell::NeuroValue>(n);
            ds >> n; _running_average = static_cast<NeuroCell::NeuroValue>(n);
        }
    }

} // namespace NeuroLib
