/*
 * Copyright Aleksey Verkholat 2018
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt
*/

#pragma once
#include "std_import.hpp"

#ifdef __linux

#include <unistd.h>
#include <sys/times.h>

#elif _WIN32

#include <Windows.h>

#else

#error "Unknown OS"

#endif

namespace uf
{
    template<typename TimePoint>
    class basic_time_meter
    {
        function<TimePoint()> get_now_;
        function<double(TimePoint, TimePoint)> get_sec_;

        TimePoint begin_;
        TimePoint stop_;
        bool stopped_;

    public:
        template<typename GetNow, typename GetSec>
        basic_time_meter(GetNow&& gn, GetSec&& gs) : get_now_(std::forward<GetNow>(gn)), get_sec_(std::forward<GetSec>(gs)),
                                                     begin_(get_now_()), stopped_(false) { }

        double seconds() const
        {
            if (stopped_)
                return get_sec_(begin_, stop_);
            return get_sec_(begin_, get_now_());
        }

        double restart()
        {
            const TimePoint now = get_now_();
            double result = seconds(now);
            stopped_ = false;
            begin_ = now;
            return result;
        }

        double stop()
        {
            if (stopped_)
                return seconds();
            stop_ = get_now_();
            stopped_ = true;
            return seconds(stop_);
        }

        void start()
        {
            if (!stopped_)
                return;
            stopped_ = false;
            begin_ += get_now_() - stop_;
        }

        bool stopped() const
        {
            return stopped_;
        }

    private:
        double seconds(TimePoint now) const
        {
            if (stopped_)
                return get_sec_(begin_, stop_);
            return get_sec_(begin_, now);
        }
    };

    class time_meter : public basic_time_meter<chrono::high_resolution_clock::time_point>
    {
    public:
        using time_point = chrono::high_resolution_clock::time_point;

        time_meter() : basic_time_meter<time_point>(chrono::high_resolution_clock::now, get_seconds) { }

    private:
        static double get_seconds(time_point p1, time_point p2)
        {
            return static_cast<double>((p2 - p1).count()) / chrono::high_resolution_clock::period::den;
        }
    };

#ifdef __linux__

    class proc_time_meter : public basic_time_meter<i64>
    {
    public:
        using time_point = i64;

        proc_time_meter() : basic_time_meter<time_point>(get_now, get_seconds) { }

    private:
        static double get_seconds(time_point p1, time_point p2)
        {
            static const auto per_second = sysconf(_SC_CLK_TCK);
            return static_cast<double>(p2 - p1) / per_second;
        }

        static time_point get_now()
        {
            tms result;
            times(&result);
            return result.tms_stime + result.tms_utime;
        }
    };

#elif _WIN32

// TODO

#endif

}
// end namespace uf





