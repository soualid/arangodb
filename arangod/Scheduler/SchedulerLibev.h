////////////////////////////////////////////////////////////////////////////////
/// @brief input-output scheduler using libev
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2008-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_SCHEDULER_SCHEDULER_LIBEV_H
#define ARANGODB_SCHEDULER_SCHEDULER_LIBEV_H 1

#include "Basics/Common.h"
#include "Scheduler/Scheduler.h"
#include "Basics/Mutex.h"
#include "Basics/SpinLock.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              class SchedulerLibev
// -----------------------------------------------------------------------------

namespace triagens {
  namespace rest {

////////////////////////////////////////////////////////////////////////////////
/// @brief input-output scheduler using libev
////////////////////////////////////////////////////////////////////////////////

    class SchedulerLibev : public Scheduler {
      private:
        SchedulerLibev (SchedulerLibev const&);
        SchedulerLibev& operator= (SchedulerLibev const&);

// -----------------------------------------------------------------------------
// --SECTION--                                             static public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the available backends
////////////////////////////////////////////////////////////////////////////////

        static int availableBackends ();

////////////////////////////////////////////////////////////////////////////////
/// @brief switch the libev allocator
////////////////////////////////////////////////////////////////////////////////

        static void switchAllocator ();

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a scheduler
////////////////////////////////////////////////////////////////////////////////

        explicit
        SchedulerLibev (size_t nrThreads = 1, int backend = BACKEND_AUTO);

////////////////////////////////////////////////////////////////////////////////
/// @brief deletes a scheduler
////////////////////////////////////////////////////////////////////////////////

        ~SchedulerLibev ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 Scheduler methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void eventLoop (EventLoop) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void wakeupLoop (EventLoop) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installSocketEvent (EventLoop, EventType, Task*, TRI_socket_t) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void startSocketEvents (EventToken) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void stopSocketEvents (EventToken) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installTimerEvent (EventLoop, Task*, double timeout) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void clearTimer (EventToken) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void rearmTimer (EventToken, double timeout) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installPeriodicEvent (EventLoop, Task*, double offset, double interval) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void rearmPeriodic (EventToken, double offset, double timeout) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        EventToken installSignalEvent (EventLoop, Task*, int signal) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void uninstallEvent (EventToken) override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void signalTask (std::unique_ptr<TaskData>&) override;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up an event lookup
////////////////////////////////////////////////////////////////////////////////

        void* lookupLoop (EventLoop);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief backend to use
////////////////////////////////////////////////////////////////////////////////

        int _backend;

////////////////////////////////////////////////////////////////////////////////
/// @brief event loops
////////////////////////////////////////////////////////////////////////////////

        void* _loops;

////////////////////////////////////////////////////////////////////////////////
/// @brief event wakers
////////////////////////////////////////////////////////////////////////////////

        void* _wakers;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the allocator was switched
////////////////////////////////////////////////////////////////////////////////

        static bool SwitchedAllocator;

    };
  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
