#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <list>

#include "threadsafe_stack.h"
#include "hierarchical_mutex.h"
#include "threadsafe_queue.h"
#include "quick_sort.h"

using namespace std;

// Hello Concurrent World
void hello() {
    cout << "Hello Concurrent world!" << endl;
}

// thread function
void do_something(int i) {
    cout << i << endl;
}

// current thread throw exception
void do_something_in_current_thread() {
    cout << "do something in current thread" << endl;
    throw;
}

// callable type -- with operator()
// thread my_thread(background_task()); Error!
// thread my_thread((background_task())); OK!
// thread my_thread{background_task()}; OK!
class background_task {
public:
    void operator()() const {
        hello();
    }
};

// reference to local variable, may be accessed after being destroyed
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

// RAII
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

// Transfer parameter to thread
void f1(int i, const string& s) {
    cout << i << " " << s << endl;
}

void not_oops(int some_param) {
//    thread t(f1, 3, "hello");
//    t.join();
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

// transfer reference of object
void oops_again() {
    widget_data w(0);
    thread t(update_data_for_widget, ref(w));
    cout << w.data << endl;
    t.join();
    cout << w.data << endl;
}

// use member function as thread function -- first argument
// a pointer of object -- second argument, member function acting on it
// and from the 3rd argument as the member function's arguments
void oops() {
    widget_data w(2);
    int num(3);
    thread t(&widget_data::show, &w, num);
    t.join();
}

// move
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

void some_function() {
    cout << "some function" << endl;
}

// return thread
thread ff() {
    return thread(some_function);
}

void some_other_function() {
    cout << "some other function" << endl;
}

// return thread
thread gg() {
    thread t(some_other_function);
    return t;
}

// use thread as parameter
void fff(thread t) {
    t.join();
}

void ggg() {
    fff(thread(some_function));
    thread t(some_other_function);
    fff(move(t));
}

//
class scoped_thread {
    thread t;
public:
    explicit scoped_thread(thread t_) : t(move(t_)) {
        cout << "scoped_thread()" << endl;
        if(!t.joinable()) {
            throw logic_error("No thread");
        }
    }
    ~scoped_thread() {
        cout << "~scoped_thread()" << endl;
        t.join();
    }
    scoped_thread(const scoped_thread&) = delete;
    scoped_thread& operator=(const scoped_thread&) = delete;
};

void ffff() {
    int some_local_state = 1;
    func my_func(some_local_state);
    thread my_thread(my_func);
    scoped_thread t(move(my_thread));
    do_something_in_current_thread();
}

// mass production thread
void do_work(unsigned id) {
    cout << "do work " << id << endl;
}

void fffff() {
    vector<thread> threads;
    for(unsigned i = 0; i < 20; ++i) {
        threads.push_back(thread(do_work, i));
    }
    for_each(threads.begin(), threads.end(), mem_fn(&thread::join));
}

// hierarchical mutex
hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

int do_low_level_stuff() {
    return 1;
}

int low_level_func() {
    lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}

void high_level_stuff(int some_param) {
    cout << some_param << endl;
}

void high_level_func() {
    lock_guard<hierarchical_mutex> lk(high_level_mutex);
    high_level_stuff(low_level_func());
}

void thread_a() {
    high_level_func();
}

hierarchical_mutex other_mutex(100);
void do_other_stuff() {

}

void other_stuff() {
    high_level_func();
    do_other_stuff();
}

void thread_b() {
    lock_guard<hierarchical_mutex> lk(other_mutex);
    other_stuff();
}

threadsafe_queue<int> data_queue;

int data = 10;

bool more_data_to_prepare() {
    return data > 0;
}
int prepare_data() {
    return data--;
}
void data_preparation_thread() {
    while(more_data_to_prepare()) {
        const int data = prepare_data();
        data_queue.push(data);
    }
}

void process(int data) {
    cout << "process" << data << endl;
}
bool is_last() {
    return data <= 1 && data_queue.empty();
}
void data_processing_thread() {
    while(true) {
        int data;
        data_queue.wait_and_pop(data);
        process(data);
        if(is_last()) {
            break;
        }
    }
}

int main()
{
    int myints[] = {1, 4, 2, 5, 7, 0};
    list<int> mlist(begin(myints), end(myints));

    for(auto i : mlist) {
        cout << i << " ";
    }
    cout << endl;
    list<int> ret = parallel_quick_sort(mlist);
    for(auto i : ret) {
        cout << i << " ";
    }
    cout << endl;

//    thread ta(data_preparation_thread);
//    thread tb(data_processing_thread);
//    thread tc(data_processing_thread);
//    ta.join();
//    tb.join();
//    tc.join();
    return 0;
}
