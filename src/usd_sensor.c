#ifndef USD_SENSOR_DEF
#define USD_SENSOR_DEF

/*
 File:          usd_sensor.c
 Description:   Handler for the ultra-sonic distance sensor.
 Created:       May 24, 2017
 Author:        Matt Mumau
 */

#define _POSIX_C_SOURCE 199309L

/* System includes */
#include <sys/prctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

/* Raspberry Pi includes */
#include <wiringPi.h>

/* Application includes */
#include "main.h"
#include "config_defaults.h"
#include "utils.h"
#include "main.h"
#include "log.h"

/* Header */
#include "usd_sensor.h"

/* Forward decs */
static void *usd_sensor_main(void *arg);

static bool running = true;
static double distance;
static pthread_t usd_thread;
static int error;

void usd_sensor_init()
{
    error = pthread_create(&usd_thread, NULL, usd_sensor_main, NULL);
    if (error)
        APP_ERROR("Could not initialize USD sensor thread.", error);

    pinMode(DEFAULT_HRC_SR04_TRIGGER_PIN, OUTPUT);
    pinMode(DEFAULT_HRC_SR04_ECHO_PIN, INPUT);    

    digitalWrite(DEFAULT_HRC_SR04_TRIGGER_PIN, LOW);
}

void usd_sensor_halt()
{
    running = false;
    error = pthread_join(usd_thread, NULL);
    if (error)
        log_error("Could not rejoin from USD sensor thread.", error);
}

double usd_sensor_getdist()
{
    return distance;
}

static void *usd_sensor_main(void *arg)
{
    prctl(PR_SET_NAME, "PEABOT_USD\0", NULL, NULL, NULL);

    double new_distance;

    unsigned int timeout, max_timeout;
    max_timeout = 1000000;

    bool timeout_error = false;

    while (running)
    {
        timeout = 0;
        timeout_error = false;

        digitalWrite(DEFAULT_HRC_SR04_TRIGGER_PIN, HIGH);
        delayMicroseconds(20);
        digitalWrite(DEFAULT_HRC_SR04_TRIGGER_PIN, LOW);
 
        while (digitalRead(DEFAULT_HRC_SR04_ECHO_PIN) == LOW && !timeout_error)
        {
            timeout++;
            if (timeout > max_timeout)
                timeout_error = true;
        }
        timeout = 0;

        //Wait for echo end
        long start_time = micros();
        while (digitalRead(DEFAULT_HRC_SR04_ECHO_PIN) == HIGH && !timeout_error)
        {
            timeout++;
            if (timeout > max_timeout)
                timeout_error = true;
        }
        long travel_time = micros() - start_time;
 
        //Get distance in cm
        if (!timeout_error)
        {
            new_distance = travel_time / 58.0;
            if (new_distance <= 400.0)
            {
                distance = new_distance;
            }
        }

        //printf("Distance: %f\n", distance);

        delayMicroseconds(100000);
    }

    return (void *) NULL;
}

#endif