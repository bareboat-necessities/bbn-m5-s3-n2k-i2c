#ifndef status_led_h
#define status_led_h

static bool led_state = false;

void ToggleLed() {
  if (led_state) {
    AtomS3.dis.drawpix(0x00ff00);
  } else {
    AtomS3.dis.drawpix(0x000000);
  }
  AtomS3.update();
  led_state = !led_state;
}

#endif
