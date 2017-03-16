/**
 * Arduheater - Telescope heat controller
 * Copyright (C) 2016-2017 João Brázio [joao@brazio.org]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"

void utils::itoa(char* buf, uint8_t& pos, int16_t n, const uint8_t& base, uint8_t digits) {
  if (n == 0) {
    buf[pos++] = 0x30;
    buf[pos] = 0x00;
    return;
  }

  if (n < 0) {
    buf[pos++] = pgm_read_byte(string_minus);
    n *= -1;  // no need to deal with signs from now on
  }

  if (digits == 0) {
    int16_t t = n;
    while (t) { t /= base; digits++; }
  }

  uint8_t start = pos;
  uint8_t end   = start + digits;

  while (end > start) {
    buf[--end] = '0' + n % base;
    n /= base;
    pos++;
  }

  buf[pos] = 0x00;
}

uint8_t utils::mstotick(const uint16_t& ms) {
  // WARNING: max 12750ms as input
  return ms / HEARTBEAT;
}

float utils::weather::dew(const float& t, const float& rh) {
  #ifdef NOAA_DEW_FUNCTION
    // dewPoint function NOAA
    // reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
    // reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm

    // (1) Saturation Vapor Pressure = ESGG(T)
    const float RATIO = 373.15 / (273.15 + t);
    float RHS = -7.90298 * (RATIO - 1);
    RHS += 5.02808 * log10(RATIO);
    RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
    RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
    RHS += log10(1013.246);

    // factor -3 is to adjust units - Vapor Pressure SVP * rh
    const float VP = pow(10, RHS - 3) * rh;

    // (2) DEWPOINT = F(Vapor Pressure)
    const float T = log(VP/0.61078);   // temp var
    return (241.88 * T) / (17.558 - T);
  #else
    // delta max = 0.6544 wrt dewPoint()
    // 6.9 x faster than dewPoint function NOAA
    // reference: http://en.wikipedia.org/wiki/Dew_point
    const float temp = (17.271f * t) / (237.7f + t) + log(rh * 0.01);
    return (237.7f * temp) / (17.271f - temp);
  #endif
}