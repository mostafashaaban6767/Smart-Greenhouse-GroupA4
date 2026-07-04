void initScaleHX711() {
  scale.begin(SCALE_DOUT_PIN, SCALE_SCK_PIN);
}

bool readScaleMedianSafe(int samples, unsigned long perSampleTimeoutMs, long *outMedian) {
  long buf[samples];
  int collected = 0;

  for (int i = 0; i < samples; i++) {
    unsigned long waitStart = millis();
    while (!scale.is_ready()) {
      if (millis() - waitStart > perSampleTimeoutMs) {
        break; 
      }
    }
    if (scale.is_ready()) {
      buf[collected] = scale.read();
      collected++;
    } else {
      break; 
    }
  }

  if (collected == 0) {
    return false;
  }

  for (int i = 1; i < collected; i++) {
    long key = buf[i];
    int j = i - 1;
    while (j >= 0 && buf[j] > key) {
      buf[j + 1] = buf[j];
      j--;
    }
    buf[j + 1] = key;
  }

  *outMedian = buf[collected / 2];
  return true;
}

void updateScaleWeight() {
  if (scale.is_ready()) {
    long medianRaw;
    if (readScaleMedianSafe(SCALE_SAMPLE_COUNT, SCALE_SAMPLE_TIMEOUT_MS, &medianRaw)) {
      currentWeight = getCalibratedScaleWeight((double)medianRaw);
    }
  }
}