/**
 * @file    I2C_freeRTOS.hpp
 * @brief   Semáforos FreeRTOS para acesso thread-safe ao I2C
 * 
 * @project BitDogLab_HTTPDd_workspace
 * @url     https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace
 * 
 * @author  Carlos Delfino
 * @email   consultoria@carlosdelfino.eti.br
 * @website https://carlosdelfino.eti.br
 * @github  https://github.com/CarlosDelfino
 * 
 * @license CC BY 4.0 - https://creativecommons.org/licenses/by/4.0/
 */

#pragma once

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "semphr.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t initI2CSemaphore(uint8_t port);
BaseType_t takeI2C(uint8_t port, TickType_t delay);
BaseType_t releaseI2C(uint8_t port);

#ifdef __cplusplus
}
#endif
