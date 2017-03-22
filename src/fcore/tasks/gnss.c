///
/// @file        tasks/gnss.c
/// @brief       FCORE - Task used for polling GNSS status regularly
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcore/fcore.h>
#include <fcore/tasks/gnss.h>
#include <fcore/gnss/gnss_parser.h>
#include <fcore/buses/uart.h>
#include "FreeRTOS.h"
#include "task.h"

#define GNSS_MAX_ATTEMPTS 4

void fcore_gnssTask(void* parameters) {
    while(true) {
        fcore_uartClear();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        

        taskENTER_CRITICAL();

        GNSSStatus state;
        for(int i = 0; i < GNSS_MAX_ATTEMPTS; ++i) {
            
            // Reset the GNSS NMEA parser
            state = GNSS_PARSING;
            fcore_gnssReset();
            
            // Get the next sentence from UART0
            while(fcore_uartAvailable() > 0 && state == GNSS_PARSING) {
                state = fcore_gnssFeed(fcore_uartReadChar());
            }
            
            if(state == GNSS_VALID) {
                break;
            }
        }
        switch(state) {
            case GNSS_VALID:
                FCORE.gnssStatus = STATUS_UP;
                break;
            case GNSS_NOTGGA:
                FCORE.gnssStatus = STATUS_RECOVERY;
                break;
            default:
                FCORE.gnssStatus = STATUS_DOWN;
                break;
        }
        taskEXIT_CRITICAL();

        vTaskDelay((FCORE_GNSS_INTERVAL * 1000) / portTICK_PERIOD_MS);
    }
}

bool fcore_startGNSSTask(void) {
    return xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256,
                       NULL, PRIORITY_GNSS, NULL) == pdPASS;
}
