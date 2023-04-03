/*
 * display.h
 *
 *  Created on: Mar 27, 2023
 *      Author: fminoru
 */

#ifndef SRC_DISPLAY_H_
#define SRC_DISPLAY_H_

#include <stdint.h>

int display_init(void);

int dm_render_clear(void);

int display_update(void);
int dm_render_column_line(int col);

int dm_render_progress(const uint32_t value, const uint32_t total, int foward);

int dm_render_progress_dfu(const uint32_t value, const uint32_t total);

#endif /* SRC_DISPLAY_H_ */
