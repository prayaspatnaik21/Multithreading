/*

Managing Threads
----------------

2.1) Basic Thread Managment

    1. Every C++ program has at least one thread , which is started by the c++ runtime , the thread running main().
    2. When we create a thread object , we launch a thread of execution.
    3. If you want to wait for a thread to complete , call join()
    4. join() is a simple and brute force technique.
    5. The act of joining also cleans up any storage associated with the thread.
    6. you can call join() only once for a given thread.Once you call join on a thread , it
       is no longer joinable.
    7. If you are detaching a thread , you can call detach() immediately after the thread has been started.
    8. Carefully pick the place in the code where you call join(). This means that the call to join() is liable to 
       be skipped if an exception is thrown after the thread has been started but before the call to join().
    9. Calling detach() on a std::thread object leaves the thread to run in the background, with no direct means of communicating with it.
    10. It isn't possible to obtain a std::thread object that references it.Truly runs in the background.
    11. Ownership and control are passed over to the c++ runtime whcih ensures that the resources associated with the thread are correctly reclaimed
        with the thread exits.
    12. Detached threads often called daemon threads.
    13. In order to detach the thread from a std::thread object, there must be a thread to detach. You can't call detach() on a std::thread object 
        with no associated thread of execution.

2.2) Passing arguments to a thread function

    1. By default , The arguments are copied into internal storage, where they can be accessed by the newly created thread of execution and then passed
        to the callable object or function as rvalues as if they are temporaries.
    2. This is done even if the corresponding parameter in the function is expecting a reference.
    3. If the thread function wants a reference to the datastructure , pass using std::ref(data)
    4. We can pass a member function pointer as the funciton , provided we supply a suitable object pointer as the first argument.
        
        class X
        {
            public:
                void do_lengthy_work();
        };
        X my_x;
        std::thread t(&X::do_lengthy_work,&my_x);

    5. Only one std:unique_pointer instance can point to a given object at a time.
    6. The move constructor and move assignmet operator allow the ownership of an object to be transferred around std::unique_ptr instances.

2.3) Transferring ownership of a thread

    1.  ownership of a particular thread of execution can be moved between std::thread instances
    2.  what is mem_fn
        void f() {
        std::vector<std::thread> threads;
        for(unsigned i=0;i<20;++i)
        {
            threads.push_back(std::thread(do_work,i));  -> Spawn threads
        }
        std::for_each(threads.begin(),threads.end(), -> Call join() on each thread in turn
                  std::mem_fn(&std::thread::join));
    }

2.4) Choosing the number of threads at runtime

    1. One feature of the C++ Standard Library that helps here is std::thread::hardware_ concurrency().
    2. This function returns an indication of the number of threads that can truly run concurrently for a given execution of a program. 

2.5) Identifying threads

    1. Thread identifies are of type std::thread::id
    2. First, the identifier for a thread can be obtained from its associated std::thread object by calling the get_id() member function.
    3. The identifier for the current thread can be obtained by calling std::this_thread:: get_id(), which is also defined in the <thread> header.
    4. Objects of type std::thread::id can be freely copied and compared;
    5. If two objects of type std::thread::id are equal, they represent the same thread, or both are holding the “not any thread” value. 
       If two objects aren’t equal, they represent different threads, or one represents a thread and the other is holding the “not any thread” value.


Listings

// /opt/homebrew/bin/g++-14 -std=c++20 Algorithms_klaus.cpp
*/
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
class scoped_thread
{
    std::thread t;
    public:
        explicit scoped_thread(std::thread t_):
            t(std::move(t_))
        {
            if(!t.joinable())
            {
                throw std::logic_error("No thread");
            }
        }

        ~scoped_thread()
        {
            t.join();
        }

        scoped_thread(scoped_thread const&)=delete;
        scoped_thread& operator=(scoped_thread const&) = delete;
};
class thread_guard
{
    std::thread& t;

    public:
        explicit thread_guard(std::thread& t_):
            t(t_)
        {}
        ~thread_guard()
        {
            if(t.joinable())
            {
                std::cout << "Joining Thread" << std::endl;
                t.join();
            }
        }
        thread_guard(thread_guard const&) = delete;
        thread_guard& operator=(thread_guard const&)=delete;
};

void do_something(int& i)
{
    ++i;
}

struct func
{
    int& i;

    func(int& i_)
    :i(i_)
    {}

    void operator()()
    {
        for(auto j = 0 ; j < 1000 ; ++j)
        {
            do_something(i);
        }
        std::cout << i << std::endl;
    }
};

void do_something_in_current_thread()
{}

void f()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    {
        std::thread t(my_func);
        thread_guard g(t);
    }

    /*
    If I remove the scope above and print out the some_local_state variable , it will print out zero , 
    because the changes to some_local_state made in the thread are not guaranteed to be visible in the main
    thread when cout is executed in the f() function.

    Since the main thread continues execution without waiting for the thread t to finish, it print the current
    value of some_local_state (which is still 0) at that point , before the other thread has a chance to increment it.
    */
    do_something_in_current_thread();
    std::cout << some_local_state << std::endl;
}

std::mutex mtx;

void do_work(unsigned int id, int& shared_state)
{
    std::lock_guard<std::mutex> lock(mtx);
    ++shared_state;
}

void work()
{
    std::vector<std::thread> threads;
    int shared_state = 0;

    for(unsigned i = 0 ; i < 20 ; i ++)
    {
        threads.push_back(std::thread(do_work ,i, std::ref(shared_state)));
    }

    std::for_each(threads.begin() , threads.end() , std::mem_fn(&std::thread::join));
    std::cout << shared_state << std::endl;
}
int main()
{
    f();
    work();
}