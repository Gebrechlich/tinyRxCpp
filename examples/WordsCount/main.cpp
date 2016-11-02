#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include "TinyRxCpp.h"

using namespace std;

#define FILE_NAME "test.txt"

struct NoFileException : public std::exception
{
    virtual const char* what() const throw()
    {
        return "No such file\n";
    }

};

int main()
{
    auto words = Observable<std::string>::create([](const Observable<std::string>::ThisSubscriberPtrType& t) {
        std::ifstream in (FILE_NAME);
        try
        {
            if(in.is_open())
            {
                std::string str;
                while(std::getline(in, str))
                {
                    t->onNext(str);
                }
                t->onComplete();
            }
            else
            {
                throw NoFileException();
            }
        }
        catch(std::exception& e)
        {
            t->onError(std::current_exception());
        }
    })
     .concatMap([](const std::string& str){
        return Observable<std::string>::create([=](const Observable<std::string>::ThisSubscriberPtrType& t){
            std::string word;
            char prev = ' ';
            for(auto c : str)
            {
                if(isalpha(c) || (prev != ' ' && c == '\''))
                {
                    word.push_back(std::tolower(c));
                }
                else if(c == ' ')
                {
                    t->onNext(word);
                    word.clear();
                }
                prev = c;
            }

            t->onNext(word);
            t->onComplete();
        });
    })
     .toMap([](const std::string& s){
        return s;
    }, [](const std::string&){
        return 0;
    }, [](const int&, const int& prev){
        return prev + 1;
    });

    words.subscribe([](const std::map<std::string, int>& str) {
        for(auto a : str)
        {
            std::cout << a.first << " - " << a.second << std::endl;
        }
    },
    [](std::exception_ptr eptr){
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const std::exception& e) {cout << e.what();}
    }, [](){
        std::cout << "\n============= complete ================\n";
    });
    return 0;
}

