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
  void (*render)(treadmill_t *treadmill);
} scene_t;

#endif
