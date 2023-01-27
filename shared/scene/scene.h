#ifndef SCENE_H
#define SCENE_H

#include "../treadmill.h"
#include "../treadmill_key.h"
#include "../treadmill_action.h"

typedef struct scene {
  treadmill_scene_t id;
  char name[16];
  void (*keypress)(treadmill_t *treadmill, const key_t key);
  void (*update)(treadmill_t *treadmill);
} scene_t;

/* Tempalte
#include "scene.h"

static void keypress(treadmill_t *treadmill, const key_t key) {
  switch (key) { default:; }
}

static void update(treadmill_t *treadmill) {
  treadmill_set_scene(treadmill, SCENE_IDLE);
}

const scene_t scene_init = {
    .id = SCENE_INIT,
    .name = "INIT",
    .keypress = keypress,
    .update = update,
};
*/

#endif
