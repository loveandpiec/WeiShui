#pragma once

namespace modern_framework
{
    template<class T>
    class linked_list
    {
    public:
        static T* header;
        T* next;
        linked_list()
        {
            next=nullptr;
            if(header)
            {
                T* cur = header;
                while(cur->next)
                {
                    cur=cur->next;
                }
                cur->next=static_cast<T*>(this);
            }
            else
                header=static_cast<T*>(this);
        }
        ~linked_list()
        {
            if(header==static_cast<T*>(this))
            {
                header=header->next;
                return;
            }
            if(header)
            {
                T* cur = header;
                T* last =header;
                while(cur->next)
                {
                    last=cur;
                    cur=cur->next;
                    if(last==static_cast<T*>(this))
                    {
                        last->next=cur;
                    }
                }
            }
        }

    };

    template<class T>
    T* linked_list<T>::header=nullptr;

}