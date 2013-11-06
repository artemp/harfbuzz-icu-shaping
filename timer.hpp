#ifndef TIMER_HPP
#define TIMER_HPP

// stl
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WINDOWS
#define NOMINMAX
#include <windows.h>
#else
#include <sys/time.h> // for gettimeofday() on unix
#include <sys/resource.h>
#endif


// Try to return the time now
inline double time_now()
{
#ifdef _WINDOWS
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return double(t.QuadPart) / double(f.QuadPart);
#else
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec * 1e-6;
#endif
}


// Measure times in both wall clock time and CPU times. Results are returned in milliseconds.
class timer
{
public:
    timer()
    {
        restart();
    }

    virtual ~timer()
    {
    }

    void restart()
    {
        stopped_ = false;
        wall_clock_start_ = time_now();
        cpu_start_ = clock();
    }

    virtual void stop() const
    {
        stopped_ = true;
        cpu_end_ = clock();
        wall_clock_end_ = time_now();
    }

    double cpu_elapsed() const
    {
        // return elapsed CPU time in ms
        if (! stopped_)
        {
            stop();
        }

        return ((double) (cpu_end_ - cpu_start_)) / CLOCKS_PER_SEC * 1000.0;
    }

    double wall_clock_elapsed() const
    {
        // return elapsed wall clock time in ms
        if (! stopped_)
        {
            stop();
        }

        return (wall_clock_end_ - wall_clock_start_) * 1000.0;
    }

protected:
    mutable double wall_clock_start_, wall_clock_end_;
    mutable clock_t cpu_start_, cpu_end_;
    mutable bool stopped_;
};

//  A progress_timer behaves like a timer except that the destructor displays
//  an elapsed time message at an appropriate place in an appropriate form.
class progress_timer : public timer
{
public:
    progress_timer(std::ostream & os, std::string const& base_message)
        : os_(os),
          base_message_(base_message)
    {}

    ~progress_timer()
    {
        if (! stopped_)
        {
            stop();
        }
    }

    void stop() const
    {
        timer::stop();
        try
        {
            std::ostringstream s;
            s.precision(2);
            s << std::fixed;
            s << wall_clock_elapsed() << "ms (cpu " << cpu_elapsed() << "ms)";
            s << std::setw(30 - (int)s.tellp()) << std::right << "| " << base_message_ << "\n";
            os_ << s.str();
        }
        catch (...) {} // eat any exceptions
    }

    void discard()
    {
        stopped_ = true;
    }

private:
    std::ostream & os_;
    std::string base_message_;
};

#endif

