#include <iostream>
#include "head.h"
#include "config.h"
using namespace std;

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    init();

    int frameID, money;
    while (cin >> frameID >> money)
    {
        setNowFrameId(frameID);
        readAndSetStatus();
        if (frameID % 10 == 0)
            scanCollisionStatus();
        for (int i = 0; i < 4; ++i)
            getNextDes(i);

        cout << frameID << '\n';
        // cerr << frameID << '\n';

#ifdef AVOID_COLLIDE
        avoidCollide();
#endif

        for (int i = 0; i < 4; ++i)
            setInsToDes(i);

        avoidCollideWall();

        // for (auto x : instructions) cout << x << '\n';

        cout << "OK" << endl;
    }
    return 0;
}
