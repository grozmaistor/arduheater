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

#ifndef __UI_H__
#define __UI_H__

#include <Arduino.h>
#include "card.h"
#include "enum.h"
#include "observer.h"
#include "singleton.h"
#include "struct.h"

class ui : public Observer<message_t> {
private:
  Card* m_active_card;
  card_index_t m_active_index;

private:
  // uint16_t will allow up to ~65s timeout but due to the way we do time
  // accounting the max number of seconds for a card to timeout is ~32s which
  // is the max value of a int16 vartype.
  uint16_t m_active_timeout;
  int16_t  m_active_timeleft;

public:
  typedef Singleton<ui> single;

public:
  ui();
  virtual ~ui() {;}

public:
  inline void show(const card_index_t& card_index) {
    show(card_index, 0);
  }

public:
  void show(const card_index_t&, const uint16_t&);
  void update(const message_t&);
  void worker();

protected:
  void process_keypress(const message_t&);
};

#endif
