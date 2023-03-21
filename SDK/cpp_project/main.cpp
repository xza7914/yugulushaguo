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
        setNowFrameId(frameID);
        readAndSetStatus();
        if(frameID % COLLISION_DETECTION_FRAMES == 0)
            scanCollisionStatus();
        for (int i = 0; i < 4; ++i)
            getNextDes(i);

        cout << frameID << '\n';

        for (int i = 0; i < 4; ++i)
            setInsToDes(i);

        reduceCollideWall();
        // for (auto x : instructions) cout << x << '\n';

        cout << "OK" << endl;
    }
    return 0;
}
