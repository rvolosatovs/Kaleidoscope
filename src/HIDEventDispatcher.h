#pragma once
#include "KeyboardioHID.h"

#include "EventDispatcher.h"

/** The HIDEventDispatcher is an EventDispatcher implementation
 * that knows how to send key reports to the standard KeyboardioHID
 * keyboard interface. */
class HIDEventDispatcher : public EventDispatcher {
 public:
  // Ensure that we call the parent constructor and get registered
  HIDEventDispatcher() : EventDispatcher() {}

  void begin() override {
    eventDispatchers().insertOrReplace(this);

    // TODO: intelligently use BootKeyboard at the right time
    Keyboard.begin();
    SystemControl.begin();
    ConsumerControl.begin();
  }

  void queryConnectionTypes(uint8_t &connectionMask) override {
    // We are USB, so enable those bits
    connectionMask |= Usb;
  }

  void consumerPress(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      ConsumerControl.press(keyCode);
    }
  }

  void consumerRelease(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      ConsumerControl.release(keyCode);
    }
  }

  void consumerReleaseAll(uint8_t connectionMask) override {
    if (connectionMask & Usb) {
      ConsumerControl.releaseAll();
    }
  }

  void consumerSendReport(uint8_t connectionMask) override {
    if (connectionMask & Usb) {
      ConsumerControl.sendReport();
    }
  }

  void systemPress(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      SystemControl.press(keyCode);
    }
  }

  void systemRelease(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      SystemControl.release();
    }
  }

  void keyPress(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      Keyboard.press(keyCode);
    }
  }

  void keyRelease(uint8_t connectionMask, uint8_t keyCode) override {
    if (connectionMask & Usb) {
      Keyboard.release(keyCode);
    }
  }

  void keyReleaseAll(uint8_t connectionMask) override {
    if (connectionMask & Usb) {
      Keyboard.releaseAll();
    }
  }

  void keySendReport(uint8_t connectionMask) override {
    if (connectionMask & Usb) {
      Keyboard.sendReport();
    }
  }

  void isModifierKeyActive(uint8_t connectionMask, Key mappedKey, boolean &isActive) override {
    if (connectionMask & Usb) {
      isActive = Keyboard.isModifierActive(mappedKey.keyCode);
    }
  }
};
