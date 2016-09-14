#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "Observable.hpp"
#include "SchedulersFactory.hpp"
#include <gtest/gtest.h>
#include "Util.hpp"

using namespace std;

struct some_exception {
    explicit some_exception(int x): v(x) {
    }

    some_exception(const some_exception & e): v(e.v) {
    }

    int v;
};

TEST(RxCppTest, Subscribe)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(10);
        t->onComplete();
    });
    int res;
    values.subscribe([&](const int& i){res = i;});

    ASSERT_EQ(10, res);
}

TEST(RxCppTest, Just)
{
    std::vector<int> res;
    Observable<int>::just({1,2,3}).subscribe(
                [&](const int& i){res.push_back(i);});

    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(2, res[1]);
    ASSERT_EQ(3, res[2]);
}


TEST(RxCppTest, Filter)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onNext(4);
        t->onComplete();
    });
    int res = 0;
    values.filter([&](const int& i){return i == 3;}
    ).subscribe([&](const int& i){res = i;});

    ASSERT_EQ(3, res);
}

TEST(RxCppTest, Take)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onNext(4);
        t->onComplete();
    });
    std::vector<int> res;
    values.take(2).subscribe([&](const int& i){res.push_back(i);});

    ASSERT_EQ(2, res.size());
    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(2, res[1]);
}


TEST(RxCppTest, Map)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(100);
        t->onComplete();
    });

    std::string str;

    values.map([&](const int& i){
        std::ostringstream os;
        os << i;
        return os.str();
    }).subscribe([&](const string& s){
        str = "str" + s;});

    ASSERT_EQ("str100", str);

    auto f = [&](const int& i){
        std::ostringstream os;
        os << i;
        return os.str();
    };

    values.map(std::move(f)).subscribe([&](const string& s){
        str = "str" + s;});

    ASSERT_EQ("str100", str);
}

TEST(RxCppTest, Distinct)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(2);
        t->onNext(3);
        t->onComplete();
    });
    std::vector<int> res;
    values.distinct(
                ).subscribe([&](const int& i){res.push_back(i);});

    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(2, res[1]);
    ASSERT_EQ(3, res[2]);
}

TEST(RxCppTest, All)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(2);
        t->onNext(2);
        t->onNext(6);
        t->onNext(2);
        t->onNext(4);
        t->onComplete();
    });

    bool res;

    values.all([](const int& i){ return i%2 == 0;}
    ).subscribe([&](bool b){res = b;});
    ASSERT_TRUE(res);

    auto values2 = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(2);
        t->onNext(2);
        t->onNext(3);
        t->onNext(2);
        t->onNext(4);
        t->onComplete();
    });

    values2.all([](const int& i){ return i%2 == 0;}
    ).subscribe([&](bool b){res = b;});

    ASSERT_FALSE(res);
}


TEST(RxCppTest, DoOn)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onNext(4);
        t->onComplete();
    });

    int res = 0;
    bool complete = false;

    values.doOnEach([&res](const int& i){res += i;}, [](std::exception_ptr){}
    ,[&complete](){complete = true;}).subscribe([&](const int&){});

    ASSERT_TRUE(complete);
    ASSERT_EQ(10, res);

    res = 0;

    values.doOnNext([&res](const int& i){res += i;}).subscribe([&](const int&){});

    ASSERT_EQ(10, res);

    bool error = false;

    auto errorValue = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onError(std::make_exception_ptr(some_exception(1)));
        t->onComplete();
    });

    errorValue.doOnError([&error](std::exception_ptr ex){
        try
        {
            std::rethrow_exception(ex);
        }catch(...)
        {
            error = true;
        }
    }).subscribe([&](const int&){});

    ASSERT_TRUE(error);
}

TEST(RxCppTest, Exist)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(2);
        t->onNext(2);
        t->onNext(6);
        t->onNext(2);
        t->onNext(4);
        t->onComplete();
    });

    bool res;

    values.exist([](const int& i){ return i%2 == 0;}
    ).subscribe([&](bool b){res = b;});
    ASSERT_TRUE(res);

    auto values2 = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(3);
        t->onNext(1);
        t->onNext(7);
        t->onNext(5);
        t->onComplete();
    });

    values2.exist([](const int& i){ return i%2 == 0;}
    ).subscribe([&](bool b){res = b;});
    ASSERT_FALSE(res);
}

