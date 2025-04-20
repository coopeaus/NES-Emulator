#pragma once
#include "global-types.h"

union r4000_4004 {
  struct {
    u8 envelopePeriod : 4;
    u8 constantVolume : 1;
    u8 lengthCounterHalt : 1;
    u8 pulseDutyCycle : 2;
  } bit;
  u8 value = 0x00;
};

union r4001_4005 {
  struct {
    u8 sweepShiftCount : 3;
    u8 sweepNegate : 1;
    u8 sweepDividerPeriod : 3;
    u8 sweepEnable : 1;
  } bit;
  u8 value = 0x00;
};

union r4002_4006 {
  struct {
    u8 timerLow : 8;
  } bit;
  u8 value = 0x00;
};

union r4003_4007 {
  struct {
    u8 timerHigh : 3;
    u8 lengthCounterLoad : 5;
  } bit;
  u8 value = 0x00;
};

union r4008 {
  struct {
    u8 linearCounterReload : 7;
    u8 linearCounterControl : 1;
  } bit;
  u8 value = 0x00;
};

union r400A {
  struct {
    u8 timerLow : 8;
  } bit;
  u8 value = 0x00;
};

union r400B {
  struct {
    u8 timerHigh : 3;
    u8 lengthCounterLoad : 5;
  } bit;
  u8 value = 0x00;
};

union r400C {
  struct {
    u8 envelopePeriod : 4;
    u8 constantVolume : 1;
    u8 loopEnvelope : 1;
    u8 unused : 2;
  } bit;
  u8 value = 0x00;
};

union r400E {
  struct {
    u8 noisePeriod : 4;
    u8 unused : 3;
    u8 loopNoise : 1;
  } bit;
  u8 value = 0x00;
};

union r400F {
  struct {
    u8 unused : 3;
    u8 lengthCounterLoad : 5;
  } bit;
  u8 value = 0x00;
};

union r4010 {
  struct {
    u8 frequencyIndex : 4;
    u8 unused : 2;
    u8 loopSample : 1;
    u8 irqEnable : 1;
  } bit;
  u8 value = 0x00;
};

union r4011 {
  struct {
    u8 directLoad : 7;
    u8 unused : 1;
  } bit;
  u8 value = 0x00;
};

union r4012 {
  struct {
    u8 sampleAddress : 8;
  } bit;
  u8 value = 0x00;
};

union r4013 {
  struct {
    u8 sampleLength : 8;
  } bit;
  u8 value = 0x00;
};

// TODO: 4015, 4017

struct PulseControl {
  // $4000/$4004
  u8 envelopePeriod : 4;
  u8 constantVolume : 1;
  u8 lengthCounterHalt : 1;
  u8 pulseDutyCycle : 2;
  // $4001/$4005
  u8 sweepShiftCount : 3;
  u8 sweepNegate : 1;
  u8 sweepDividerPeriod : 3;
  u8 sweepEnable : 1;
  // $4002/$4006
  u8 timerLow : 8;
  // $4003/$4007
  u8 timerHigh : 3;
  u8 lengthCounterLoad : 5;
};

struct TriangleControl {
  // $4008
  u8 linearCounterReload : 7;
  u8 linearCounterControl : 1;
  // $400A
  u8 timerLow : 8;
  // $400B
  u8 timerHigh : 3;
  u8 lengthCounterLoad : 5;
};

struct NoiseControl {
  // $400C
  u8 envelopePeriod : 4;
  u8 constantVolume : 1;
  u8 loopEnvelope : 1;
  u8 unused : 2;
  // $400E
  u8 noisePeriod : 4;
  u8 loopNoise : 1;
  // $400F
  u8 lengthCounterLoad : 5;
};

struct DmcControl {
  // $4010
  u8 frequencyIndex : 4;
  u8 loopSample : 1;
  u8 irqEnable : 1;
  // $4011
  u8 directLoad : 7;
  // $4012
  u8 sampleAddress : 8;
  // $4013
  u8 sampleLength : 8;
};
