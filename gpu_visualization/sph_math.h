#ifndef __SPHMATH_H__
#define __SPHMATH_H__

uint iDivUp(uint a, uint b);

uint iDivUp(uint a, uint b)
{
    return (a % b != 0) ? (a / b + 1) : (a / b);
}


#endif
