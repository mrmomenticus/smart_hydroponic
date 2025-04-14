#pragma once

#include "config.h"
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/queue.h"

// Function Prototypes
void water_level_task(void *param);