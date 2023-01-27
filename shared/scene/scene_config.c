#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) { default:; }
}

static void update(treadmill_t *treadmill) {
  //
}

const scene_t scene_config = {
    .id = SCENE_INIT,
    .name = "INIT",
    .keypress = keypress,
    .update = update,
};
