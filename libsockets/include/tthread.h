/*
    Copyright 2011 Roberto Panerai Velloso.

    This file is part of libsockets.

    libsockets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libsockets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsockets.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TTHREAD_H_
#define TTHREAD_H_

#include <pthread.h>
#ifdef DEVEL_ENV
	#include "tobject.h"
#else
	#include <libsockets/tobject.h>
#endif

enum tThreadStatus {
     tThreadStopped,
     tThreadRunning
};

class tThread : public tObject {
friend void *pthread_execute(void *);
public:
       tThread(int);
       virtual ~tThread();
       virtual void execute() = 0;
       void start();
       void stop();
       tThreadStatus getThreadStatus();
       pthread_t getThreadId();
       void setSelfDestroy(int);
       int getSelfDestroy();
       
       virtual void onStart() = 0;
       virtual void onStop() = 0;
       
protected:
       pthread_t threadId;
       tThreadStatus threadStatus;
       int selfDestroy;
};

#endif /* TTHREAD_H_ */