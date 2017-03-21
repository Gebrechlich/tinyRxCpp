#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <sstream>
#include <iterator>
#include "TinyRxCpp.h"

using namespace std;

#define FILE_NAME "test.txt"

int main()
{
    auto ifs = shared_ptr<std::basic_istream<char>>(new std::ifstream(FILE_NAME));

    auto values = Observable<>::from(ifs)
            .concatMap([](const std::string& str) {
        return Observable<std::string>::create([=](const Observable<std::string>::ThisSubscriberPtrType& t) {
            std::istringstream iss(str);
            std::for_each(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), [&](std::string s) {
                s.erase(remove_if(s.begin(), s.end(), [](char c) { return !isalpha(c) && c != '\''; } ), s.end());
                t->onNext(s);
            });
            t->onComplete();
        }).subscribeOn(SchedulersFactory::instance().threadPoolScheduler());

    })
            .map([](const std::string& s) {
        std::string res(s);
        std::transform(res.begin(), res.end(), res.begin(), ::tolower);
        return res;
    })
            .synchronize()
            .toMap([](const std::string& s) {
        return s;
    }, [](const std::string&) {
        return 0;
    }, [](const int&, const int& prev){
        return prev + 1;
    });

    values.subscribe([](const std::map<std::string, int>& str) {
        for(auto a : str)
        {
            std::cout << a.first << " - " << a.second << std::endl;
        }
    },
    [](std::exception_ptr eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const std::exception& e) {cout << e.what();}
    }, []() {
        std::cout << "\n============= complete ================\n";
    });

    std::cin.get();

    return 0;
}

