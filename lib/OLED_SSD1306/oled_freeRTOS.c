/**
 * @file    oled_freeRTOS.c
 * @brief   Implementação de semáforos FreeRTOS para OLED
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

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "semphr.h"

#include "log_vt100.h"
#include "oled_freeRTOS.h"

SemaphoreHandle_t oledSemaphore = NULL;

BaseType_t initOledSemaphore() {
    oledSemaphore = xSemaphoreCreateMutex();
    if (oledSemaphore == NULL) {
        LOG_WARN("[OLED] Semaphore not initialized");
        return pdFAIL;
    }
    LOG_INFO("[OLED] Semaphore initialized");
    return pdPASS;
}

BaseType_t takeOled() {
    if (oledSemaphore == NULL) {
        LOG_WARN("[OLED] Semaphore not initialized");
        return pdFAIL;
    }
    LOG_INFO("[OLED] Semaphore taken");
    return xSemaphoreTake(oledSemaphore, portMAX_DELAY);
}

BaseType_t releaseOled() {
    if (oledSemaphore == NULL) {
        LOG_WARN("[OLED] Semaphore not initialized");
        return pdFAIL;
    }
    LOG_INFO("[OLED] Semaphore released");
    return xSemaphoreGive(oledSemaphore);
}
