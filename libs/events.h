
#ifndef __EVENTS_H__
#define __EVENTS_H__


#ifndef NULL
#define NULL 0
#endif

class EventHandler {
public:
    EventHandler(unsigned long (*handlerFunc)(unsigned long now)) {
        handle = handlerFunc;
        hasNext = false;
    }
    unsigned long (*handle)(unsigned long now); 
    unsigned long next;
    bool hasNext;
    EventHandler *nextHandler;
};


class TimedEventQueue {
public:
    TimedEventQueue(void) {
        firstEventHandler = NULL;
        next_report = 0;
    }

    void addHandler(EventHandler *handler) {
        EventHandler *ne = firstEventHandler;
        if ( ne == NULL) {
            firstEventHandler = handler;
            DEBUG(std::cout << "Added handler as first handler " << std::endl);
        } else {
            for ( EventHandler *e = ne; e != NULL; e = e->nextHandler) {
                DEBUG(std::cout << "next handler" << std::endl);
                ne = e;
            }
            ne->nextHandler = handler;
            DEBUG(std::cout << "Added handler as last handler " << std::endl);
        }
        handler ->nextHandler = NULL;
    }

    void tick(unsigned long now) {
        if ( firstEventHandler == NULL) {
            return; // no event handlers at all.
        }
        // work out which the next event is and call the handler.
        EventHandler *ne = firstEventHandler;
        int n = 0;
        for ( EventHandler *e = ne; e != NULL; e = e->nextHandler) {
            if ( e->next < ne->next) {
                ne = e;
            }
        }
        if ( ne->next < now) {
            TRACE(std::cout << "Calling func for Next Event. " << ne->next << std::endl);
            ne->next = ne->handle(now);
            eventCalled = true;
            nevents++;
        } else if (eventCalled) {
            eventCalled = false;
            tidle += ne->next - now;
        } else {
            TRACE(std::cout << "Next Event is in the future. " << ne->next << ":" << now <<std::endl);
        }
        if ( now > next_report) {
            STATUS(F("Status: Events "));
            STATUS(nevents);
            STATUS(F(" idle:"));
            STATUS(tidle);
            STATUS(F("\n"));
            tidle = 0;
            nevents = 0;
            next_report = now + 10000;
        }
    }

    unsigned long getIdletime() {
        return tidle;
    }
    unsigned long getEventCount() {
        return nevents;
    }

private:
    EventHandler *firstEventHandler;
    int nhandlers;
    bool eventCalled;
    unsigned long nevents;
    unsigned long tidle;
    unsigned long next_report;
};


// generic event listener and eventemitter. Event payload is not defined.
template <class T> class EventListener {
    public:
        EventListener() {
        }
        virtual void onEvent(T obj) {
        }
        int listenerId;
        EventListener<T> *next;
};



template <class T> class EventListenerFunc : public EventListener<T> {
    public:
        EventListenerFunc(void (*handlerFunc)(T obj)) {
            handler = handlerFunc;
        } 
        void onEvent(T obj) {
            handler(obj);
            DEBUG(std::cout << "Done handler " << std::endl);
        }
        void (*handler)(T obj);
};

template <class T> class EventEmitter {
public:

    EventEmitter(void) {
        lastListenerId = 0;
        firstListener = 0;

    }
    void addListener(EventListener<T> *listener) {
        if ( firstListener == NULL) {
            firstListener = listener;
            lastListener = listener;
            listener->next = NULL;
        } else {
            lastListener->next = listener;
            lastListener = listener;
            listener->next = NULL;
        }
        listener->listenerId = lastListenerId++;
    }
    void removeListener(EventListener<T> *listener) {
        if (firstListener == NULL) {
            return;
        }
        EventListener<T> *previousListener = NULL;
        for (EventListener<T> *e = firstListener; e != NULL; e = e->next) {
            if ( e->listenerId == listener->listenerId) {
                if ( previousListener == NULL) {
                    firstListener = e->next;
                } else {
                    previousListener->next = e->next;
                }
                e->next = NULL; // in case its reused, dont want a circular.
                break;
            }
            previousListener = e;
        }
    }
    void emit(T obj) {
        if (firstListener == NULL) {
            return;
        }
        for (EventListener<T> *e = firstListener; e != NULL; e = e->next) {
            e->onEvent(obj);
        }
    }
private:
    int lastListenerId;
    EventListener<T> *firstListener;
    EventListener<T> *lastListener;
};



#endif
