#ifndef NEURONODE_H
#define NEURONODE_H

#include <QSet>
#include <QDataStream>

#include "../automata/automaton.h"
#include "neurolib_global.h"

namespace NeuroLib
{

    class NeuroNet;

    /// Base class for elements of the neurocognitive network (nodes or links).
    class NEUROLIBSHARED_EXPORT NeuroCell
    {
    public:
        /// As the update operation does not allow for polymorphism, we need to know what kind of
        /// cell we're dealing with.
        enum KindOfCell
        {
            NODE = 0,
            EXCITORY_LINK,
            INHIBITORY_LINK,
            OSCILLATOR,
            NUM_KINDS
        };

        /// The type used to index into the neurocognitive network automaton.
        typedef qint32 NeuroIndex;

        /// The type used for real number values in the neural network.
        typedef float NeuroValue;

        typedef quint16 NeuroStep;

        /// Constructor.
        /// \param k The kind of cell.
        /// \param weight For links, the output weight; for nodes, the input weight (roughly, the input value required to produce an output close to 1).
        /// \param run The distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \param current_value The current output value to initialize the cell.
        NeuroCell(const KindOfCell & k = NeuroCell::NODE,
                  const NeuroValue & weight = 1.0f,
                  const NeuroValue & run = 0.1f,
                  const NeuroValue & current_value = 0);

        /// \return The kind of cell.
        inline const KindOfCell & kind() const { return _kind; }

        /// \return Whether or not the node is "frozen".  A frozen node will not be updated, but maintain its current output value.
        /// \see NeuroCell::setFrozen()
        inline const bool & frozen() const { return _frozen; }

        /// Sets the "frozen" state of the node.
        /// \see NeuroCell::frozen()
        inline void setFrozen(const bool & frozen) { _frozen = frozen; }

        /// \return The "weight" of the cell.  For links, this is the output weight; for nodes, this is the input weight (roughly, the input value required to produce an output close to 1).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setWeight()
        const NeuroValue & weight() const { return _weight; }

        /// Set the "weight" of the cell.  For links, this is the output weight; for nodes, this is the input weight (roughly, the input value required to produce an output close to 1).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::weight()
        void setWeight(const NeuroValue & weight) { _weight = weight; }

        /// \return The distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setRun()
        const NeuroValue & run() const { return _run; }

        /// Sets the distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::run()
        void setRun(const NeuroValue & run) { _run = run; }

        const NeuroStep & gap() const { return _gap_peak[0]; }
        void setGap(const NeuroStep & gap) { _gap_peak[0] = gap; }

        const NeuroStep & peak() const { return _gap_peak[1]; }
        void setPeak(const NeuroStep & peak) { _gap_peak[1] = peak; }

        const NeuroStep & phase() const { return _phase_step[0]; }
        void setPhase(const NeuroStep & phase) { _phase_step[0] = phase; }

        const NeuroStep & step() const { return _phase_step[1]; }
        void setStep(const NeuroStep & step) { _phase_step[1] = step; };

        /// \return The current output value of the cell.
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setCurrentValue()
        const NeuroValue & outputValue() const { return _output_value; }

        /// Set the current output value of the cell.
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::currentValue()
        void setOutputValue(const NeuroValue & v) { _output_value = _running_average = v; }

        /// \return The running average of the cell's output values.
        /// \see NeuroNet::learnTime()
        const NeuroValue & runningAverage() const { return _running_average; }

        /// Adds an input to the cell.  A node can have links as input, while links can only have inhibitory links as inputs.
        void addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);

        /// Removes an input from the cell.
        void removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);

        /// A functor providing the update operation for neural network cells.
        struct Update
        {
            void operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *neuronet,
                             const NeuroIndex & index, const NeuroCell & prev, NeuroCell & next,
                             const QVector<int> & neighbor_indices, const NeuroCell * const * const neighbors) const;
        };

    private:
        KindOfCell _kind;
        bool _frozen;

        union
        {
            NeuroValue _weight; ///< Used in links for their weight; in nodes for their input thresholds.
            NeuroStep  _gap_peak[2]; ///< Used by oscillators for their gap and peak values.
        };

        union
        {
            NeuroValue _run; ///< The width of the slope in the sigmoid curve (for nodes).
            NeuroStep  _phase_step[2]; ///< For oscillators, the phase of the oscillator (the delay before it starts), and the current timestep.
        };

        NeuroValue _output_value;
        NeuroValue _running_average;

        friend NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    }; // class NeuroCell

    /// Write a cell to a data stream.
    extern NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);

    /// Read a cell from a data stream.
    extern NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);

} // namespace NeuroLib

#endif // NEURONODE_H