TEST(RxCppTest, Scan)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onComplete();
    });

    std::vector<int> res;
    values.scan([](int i, int j){
        return i+j;}
    ).subscribe([&](const int& i){
        res.push_back(i);});

    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(3, res[1]);
    ASSERT_EQ(6, res[2]);
}

TEST(RxCppTest, ScanWithSeed)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onComplete();
    });

    std::vector<int> res;

    values.scan([](int i, int j){
        return i+j;}, 10).subscribe([&](const int& i){
        res.push_back(i);});

    ASSERT_EQ(11, res[0]);
    ASSERT_EQ(13, res[1]);
    ASSERT_EQ(16, res[2]);
}

TEST(RxCppTest, Last)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onComplete();
    });
    int res = 0;
    values.last().subscribe([&](const int& i){res = i;});
    ASSERT_EQ(3, res);
}

struct ReduceFunction
{
    int operator()(const int& t0, const int& t1)
    {
        return t0 + t1;
    }
};

TEST(RxCppTest, Reduce)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onNext(2);
        t->onNext(3);
        t->onComplete();
    });

    int res = 0;
    values.reduce([](const int& i, const int& j){
        return i+j;}
    ).subscribe([&](const int& i){
        res = i;});

    ASSERT_EQ(6, res);

    values.reduce(ReduceFunction()).subscribe([&](const int& i){
        res = i;});

    ASSERT_EQ(6, res);
}

int inc()
{
    static int i = 0;
    return ++i;
}

TEST(RxCppTest, Defer)
{
    auto values = Observable<int>::defer([](){
        return Observable<int>::just(inc());
    });

    int res = 0;

    values.subscribe([&](const int& i){
        res = i;});
    ASSERT_EQ(1, res);
    values.subscribe([&](const int& i){
        res = i;});
    ASSERT_EQ(2, res);
    values.subscribe([&](const int& i){
        res = i;});
    ASSERT_EQ(3, res);
}

TEST(RxCppTest, ToMapKSelector)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onComplete();
    });

    int v1 = 0;

    values.toMap([](const int& i){
        return i * 10;
    }).subscribe([&](const std::map<int, int>& m){
        auto it = m.find(10);
        if(it != m.end())
        {
            v1 = (*it).second;
        }
    });

    ASSERT_EQ(1, v1);
}


TEST(RxCppTest, ToMapKVSelector)
{
    auto values = Observable<std::string>::
            create([](const Observable<std::string>::ThisSubscriberPtrType& t)
    {
        t->onNext("aaaaa");
        t->onComplete();
    });

    size_t v1 = 0;

    values.toMap([](const std::string& s){
        return s.at(0);
    }, [](const std::string& s){
        return s.length();
    }).subscribe([&](const std::map<char, size_t>& m){
        auto it = m.find('a');
        if(it != m.end())
        {
            v1 = (*it).second;
        }
    });

    ASSERT_EQ(5, v1);
}

TEST(RxCppTest, ToMapKVPSelector)
{
    auto values = Observable<int>::
            create([](const Observable<int>::ThisSubscriberPtrType& t)
    {
        t->onNext(1);
        t->onComplete();
    });

    int v1 = 0;

    values.toMap([](const int& i){
        return i * 10;
    }, [](const int& i){
        return i;
    },[](const int& prev, const int&){
        return prev + 10;
    }).subscribe([&](const std::map<int, int>& m){
        auto it = m.find(10);
        if(it != m.end())
        {
            v1 = (*it).second;
        }
    });

    ASSERT_EQ(11, v1);
}

TEST(RxCppTest, ConcatMap)
{
    auto mapper = [](const int& i){
        return Observable<int>::just({1*i,2*i});
    };

    auto values = Observable<int>::just({100,1000});

    std::vector<int> result;

    values.concatMap(std::move(mapper))
    .subscribe([&](const int& i){
        result.push_back(i);
    });

    ASSERT_EQ(100, result[0]);
    ASSERT_EQ(200, result[1]);
    ASSERT_EQ(1000, result[2]);
    ASSERT_EQ(2000, result[3]);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
