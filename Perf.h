#ifndef __PERF_H__
#define __PERF_H__

unsigned long NumCreated;   // number of foreground threads created
unsigned long PIDWork;      // current number of PID calculations finished
unsigned long FilterWork;   // number of digital filter calculations finished
unsigned long NumSamples;   // incremented every ADC sample, in Producer
unsigned long DataLost;     // data sent by Producer, but not received by Consumer
long MaxJitter;             // largest time jitter between interrupts in usec
#define JITTERSIZE 64
unsigned long const JitterSize=JITTERSIZE;
long x[64],y[64];         // input and output arrays for FFT
long jitter;                    // time between measured and expected, in us

#endif /*#ifndef __PERF_H__*/