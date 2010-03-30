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

    /// Updates the cell.
    void NeuroCell::Update::operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *_network,
                                        const NeuroIndex &, const NeuroCell & prev, NeuroCell & next,
                                        const QVector<int> & neighbor_indices, const NeuroCell * const * const neighbors) const
    {
        NeuroNet *network = dynamic_cast<NeuroNet *>(_network);

        next._frozen = prev._frozen;

        if (prev._frozen)
        {
            next._output_value = prev._output_value;
            return;
        }

        NeuroValue input_sum = 0;
        for (int i = 0; i < neighbor_indices.size(); ++i)
            input_sum += neighbors[i]->_output_value;

        NeuroValue next_value;

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

            next_value = qMax(next_value, prev._output_value * (ONE - network->_decay));

            // hebbian learning
            for (int i = 0; i < neighbor_indices.size(); ++i)
            {
                NeuroCell & incoming = (*network)[neighbor_indices[i]].current();

                if (incoming._kind == EXCITORY_LINK)
                {
                    NeuroValue delta_weight = network->learn() * (incoming._output_value - incoming._running_average) * (next_value - prev._running_average);
                    incoming.setWeight(qBound(ZERO, incoming.weight() + delta_weight, ONE));
                }
            }

            break;
        case OSCILLATOR:
            {
                NeuroStep step = prev._phase_step[0];
                NeuroStep phase = prev._phase_step[1];

                if (step >= phase)
                {
                    NeuroStep gap = prev._gap_peak[0];
                    NeuroStep peak = prev._gap_peak[1];

                    if ((step % (gap + peak)) < peak)
                        next_value = 1;
                    else
                        next_value = 0;

                    NeuroStep max = static_cast<NeuroStep>(-1);
                    while ((max - (step+1)) < phase)
                        step += gap + peak; // make step overflow, and inc it beyond phase, so it doesn't pause
                    step += 1;
                }
                else
                {
                    next_value = 0;
                }
            }

            break;
        case EXCITORY_LINK:
            next_value = qBound(ZERO, input_sum * prev._weight, ONE);
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

    QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc)
    {
        ds << static_cast<quint8>(nc._kind);
        ds << static_cast<bool>(nc._frozen);

        ds << static_cast<float>(nc._weight);

        if (nc._kind == NeuroCell::NODE)
        {
            ds << static_cast<float>(nc._run);
        }

        ds << static_cast<float>(nc._output_value);
        ds << static_cast<float>(nc._running_average);

        return ds;
    }

    QDataStream & operator>> (QDataStream & ds, NeuroCell & nc)
    {
        quint8 k;
        float n;
        bool f;

        ds >> k; nc._kind = static_cast<NeuroCell::KindOfCell>(k);
        ds >> f; nc._frozen = static_cast<bool>(f);

        ds >> n; nc._weight = static_cast<NeuroCell::NeuroValue>(n);

        if (nc._kind == NeuroCell::NODE)
        {
            ds >> n; nc._run = static_cast<NeuroCell::NeuroValue>(n);
        }

        ds >> n; nc._output_value = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; nc._running_average = static_cast<NeuroCell::NeuroValue>(n);

        return ds;
    }

} // namespace NeuroLib
