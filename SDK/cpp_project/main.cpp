#include <iostream>
#include "head.h"
using namespace std;

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    init();

    int frameID, money;
    while (cin >> frameID >> money)
    {
        readAndSetStatus();

        for (int i = 0; i < 4; ++i)
            getNextDes(i, frameID);

        cout << frameID << '\n';

        for (int i = 0; i < 4; ++i)
        {
            vector<string> instructions = setInsToDes(i);
            for (auto x : instructions)
                cout << x << '\n';
        }
        
        cout << "OK" << endl;
    }
    return 0;
}
