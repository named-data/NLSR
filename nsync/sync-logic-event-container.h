/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_LOGIC_EVENT_CONTAINER_H
#define SYNC_LOGIC_EVENT_CONTAINER_H

#include "sync-event.h"

#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/multi_index_container.hpp>
// #include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
// #include <boost/multi_index/composite_key.hpp>
// #include <boost/multi_index/hashed_index.hpp>
// #include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
// #include <boost/multi_index/mem_fun.hpp>

namespace mi = boost::multi_index;

namespace Sync
{

struct LogicEvent
{
  LogicEvent (const boost::system_time &_time, Event _event, uint32_t _label)
    : time (_time)
    , event (_event)
    , lbl (_label)
  { }
  
  boost::system_time time;
  Event event;
  uint32_t lbl;
};

/// @cond include_hidden
struct byLabel { } ;
/// @endcond

/**
 * \ingroup sync
 * @brief ???
 */
struct EventsContainer : public mi::multi_index_container<
  LogicEvent,
  mi::indexed_by<

    mi::ordered_non_unique<
      mi::member<LogicEvent, boost::system_time, &LogicEvent::time>
      >,

    mi::ordered_non_unique<
      mi::tag<byLabel>,
      mi::member<LogicEvent, uint32_t, &LogicEvent::lbl>
      >
    >
  >
{
};

} // Sync

#endif // SYNC_LOGIC_EVENT_CONTAINER_H
