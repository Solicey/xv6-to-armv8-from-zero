#include "kernel/types.h"
#include "user.h"

//
// wrapper so that it's OK if main() does not call exit().
//
void _main()
{
    extern int main();
    main();
    // TODO: exit
    while (1);
}
