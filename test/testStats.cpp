/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2016 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <chrono>

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

using namespace std;

#include "../include/portab.h"
#include "testStats.h"

#define NUM_TIMERS 2000
#define COUNTER_SLOTS 200

time_point<chrono::steady_clock> ttimerUser0;
time_point<chrono::steady_clock> ttimerUser1;
time_point<chrono::steady_clock> ttimerListUser0[NUM_TIMERS];
time_point<chrono::steady_clock> ttimerListUser1[NUM_TIMERS];

clock_t ttimerSys0;
clock_t ttimerSys1;
clock_t ttimerListSys0[NUM_TIMERS];
clock_t ttimerListSys1[NUM_TIMERS];

int ttimerCount;
int ttimerListCount[NUM_TIMERS];

int ttimerNameSet;

char ttimerName[80];

long long ttimerUserCum;
long long ttimerSysCum;
long long ttimerListUserCum[NUM_TIMERS];
long long ttimerListSysCum[NUM_TIMERS];
long long tpredError;
long long tpredAbsError;


void TestInitTimer()
{
  ttimerCount = 0;
  ttimerUserCum = 0;
  ttimerSysCum = 0;
  ttimerNameSet = 0;

  tpredError = 0;
  tpredAbsError = 0;
}


void TestSetTimerName(const char * name)
{
  strcpy(ttimerName, name);
  ttimerNameSet = 1;
}


void TestStartTimer()
{
  ttimerCount++;
  ttimerSys0 = clock();

  ttimerUser0 = Clock::now();
}


void TestEndTimer()
{
  ttimerSys1 = clock();

  ttimerUser1 = Clock::now();

  chrono::duration<double, milli> d = ttimerUser1 - ttimerUser0;

  int ttimeUser = static_cast<int>(1000. * d.count());

  ttimerUserCum += ttimeUser;

  ttimerSysCum += static_cast<int>((1000 * (ttimerSys1 - ttimerSys0)) /
                                   static_cast<double>(CLOCKS_PER_SEC));
}


void TestPrintTimer()
{
  if (ttimerCount == 0) return;

  if (ttimerNameSet)
    printf("%-18s : %s\n", "Timer name", ttimerName);

  printf("%-18s : %10d\n", "Number of calls", ttimerCount);

  if (ttimerUserCum == 0)
    printf("%-18s : %s\n", "User time", "zero");
  else
  {
    printf("%-18s : %10lld\n", "User time/ticks", ttimerUserCum);
    printf("%-18s : %10.2f\n", "User per call",
           static_cast<float>(ttimerUserCum / ttimerCount));
  }

  if (ttimerSysCum == 0)
    printf("%-18s : %s\n", "Sys time", "zero");
  else
  {
    printf("%-18s : %10lld\n", "Sys time/ticks", ttimerSysCum);
    printf("%-18s : %10.2f\n", "Sys per call",
           static_cast<float>(ttimerSysCum / ttimerCount));
    printf("%-18s : %10.2f\n", "Ratio",
           static_cast<float>(ttimerSysCum / ttimerUserCum));
  }
  printf("\n");
}


void TestInitTimerList()
{
  for (int i = 0; i < NUM_TIMERS; i++)
  {
    ttimerListCount [i] = 0;
    ttimerListUserCum[i] = 0;
    ttimerListSysCum [i] = 0;
  }
}


void TestStartTimerNo(int no)
{
  ttimerListCount[no]++;
  ttimerListSys0[no] = clock();

  ttimerListUser0[no] = Clock::now();
}


void TestEndTimerNo(int no)
{
  ttimerListSys1[no] = clock();

  ttimerListUser1[no] = Clock::now();

  chrono::duration<double, milli> d = ttimerListUser1[no] - 
    ttimerListUser0[no];

  int timeUser = static_cast<int>(1000. * d.count());

  ttimerListUserCum[no] += static_cast<long long>(timeUser);

  ttimerListSysCum[no] +=
    static_cast<long long>((1000 *
                            (ttimerListSys1[no] - ttimerListSys0[no])) /
                           static_cast<double>(CLOCKS_PER_SEC));
}


void TestEndTimerNoAndComp(int no, int pred)
{
  ttimerListSys1[no] = clock();

  ttimerListUser1[no] = Clock::now();

  chrono::duration<double, milli> d = ttimerListUser1[no] - 
    ttimerListUser0[no];

  int timeUser = static_cast<int>(1000. * d.count());

  ttimerListUserCum[no] += static_cast<long long>(timeUser);

  tpredError += timeUser - pred;

  tpredAbsError += (timeUser >= pred ?
                    timeUser - pred : pred - timeUser);

  ttimerListSysCum[no] +=
    static_cast<long long>(
      (1000 * (ttimerListSys1[no] - ttimerListSys0[no])) /
      static_cast<double>(CLOCKS_PER_SEC));
}


void TestPrintTimerList()
{
  int totNum = 0;
  for (int no = 0; no < NUM_TIMERS; no++)
  {
    if (ttimerListCount[no] == 0)
      continue;

    totNum += ttimerListCount[no];
  }

  if (totNum == 0)
    return;

  printf("%5s %10s %12s %10s %10s\n",
         "n", "Number", "User ticks", "Avg", "Syst time");

  for (int no = 0; no < NUM_TIMERS; no++)
  {
    double avg = static_cast<double>(ttimerListUserCum[no]) /
                 static_cast<double>(ttimerListCount[no]);

    // For some reason I have trouble when putting it on one line...
    printf("%5d %10d %12lld ",
           no,
           ttimerListCount[no],
           ttimerListUserCum[no]);
    printf(" %10.2f %10lld\n",
           avg,
           ttimerListSysCum[no]);
  }
  printf("\n");
  if (tpredError != 0)
  {
    printf("Total number %10d\n", totNum);
    printf("Prediction mean %10.0f\n",
           static_cast<double>(tpredError) /
           static_cast<double>(totNum));
    printf("Prediction abs mean %10.0f\n",
           static_cast<double>(tpredAbsError) /
           static_cast<double>(totNum));
    printf("\n");
  }
}

