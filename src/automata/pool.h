#ifndef POOL_H
#define POOL_H

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

#include <QVector>
#include <QStack>
#include <QReadWriteLock>
#include <QWriteLocker>

namespace Automata
{

    /// A simple pool of objects.
    template <typename T>
    class Pool
    {
        QVector<T*> items;
        QStack<T*> free_items;
        QReadWriteLock lock;

    public:
        /// Constructor.
        /// \param initial_capacity The number of objects to initially create.
        Pool(const int initial_capacity = 0)
        {
            for (int i = 0; i < initial_capacity; ++i)
            {
                T *item = new T();
                items.append(item);
                free_items.push(item);
            }
        }

        /// Destructor.
        virtual ~Pool()
        {
            for (int i = 0; i < items.size(); ++i)
                delete items[i];
        }

        /// A convenience class for automatically allocating and releasing an item from the pool in RAII style.
        class Item
        {
            Pool<T> & pool;
        public:
            /// A pointer to the item from the pool.
            T *data;

            /// Constructor.
            /// \param pool The pool from which to get an item.
            Item(Pool<T> & pool) : pool(pool), data(pool.getItem()) {}
            ~Item() { pool.returnItem(data); }
        };

    protected:

        /// Get an item from the pool.
        /// \note The item may not be initialized, if it was used before.
        /// \return An item from the pool.
        T *getItem()
        {
            QWriteLocker l(&lock);

            if (free_items.size() > 0)
            {
                T *item = free_items.pop();
                new (item) T();
                return item;
            }
            else
            {
                T *item = new T();
                items.append(item);
                return item;
            }
        }

        /// Releases a pool item for use by other code.
        void returnItem(T *item)
        {
            QWriteLocker l(&lock);
            item->~T();
            free_items.push(item);
        }
    };

}

#endif // POOL_H
