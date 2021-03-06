#include <iostream>
#include <iomanip>
#include "TinyRxCpp.h"

using namespace std;

int main()
{
    auto values = Observable<>::range(1,100).concatMap([](const int& a) {
        return Observable<>::range(1, a).concatMap([=](const int& b){
            return Observable<>::range(b, a).filter([=](const int& c){
                return b*b + c*c == a*a;
            }).map([=](const int& d){
                return std::make_tuple(b,d,a);
            });
        });
    });

    values.subscribe([](const std::tuple<int,int,int>& t){
        cout << setw(2) << std::get<0>(t) << "  "<<
                setw(2) << std::get<1>(t)<<"  "<<
                setw(2) << std::get<2>(t) << std::endl;
    });
    return 0;
}
