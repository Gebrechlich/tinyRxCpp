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

template<typename T>
class WordSplitOperator : public Operator<T,T>
{
    using SourceSubscriberType = std::shared_ptr<Subscriber<T>>;
    using ThisSubscriberType = typename CompositeSubscriber<T,T>::ChildSubscriberType;

    struct WordSplitSubscriber : public CompositeSubscriber<T,T>
    {
        WordSplitSubscriber(ThisSubscriberType p) :
            CompositeSubscriber<T,T>(p)
        {}

        void onNext(const T& t) override
        {
            std::string word;
            char prev = ' ';
            for(auto c: t)
            {
                if(isalpha(c) || (prev != ' ' && c == '\''))
                {
                    word.push_back(std::tolower(c));
                }
                else if(c == ' ')
                {
                    this->child->onNext(word);
                    word.clear();
                }
                prev = c;
            }
        }
    };

public:
    WordSplitOperator() : Operator<T,T>()
    {}

    SourceSubscriberType operator()(const ThisSubscriberType& t) override
    {
        auto subs = std::make_shared<WordSplitSubscriber>(t);
        subs->addChildSubscriptionFromThis();
        return subs;
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
    });

    words.lift(std::unique_ptr<Operator<std::string, std::string>>(make_unique<WordSplitOperator<std::string>>()))
     .toMap([](const std::string& s){
        return s;
    }, [](const std::string&){
        return 0;
    }, [](const int&, const int& prev){
        return prev + 1;
    })
    .subscribe([](const std::map<std::string, int>& str) {
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
        std::cout << "complete\n";
    });
    return 0;
}

