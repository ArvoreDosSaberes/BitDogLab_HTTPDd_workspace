/**
 * @file    oled_freeRTOS.h
 * @brief   Semáforos FreeRTOS para acesso thread-safe ao OLED
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

#ifndef OLED_FREERTOS_H
#define OLED_FREERTOS_H

#include "portmacro.h"

#ifdef __cplusplus
extern "C" {
#endif

BaseType_t initOledSemaphore();
BaseType_t takeOled();
BaseType_t releaseOled();

#ifdef __cplusplus
}
#endif

#endif