#ifndef ARDUINO
#define TEST 1
#endif

#include "testmocks.h"

#import "events.h"


#ifdef TEST
unsigned long handlerTest1(unsigned long now) {
    std::cout << "Handler 1 called " << std::endl;
    return now+(rand()%100)+1;
}
unsigned long handlerTest2(unsigned long now) {
    std::cout << "Handler 2 called " << std::endl;
    return now+(rand()%100)+1;
}
unsigned long handlerTest3(unsigned long now) {
    std::cout << "Handler 3 called " << std::endl;
    return now+(rand()%100)+1;
}
EventHandler eventHandler1 = EventHandler(&handlerTest1);
EventHandler eventHandler2 = EventHandler(&handlerTest2);
EventHandler eventHandler3 = EventHandler(&handlerTest3);

bool testTimedEventQueue(void) {
    TimedEventQueue te = TimedEventQueue();
    te.addHandler(&eventHandler1);
    te.addHandler(&eventHandler2);
    te.addHandler(&eventHandler3);
    for (unsigned long i =0; i < 1000; i++) {
        te.tick(i);
    }
    return true;
}

template <class T> class EventEmitterTest : public EventEmitter<T> {
public:
    EventEmitterTest(void) {

    }

};

template <class T> class EventListenerTest1 : public EventListener<T> {
public:
    virtual void onEvent(T message) {
        std::cout << "1Got message:" << message << std::endl;
    }
};
template <class T> class EventListenerTest2 : public EventListener<T> {
public:
    virtual void onEvent(T message) {
        std::cout << "2Got message:" << message << std::endl;
    }
};

void funcEvent(std::string message) {
      std::cout << "Func got message" << message << std::endl;
}

bool testEventEmitter(void) {
    EventEmitterTest<std::string> eventET = EventEmitterTest<std::string>();
    EventListenerTest1<std::string> l1 = EventListenerTest1<std::string>();
    EventListenerTest2<std::string> l2 = EventListenerTest2<std::string>();
    EventListenerFunc<std::string> l3 = EventListenerFunc<std::string>(funcEvent);
    eventET.addListener(&l1);
    eventET.addListener(&l2);
    eventET.emit("testing");
    eventET.removeListener(&l1);
    eventET.emit("testing2 shows on 2 only");
    eventET.removeListener(&l2);
    eventET.emit("testing3 should not show");
    eventET.addListener(&l1);
    eventET.addListener(&l2);
    eventET.emit("testing3 should show pm 1 and 2");
    eventET.removeListener(&l2);
    eventET.emit("testing4 should show on 1");
    eventET.removeListener(&l1);
    eventET.emit("testing5 should not show");
    eventET.addListener(&l3);
    eventET.emit("testing5 should show on func");
    std::cout << " Removing listeners twice " << std::endl;
    eventET.removeListener(&l2);
    std::cout << " Removing listeners twice " << std::endl;
    eventET.removeListener(&l2);
    std::cout << " Done Removing listeners twice " << std::endl;
    eventET.emit("testing6 should not show");
    return 1;
}



int main() {
    if (testTimedEventQueue() &&
        testEventEmitter()) {
        return 0;
    }
    return 1;
}

#endif