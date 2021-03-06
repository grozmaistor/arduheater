/**
 * Arduheater - An intelligent dew buster for astronomy
 * Copyright (C) 2016-2019 João Brázio [joao@brazio.org]
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

#include "protocol.h"

void protocol::process(const char *cmd) {
  char buffer[64];
  Output *channel;
  Output::runtime_t dump;

  switch (cmd[0]) {
    case 0:
      LogLn::PGM(PSTR("Arduheater " ARDUHEATER_VERSION " ['$' for help]"));
      break;

    case '$':
      LogLn::PGM(PSTR("Currently there is no support for a human on the serial line."));
      break;

    case '+':
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      output[cmd[1] - '0'].enable();
      break;

    case '-':
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      output[cmd[1] - '0'].disable();
      break;

    case '?': // Output status --------------------------------------------------------------------
      sprintf_P(buffer, PSTR(":?%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i#"),
        output[0].is_connected(), output[1].is_connected(), output[2].is_connected(), output[3].is_connected(),
        output[0].is_ready(),     output[1].is_ready(),     output[2].is_ready(),     output[3].is_ready(),
        output[0].is_enabled(),   output[1].is_enabled(),   output[2].is_enabled(),   output[3].is_enabled()
      ); Log::string(buffer);

      break;

    case 'A': // Ambient --------------------------------------------------------------------------
      sprintf_P(buffer, PSTR(":A%i,%i,%i#"),
        ftol(ambient.temperature()), ftol(ambient.humidity()), ftol(ambient.dew_point())
      ); Log::string(buffer);
      break;

    case 'B': // Output data ----------------------------------------------------------------------
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      channel = &output[(uint8_t)(cmd[1] - '0')];

      sprintf_P(buffer, PSTR(":B%i,%i,%i,%i#"), (uint8_t)(cmd[1] - '0'),
        ftol(channel->temperature()), ftol(channel->setpoint()), channel->output_value()
      ); Log::string(buffer);
      break;

    case 'C': // PID data -------------------------------------------------------------------------
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      channel = &output[(uint8_t)(cmd[1] - '0')];

      dump = channel->export_runtime();

      sprintf_P(buffer, PSTR(":C%i,%i,%i,%i,%i#"), (uint8_t)(cmd[1] - '0'),
        // Cast away the volatile qualifier
        ftol((float)dump.Pterm), ftol((float)dump.Iterm), ftol((float)dump.Dterm), dump.u
      ); Log::string(buffer);
      break;

    case 'D': // Output config --------------------------------------------------------------------
      // TODO this section needs to be better structured
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      channel = &output[(uint8_t)(cmd[1] - '0')];

      if(strlen(cmd) < 3) {
        sprintf_P(buffer, PSTR(":D%i,%i,%i,%i,%i,%i,%i,%i,%i#"), (uint8_t)(cmd[1] - '0'),
          channel->min_output(), channel->max_output(), (channel->is_autostart()) ? 1 : 0,
          ftol(channel->temp_offset()), ftol(channel->setpoint_offset()),
          ftol(channel->kp()), ftol(channel->ki()), ftol(channel->kd())
        ); Log::string(buffer);
      }

      else {
        memset(&buffer, 0, sizeof(buffer));
        int16_t args[9] = { 0 };
        int i = 0, n = 0;

        while (*cmd)
        {
          char c = *cmd++;

          switch(c)
          {
            case ',':
              args[n++] = atol2(buffer);
              memset(&buffer, 0, sizeof(buffer));
              i = 0;
              break;

            default:
              // only allows digits and negative sign
              if(is_digit(c) || c == '-') { buffer[i++] = c; }
          }
        }

        // last one is missed due to the string not
        // having the last ',' separator.
        args[n++] = atol2(buffer);

        // really poor man's input validation
        if(n != 9) { return; }

        output[args[0]].import_config({
          (uint8_t)args[1], (uint8_t)args[2], (bool)args[3], ltof(args[4]), ltof(args[5]), ltof(args[6]), ltof(args[7]), ltof(args[8])
        });
      }
      break;

    case 'F': // Sensor config --------------------------------------------------------------------
      if((uint8_t)(cmd[1] - '0') > 3) { return; }
      channel = &output[(uint8_t)(cmd[1] - '0')];

      sprintf_P(buffer, PSTR(":D%i,%i,%i,%i,%i#"), (uint8_t)(cmd[1] - '0'),
        ftol(channel->nominal_temp()), ftol(channel->resistor_value()), channel->bcoefficient_value(), channel->nominal_value()
      ); Log::string(buffer);
      break;

    case 'G': // Ambient config --------------------------------------------------------------------
      // TODO this section needs to be better structured
      if(strlen(cmd) < 3) {
        sprintf_P(buffer, PSTR(":G%i,%i#"),
          ftol(ambient.temp_offset()), ftol(ambient.rh_offset())
        ); Log::string(buffer);
      }

      else {
        memset(&buffer, 0, sizeof(buffer));
        int16_t args[2] = { 0 };
        int i = 0, n = 0;

        while (*cmd)
        {
          char c = *cmd++;

          switch(c)
          {
            case ',':
              args[n++] = atol2(buffer);
              memset(&buffer, 0, sizeof(buffer));
              i = 0;
              break;

            default:
              // only allows digits and negative sign
              if(is_digit(c) || c == '-') { buffer[i++] = c; }
          }
        }

        // last one is missed due to the string not
        // having the last ',' separator.
        args[n++] = atol2(buffer);

        // really poor man's input validation
        if(n != 2) { return; }

        ambient.import_config({
          ltof(args[0]), ltof(args[1])
        });
      }
      break;

    case 'V': // Version --------------------------------------------------------------------------
      sprintf_P(buffer, PSTR(":V%S#"),
        PSTR(ARDUHEATER_VERSION)
      ); Log::string(buffer);
      break;


    case 'I': // Reserved -------------------------------------------------------------------------
    case 'E':
    case 'W':
      // These are reserved keywords for the status messages.
      break;

    default: // Default ---------------------------------------------------------------------------
      LogLn::PGM(PSTR("E Invalid command, type '$' for help."));
      break;
  }
}
