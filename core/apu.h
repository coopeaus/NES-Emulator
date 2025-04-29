// NOTE: This APU is WIP, the APU that's being used is Blargg's, located in the lib directory
#pragma once
#include "global-types.h"
#include "apu-types.h"

class Bus;
class APU
{
public:
  APU( Bus *bus );
  Bus *bus;

  template <class Archive> void serialize( Archive & /*ar*/ ) {} // NOLINT

  /*
  ################################
  ||        APU Variables       ||
  ################################
  */

  // Pulse 1
  r4000_4004 r4000{};
  r4001_4005 r4001{};
  r4002_4006 r4002{};
  r4003_4007 r4003{};

  // Pusle 2
  r4000_4004 r4004{};
  r4001_4005 r4005{};
  r4002_4006 r4006{};
  r4003_4007 r4007{};

  // Triangle
  r4008 r4008{};
  r400A r400A{};
  r400B r400B{};

  // Noise
  r400C r400C{};
  r400E r400E{};
  r400F r400F{};

  // DMC
  r4010 r4010{};
  r4011 r4011{};
  r4012 r4012{};
  r4013 r4013{};

  PulseControl GetPulse1Control() const
  {
    PulseControl pulse1{};
    pulse1.envelopePeriod = r4000.bit.envelopePeriod;
    pulse1.constantVolume = r4000.bit.constantVolume;
    pulse1.lengthCounterHalt = r4000.bit.lengthCounterHalt;
    pulse1.pulseDutyCycle = r4000.bit.pulseDutyCycle;
    pulse1.sweepShiftCount = r4001.bit.sweepShiftCount;
    pulse1.sweepNegate = r4001.bit.sweepNegate;
    pulse1.sweepDividerPeriod = r4001.bit.sweepDividerPeriod;
    pulse1.sweepEnable = r4001.bit.sweepEnable;
    pulse1.timerLow = r4002.bit.timerLow;
    pulse1.timerHigh = r4003.bit.timerHigh;
    pulse1.lengthCounterLoad = r4003.bit.lengthCounterLoad;
    return pulse1;
  }

  PulseControl GetPulse2Control() const
  {
    PulseControl pulse2{};
    pulse2.envelopePeriod = r4004.bit.envelopePeriod;
    pulse2.constantVolume = r4004.bit.constantVolume;
    pulse2.lengthCounterHalt = r4004.bit.lengthCounterHalt;
    pulse2.pulseDutyCycle = r4004.bit.pulseDutyCycle;
    pulse2.sweepShiftCount = r4005.bit.sweepShiftCount;
    pulse2.sweepNegate = r4005.bit.sweepNegate;
    pulse2.sweepDividerPeriod = r4005.bit.sweepDividerPeriod;
    pulse2.sweepEnable = r4005.bit.sweepEnable;
    pulse2.timerLow = r4006.bit.timerLow;
    pulse2.timerHigh = r4007.bit.timerHigh;
    pulse2.lengthCounterLoad = r4007.bit.lengthCounterLoad;
    return pulse2;
  }

  TriangleControl GetTriangleControl() const
  {
    TriangleControl triangle{};
    triangle.linearCounterReload = r4008.bit.linearCounterReload;
    triangle.linearCounterControl = r4008.bit.linearCounterControl;
    triangle.timerLow = r400A.bit.timerLow;
    triangle.timerHigh = r400B.bit.timerHigh;
    triangle.lengthCounterLoad = r400B.bit.lengthCounterLoad;
    return triangle;
  }

  NoiseControl GetNoiseControl() const
  {
    NoiseControl noise{};
    noise.envelopePeriod = r400C.bit.envelopePeriod;
    noise.constantVolume = r400C.bit.constantVolume;
    noise.loopEnvelope = r400C.bit.loopEnvelope;
    noise.noisePeriod = r400E.bit.noisePeriod;
    noise.loopNoise = r400E.bit.loopNoise;
    noise.lengthCounterLoad = r400F.bit.lengthCounterLoad;
    return noise;
  }

  DmcControl GetDmcControl() const
  {
    DmcControl dmc{};
    dmc.frequencyIndex = r4010.bit.frequencyIndex;
    dmc.loopSample = r4010.bit.loopSample;
    dmc.irqEnable = r4010.bit.irqEnable;
    dmc.directLoad = r4011.bit.directLoad;
    dmc.sampleAddress = r4012.bit.sampleAddress;
    dmc.sampleLength = r4013.bit.sampleLength;
    return dmc;
  }

  /*
  ################################
  ||      CPU Read / Write      ||
  ################################
  */
  static u8 HandleCpuRead( u16 addr )
  {
    switch ( addr ) {
      case 0x4015: break; // TODO: Implement // NOLINT
      case 0x4017: break; // TODO: Implement // NOLINT
      default    : break;
    }
    return 0xFF;
  }

  void HandleCpuWrite( u16 address, u8 data )
  {
    switch ( address ) {
      // Pulse 1
      case 0x4000: r4000.value = data; break;
      case 0x4001: r4001.value = data; break;
      case 0x4002: r4002.value = data; break;
      case 0x4003: r4003.value = data; break;

      // Pulse 2
      case 0x4004: r4004.value = data; break;
      case 0x4005: r4005.value = data; break;
      case 0x4006: r4006.value = data; break;
      case 0x4007: r4007.value = data; break;

      // Triangle
      case 0x4008: r4008.value = data; break;
      case 0x400A: r400A.value = data; break;
      case 0x400B: r400B.value = data; break;

      // Noise
      case 0x400C: r400C.value = data; break;
      case 0x400E: r400E.value = data; break;
      case 0x400F: r400F.value = data; break;

      // DMC
      case 0x4010: r4010.value = data; break;
      case 0x4011: r4011.value = data; break;
      case 0x4012: r4012.value = data; break;
      case 0x4013: r4013.value = data; break;

      // TODO: 4015, 4017
      default: break;
    }
  }
};
