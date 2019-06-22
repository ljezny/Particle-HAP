/*----------------------------------------------------------------------*
 * Arduino Timezone Library                                             *
 * Jack Christensen Mar 2012                                            *
 *                                                                      *
 * Arduino Timezone Library Copyright (C) 2018 by Jack Christensen and  *
 * licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html   *
 *----------------------------------------------------------------------*/

#ifndef TIMEZONE_H_INCLUDED
#define TIMEZONE_H_INCLUDED


// convenient constants for TimeChangeRules
enum week_t {Last, First, Second, Third, Fourth};
enum dow_t {Sun=1, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};

// structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
struct TimeChangeRule
{
    char abbrev[6];    // five chars max
    uint8_t week;      // First, Second, Third, Fourth, or Last week of the month
    uint8_t dow;       // day of week, 1=Sun, 2=Mon, ... 7=Sat
    uint8_t month;     // 1=Jan, 2=Feb, ... 12=Dec
    uint8_t hour;      // 0-23
    int offset;        // offset from UTC in minutes
};


class Timezone
{
    public:
        Timezone(TimeChangeRule dstStart, TimeChangeRule stdStart);
        Timezone(TimeChangeRule stdTime);
        time_t toLocal(time_t utc);
        time_t toLocal(time_t utc, TimeChangeRule **tcr);
        time_t toUTC(time_t local);
        bool utcIsDST(time_t utc);
        bool locIsDST(time_t local);
        void setRules(TimeChangeRule dstStart, TimeChangeRule stdStart);
    private:
        void calcTimeChanges(int yr);
        void initTimeChanges();
        time_t toTime_t(TimeChangeRule r, int yr);
        TimeChangeRule m_dst;   // rule for start of dst or summer time for any year
        TimeChangeRule m_std;   // rule for start of standard time for any year
        time_t m_dstUTC;        // dst start for given/current year, given in UTC
        time_t m_stdUTC;        // std time start for given/current year, given in UTC
        time_t m_dstLoc;        // dst start for given/current year, given in local time
        time_t m_stdLoc;        // std time start for given/current year, given in local time
};
/*
//https://platformio.org/lib/show/76/Timezone
// Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
Timezone ausET(aEDT, aEST);

// Moscow Standard Time (MSK, does not observe DST)
TimeChangeRule msk = {"MSK", Last, Sun, Mar, 1, 180};
Timezone tzMSK(msk);

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

// United Kingdom (London, Belfast)
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone UK(BST, GMT);

// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
Timezone usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
Timezone usMT(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
Timezone usAZ(usMST);

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);
*/
#endif
