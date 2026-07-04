void initFlowSensor() {
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulseISR, RISING);
}

void processFlowTick(unsigned long snapshotsPulses) {
  float instantFlowRate = getCalibratedFlowRate((float)snapshotsPulses);
  flowRateHistory[flowHistoryIndex] = instantFlowRate;
  flowHistoryIndex = (flowHistoryIndex + 1) % FLOW_MEDIAN_SIZE;
  
  if (flowHistoryIndex == 0) flowHistoryFilled = true;
  
  if (flowHistoryFilled) {
    currentFlowRate = medianOf(flowRateHistory, FLOW_MEDIAN_SIZE);
  } else {
    currentFlowRate = instantFlowRate;
  }
}