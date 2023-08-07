/*
 * All rights reserved.
 * Author: Paweł Wódkowski
 */

#ifndef _LPM_H_
#define _LPM_H_

#ifdef __cplusplus
extern "C" {
#endif


int add(unsigned int base, char mask);
int del(unsigned int base, char mask);
char check(unsigned int ip);

#ifdef __cplusplus
}
#endif

#endif
