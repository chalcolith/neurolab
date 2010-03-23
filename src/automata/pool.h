#ifndef POOL_H
#define POOL_H

#include <QVector>
#include <QReadWriteLock>
#include <QWriteLocker>

namespace Automata
{

    /// A simple pool of objects.
    template <typename T>
    class Pool
    {
        QVector<T*> items;
        QVector<T*> free_items;
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
                free_items.append(item);
            }
        }

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

        /// \return An item from the pool.
        T *getItem()
        {
            QWriteLocker l(&lock);

            if (free_items.size() > 0)
            {
                T *item = free_items.last();
                free_items.remove(free_items.size()-1);
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
            free_items.append(item);
        }
    };

}

#endif // POOL_H
