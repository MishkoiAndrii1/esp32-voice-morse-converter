#ifndef _I2S_H
#define _I2S_H

#include <Arduino.h>
#include "driver/i2s.h"

enum MicType {
  ADMP441,
  ICS43434
};

class I2S {
private:
  i2s_bits_per_sample_t BITS_PER_SAMPLE;

public:
  I2S(MicType micType);
  int Read(char* data, int numData);
  int GetBitPerSample();
};

#endif
