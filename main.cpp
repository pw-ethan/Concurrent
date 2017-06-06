#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

using namespace std;

void hello() {
    cout << "Hello Concurrent world!" << endl;
}

void do_something(int i) {
    cout << i << endl;
}

void do_something_in_current_thread() {
    cout << "do something in current thread" << endl;
    throw;
}

class background_task {
public:
    void operator()() const {
        hello();
    }
};

struct func {
    int& i;
    func(int& i_) : i(i_) { cout << "func()" << endl; }
    ~func() { cout << "~func()" << endl; }
    void operator() () {
        for (unsigned j = 0; j < 10; ++j) {
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
        cout << "~thread_guard()" << endl;
        if (t.joinable()) {
            cout << "join in ~thread_guard()" << endl;
            t.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

void f() {
    int some_local_state = 0;
    func my_func(some_local_state);
    thread t(my_func);
    thread_guard g(t);
    do_something_in_current_thread();
}

void f1(int i, const string s) {
    cout << "h" << endl;
    cout << i << " " << s << endl;
}

void not_oops(int some_param) {
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    thread t(f1, 3, string(buffer));
    t.detach();
}

class widget_data {
public:
    int data;
    widget_data(int d = 0) : data(d) { }
    widget_data(const widget_data& rhs) {
        data = rhs.data;
        cout << "copy constructor" << endl;
    }
    widget_data& operator=(const widget_data& rhs) {
        if (this != &rhs) {
            data = rhs.data;
        }
        cout << "operator=" << endl;
        return *this;
    }
    ~widget_data() {
        cout << "~widget_data()" << endl;
    }
    void operator() () {
        cout << "operator()" << data << endl;
    }
    void show(int n) {
        cout << "show() " << data << " " << n << endl;
    }
};

void update_data_for_widget(widget_data& w) {
    cout << w.data << endl;
    w.data = 1;
}

void oops_again() {
    widget_data w(0);
    thread t(update_data_for_widget, ref(w));
    cout << w.data << endl;
    t.join();
    cout << w.data << endl;
}

void oops() {
    widget_data w(2);
    int num(3);
    thread t(&widget_data::show, &w, num);
    t.join();
}

void process_widget(unique_ptr<widget_data> p) {
    p->data = 12;
    p->show(1);
}

void func_ptra() {
    unique_ptr<widget_data> p(new widget_data);
    p->data = 10;
    thread t(process_widget, move(p));
    t.join();
}

int main()
{
    func_ptra();
    return 0;
}
