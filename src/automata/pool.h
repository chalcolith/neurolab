#ifndef POOL_H
#define POOL_H

#include <QVector>
#include <QReadWriteLock>
#include <QWriteLocker>

namespace Automata
{
    
    template <typename T>
    class Pool
    {
        QVector<T*> items;
        QVector<T*> free_items;
        QReadWriteLock lock;
        
    public:
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
        
        class Item
        {
            Pool<T> & pool;
        public:
            T *data;
            
            Item(Pool<T> & pool) : pool(pool), data(pool.getItem()) {}
            ~Item() { pool.returnItem(data); }
        };
        
    protected:
        
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
        
        void returnItem(T *item)
        {
            QWriteLocker l(&lock);
            free_items.append(item);
        }
    };
        
}

#endif // POOL_H
