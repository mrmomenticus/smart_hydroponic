#pragma once

#include "config.h"
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/queue.h"

// Function Prototypes
void pump_task(void *param);