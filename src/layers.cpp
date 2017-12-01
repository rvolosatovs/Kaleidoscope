#include "Kaleidoscope.h"

static uint8_t DefaultLayer;
static uint32_t LayerState;

uint8_t Layer_::topActiveLayer_;
Key Layer_::liveCompositeKeymap[ROWS][COLS];
uint8_t Layer_::activeLayers[ROWS][COLS];
Key(*Layer_::getKey)(uint8_t layer, byte row, byte col) = Layer.getKeyFromPROGMEM;

// The total number of defined layers in the firmware sketch keymaps[]
// array. If the keymap wasn't defined using CREATE_KEYMAP() in the
// sketch file, layer_count gets the default value of zero.
uint8_t layer_count __attribute__((weak)) = 0;

static void handleKeymapKeyswitchEvent(Key keymapEntry, uint8_t keyState) {
  if (keymapEntry.keyCode >= LAYER_SHIFT_OFFSET) {
    uint8_t target = keymapEntry.keyCode - LAYER_SHIFT_OFFSET;

    switch (target) {
    case KEYMAP_NEXT:
      if (keyToggledOn(keyState))
        Layer.next();
      else if (keyToggledOff(keyState))
        Layer.previous();
      break;

    case KEYMAP_PREVIOUS:
      if (keyToggledOn(keyState))
        Layer.previous();
      else if (keyToggledOff(keyState))
        Layer.next();
      break;

    default:
      /* The default case is when we are switching to a layer by its number, and
       * is a bit more complicated than switching there when the key toggles on,
       * and away when it toggles off.
       *
       * We want to handle the case where we have more than one momentary layer
       * key on our keymap that point to the same target layer, and we hold
       * both, and release one. In this case, the layer should remain active,
       * because the second momentary key is still held.
       *
       * To do this, we turn the layer back on if the switcher key is still
       * held, not only when it toggles on. So when one of them is released,
       * that does turn the layer off, but with the other still being held, the
       * layer will toggle back on in the same cycle.
       */
      if (keyIsPressed(keyState)) {
        if (!Layer.isActive(target))
          Layer.activate(target);
      } else if (keyToggledOff(keyState)) {
        Layer.deactivate(target);
      }
      break;
    }
  } else if (keyToggledOn(keyState)) {
    // switch keymap and stay there
    if (Layer.isActive(keymapEntry.keyCode) && keymapEntry.keyCode)
      Layer.deactivate(keymapEntry.keyCode);
    else
      Layer.activate(keymapEntry.keyCode);
  }
}

Key
Layer_::eventHandler(Key mappedKey, byte row, byte col, uint8_t keyState) {
  if (mappedKey.flags != (SYNTHETIC | SWITCH_TO_KEYMAP))
    return mappedKey;

  handleKeymapKeyswitchEvent(mappedKey, keyState);
  return Key_NoKey;
}

Layer_::Layer_(void) {
  defaultLayer(0);
}

Key
Layer_::getKeyFromPROGMEM(uint8_t layer, byte row, byte col) {
  Key key;

  key.raw = pgm_read_word(&(keymaps[layer][row][col]));

  return key;
}

void
Layer_::updateLiveCompositeKeymap(byte row, byte col) {
  int8_t layer = activeLayers[row][col];
  liveCompositeKeymap[row][col] = (*getKey)(layer, row, col);
}

void
Layer_::updateActiveLayers(void) {
  memset(activeLayers, DefaultLayer, ROWS * COLS);
  for (byte row = 0; row < ROWS; row++) {
    for (byte col = 0; col < COLS; col++) {
      int8_t layer = topActiveLayer_;

      while (layer > DefaultLayer) {
        if (Layer.isActive(layer)) {
          Key mappedKey = (*getKey)(layer, row, col);

          if (mappedKey != Key_Transparent) {
            activeLayers[row][col] = layer;
            break;
          }
        }
        layer--;
      }
    }
  }
}

void Layer_::updateTopActiveLayer(void) {
  for (int8_t i = 31; i >= 0; i--) {
    if (bitRead(LayerState, i)) {
      topActiveLayer_ = i;
      return;
    }
  }
  topActiveLayer_ = 0;
}

void Layer_::move(uint8_t layer) {
  LayerState = 0;
  activate(layer);
}

// Activate a given layer
void Layer_::activate(uint8_t layer) {
  // If we're trying to turn on a layer that doesn't exist, abort (but
  // if the keymap wasn't defined using the KEYMAPS() macro, proceed anyway
  if (layer_count != 0 && layer >= layer_count)
    return;

  // If the target layer was already on, return
  if (isActive(layer))
    return;

  // Otherwise, turn on its bit in LayerState
  bitSet(LayerState, layer);

  // If the target layer is above the previous highest active layer,
  // update highestLayer
  if (layer > topActiveLayer_)
    updateTopActiveLayer();

  // Update the keymap cache (but not liveCompositeKeymap; that gets
  // updated separately, when keys toggle on or off. See layers.h)
  updateActiveLayers();
}

// Deactivate a given layer
void Layer_::deactivate(uint8_t layer) {
  // If the target layer was already off, return
  if (!bitRead(LayerState, layer))
    return;

  // Turn off its bit in LayerState
  bitClear(LayerState, layer);

  // If the target layer was the previous highest active layer,
  // update highestLayer
  if (layer == topActiveLayer_)
    updateTopActiveLayer();

  // Update the keymap cache (but not liveCompositeKeymap; that gets
  // updated separately, when keys toggle on or off. See layers.h)
  updateActiveLayers();
}

boolean Layer_::isActive(uint8_t layer) {
  return bitRead(LayerState, layer);
}

void Layer_::next(void) {
  activate(topActiveLayer_ + 1);
}

void Layer_::previous(void) {
  deactivate(topActiveLayer_);
}

void Layer_::defaultLayer(uint8_t layer) {
  move(layer);
  DefaultLayer = layer;
}

uint8_t Layer_::defaultLayer(void) {
  return DefaultLayer;
}

uint32_t Layer_::getLayerState(void) {
  return LayerState;
}

Layer_ Layer;
