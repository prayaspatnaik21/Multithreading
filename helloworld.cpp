#include <iostream>
#include <thread>

using namespace std;
// /opt/homebrew/bin/g++-14 -std=c++20 Algorithms_klaus.cpp
void helloWorld()
{
    cout << "Hello World" << endl;
}

int main()
{
    std::thread t(helloWorld);

    if(t.joinable()) t.join();

}