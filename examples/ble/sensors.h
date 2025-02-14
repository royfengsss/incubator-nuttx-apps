/****************************************************************************
 * apps/examples/ble/sensors.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <stdio.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Retrieves the current temperature */

float get_temperature(void);

/* Retrieves the current atmospheric pressure */

float get_pressure(void);

/* Retrieves the current gas resistance */

float get_gas_resistance(void);

/* Retrieves the current humidity */

float get_humidity(void);

/* Monitors sensors in a continuous loop */

void *monitor_sensors(void *arg);

/* Initializes the sensors */

int init_sensors(void);

/* Closes the sensors' file descriptors */

void close_sensors_fd(void);
