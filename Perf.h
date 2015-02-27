#ifndef __PERF_H__
#define __PERF_H__
#include <stdint.h>
extern unsigned long NumCreated;   // number of foreground threads created
extern unsigned long PIDWork;      // current number of PID calculations finished
extern unsigned long FilterWork;   // number of digital filter calculations finished
extern unsigned long NumSamples;   // incremented every ADC sample, in Producer
extern unsigned long DataLost;     // data sent by Producer, but not received by Consumer
extern long MaxJitter;             // largest time jitter between interrupts in usec
#define JITTERSIZE 64
unsigned long const JitterSize=JITTERSIZE;
extern long x[64],y[64];         // input and output arrays for FFT
extern long jitter;                    // time between measured and expected, in us

#endif /*#ifndef __PERF_H__*/
