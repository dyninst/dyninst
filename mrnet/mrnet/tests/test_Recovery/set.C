#include <set>
#include <iostream>

using namespace std;

int main()
{
    std::set<unsigned int> s;
    std::set<unsigned int>::const_iterator iter;

    s.insert( 3 );
    s.insert( 3 );
    s.insert( 4 );
    s.insert( 237 );
    s.insert( 8 );
    s.insert( 9 );
    s.insert( 111 );

    for( iter=s.begin(); iter!=s.end(); iter++ ){
        cout << *iter << endl;
    }

    return 0;
}
