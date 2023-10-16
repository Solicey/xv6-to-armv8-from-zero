#include "proc.h"
#include "types.h"
#include "defs.h"
#include "arm.h"

struct cpu* mycpu()
{
    int id = cpuid();
    struct cpu* c = &cpus[id];
    return c;
}