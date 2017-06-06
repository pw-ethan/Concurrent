#include <iostream>
#include <thread>

using namespace std;

void hello() {
    cout << "Hello Concurrent world!" << endl;
}

void do_something(int i) {
    cout << i << endl;
}

class background_task {
public:
    void operator()() const {
        hello();
    }
};

struct func {
    int& i;
    func(int& i_) : i(i_) {}
    void operator() () {
        for (unsigned j = 0; j < 1000; ++j) {
            do_something(i);
            cout << j << endl;
        }
    }
};

class thread_guard {
    thread& t;
public:
    explicit thread_guard(thread& t_) : t(t_) {}
    ~thread_guard() {
        if (t.joinable()) {
            cout << "join in ~thread_guard()" << endl;
            t.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

double mul(double x, double y) {
    if (y == 0) {
        throw y;
    }
    return x / y;
}

void f() {
    int some_local_state = 0;
    func my_func(some_local_state);
    thread t(my_func);
    thread_guard g(t);
    cout << mul(2.1, 0) << endl;
}

int main()
{
    f();
    return 0;
}
