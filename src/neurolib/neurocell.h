#ifndef NEURONODE_H
#define NEURONODE_H

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

#include "neurolib_global.h"
#include "../automata/automaton.h"

#include <QSet>
#include <QDataStream>

namespace NeuroLib
{

    class NeuroNet;

    /// Base class for elements of the neurocognitive network (nodes or links).
    /// CANNOT be polymorphic, since the map algo takes an array of these...
    class NEUROLIBSHARED_EXPORT NeuroCell
    {
    public:
        /// As the update operation does not allow for polymorphism, we need to know what kind of
        /// cell we're dealing with.
        /// Don't add kinds in the middle!
        enum KindOfCell
        {
            NODE = 0,
            EXCITORY_LINK,
            INHIBITORY_LINK,
            OSCILLATOR,
            THRESHOLD_INHIBITORY_LINK,
            NUM_KINDS
        };

        /// The type used to index into the neurocognitive network automaton.
        typedef qint32 Index;

        /// The type used for real number values in the neural network.
        typedef float Value;

        /// Used to count steps for oscillators.
        typedef quint16 Step;

        typedef Automata::Automaton<NeuroCell, NeuroCell::Index> NEURONET_BASE;

        /// Constructor.
        /// \param k The kind of cell.
        /// \param weight For links, the output weight; for nodes, the input weight (roughly, the input value required to produce an output close to 1).
        /// \param run The distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \param current_value The current output value to initialize the cell.
        NeuroCell(const KindOfCell & k = NeuroCell::NODE,
                  const Value & weight = 1.0f,
                  const Value & run = 0.1f,
                  const Value & current_value = 0);

        /// \return The kind of cell.
        inline const KindOfCell & kind() const { return _kind; }

        /// \return Whether or not the node is "frozen".  A frozen node will not be updated, but maintain its current output value.
        /// \see NeuroCell::setFrozen()
        inline const bool & frozen() const { return _frozen; }

        /// Sets the "frozen" state of the node.
        /// \see NeuroCell::frozen()
        inline void setFrozen(const bool & frozen) { _frozen = frozen; }

        /// \return The "weight" of the cell.  For links, this is the output weight; for nodes, this is the input threshold (roughly, the input value required to produce an output close to 1).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setWeight()
        const Value & weight() const { return _weight; }

        /// Set the "weight" of the cell.  For links, this is the output weight; for nodes, this is the input threshold (roughly, the input value required to produce an output close to 1).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::weight()
        void setWeight(const Value & weight) { _weight = weight; }

        /// \return The distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setRun()
        const Value & run() const { return _run; }

        /// Sets the distance in which the node's output sigmoid curve goes from close to 0 to close to 1 (basically the inverse of the slope of the curve).
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::run()
        void setRun(const Value & run) { _run = run; }

        const Step & gap() const { return _gap_peak[0]; }
        void setGap(const Step & gap) { _gap_peak[0] = gap; }

        const Step & peak() const { return _gap_peak[1]; }
        void setPeak(const Step & peak) { _gap_peak[1] = peak; }

        const Step & phase() const { return _phase_step[0]; }
        void setPhase(const Step & phase) { _phase_step[0] = phase; }

        const Step & step() const { return _phase_step[1]; }
        void setStep(const Step & step) { _phase_step[1] = step; }

        /// \return The current output value of the cell.
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::setCurrentValue()
        const Value & outputValue() const { return _output_value; }

        /// Set the current output value of the cell.
        /// \see NeuroCell::NeuroCell()
        /// \see NeuroCell::currentValue()
        void setOutputValue(const Value & v) { _output_value = _running_average = v; }

        /// \return The running average of the cell's output values.
        /// \see NeuroNet::learnTime()
        const Value & runningAverage() const { return _running_average; }

        /// Adds an input to the cell.  A node can have links as input, while links can only have inhibitory links as inputs.
        void addInput(NeuroNet *network, const Index & my_index, const Index & input_index);

        /// Removes an input from the cell.
        void removeInput(NeuroNet *network, const Index & my_index, const Index & input_index);

        /// Update function.
        void update(NEURONET_BASE *neuronet, const Index & index, NeuroCell & next,
                    const QVector<int> & neighbor_indices, const NeuroCell *const neighbors) const;

        /// Write to a data stream.
        void writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const;

        /// Read from a data stream.
        void readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version);

    private:
        KindOfCell _kind;
        bool _frozen;

        union
        {
            Value _weight; ///< Used in links for their weight; in nodes for their input thresholds.
            Step  _gap_peak[2]; ///< Used by oscillators for their gap and peak values.
        };

        union
        {
            Value _run; ///< The width of the slope in the sigmoid curve (for nodes).
            Step  _phase_step[2]; ///< For oscillators, the phase of the oscillator (the delay before it starts), and the current timestep.
        };

        Value _output_value;
        Value _running_average; ///< The running average of output values.
    }; // class NeuroCell

} // namespace NeuroLib

#endif // NEURONODE_H
