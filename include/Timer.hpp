#pragma once
//
// Timer abstract interface
// ---------------------------------------------------------------------------
// (c)2019 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include "Duration.hpp"



/// @namespace lr::Timer
/// An abstract interface to a tick counter of the platform.
/// The idea of this interface is to make the other tools and components
/// more portable.


namespace lr {
namespace Timer {


/// Get the current real timer counter tick.
///
/// This tick count infinitely from zero to the end of the 32bit value.
///
/// @return The tick count in milliseconds.
///
Milliseconds tickMilliseconds();

/// Wait for the next tick.
///
/// This function will wait for the next tick of the real time counter.
///
void waitForNextTick();

/// Wait a number of milliseconds.
///
/// @param milliseconds The number of milliseconds to wait.
///
void delayMilliseconds(uint32_t milliseconds);

/// Wait a number of microseconds.
///
/// @param microseconds The number of microseconds to wait.
///
void delayMicroseconds(uint32_t microseconds);

/// Wait a given duration.
///
/// By default the precision and range is limited by the 32bit real time counter.
/// It is safe to assume delays up to one hour should work fine.
///
/// @param duration The duration to wait.
///
template<typename Ratio>
inline void delay(const Duration<Ratio> &duration) {
    delayMilliseconds(duration.toMilliseconds().ticks());
}

template<>
inline void delay<std::milli>(const Milliseconds &milliseconds) {
    delayMilliseconds(milliseconds.ticks());
}

template<>
inline void delay<std::micro>(const Microseconds &microseconds) {
    delayMicroseconds(microseconds.ticks());
}

template<>
inline void delay<std::nano>(const Nanoseconds &nanoseconds) {
    delayMicroseconds(nanoseconds.toMicroseconds().ticks());
}

/// A simple timer to measure elapsed time in milliseconds.
///
/// This timer only works up the resolution of the 32bit value, which
/// is 596 hours or 24 days. This class should be only used for very
/// short time measurements.
///
class Elapsed
{
public:
    /// Create a new elapsed timer which starts at the point of construction.
    ///
    inline Elapsed()
        : _startTime(tickMilliseconds())
    {
    }
    
    /// Restart the timer.
    ///
    inline void restart() {
        _startTime = tickMilliseconds();
    }
    
    /// Check the number of elapsed milliseconds since start.
    ///
    /// @return The number of elapsed milliseconds.
    ///
    inline Milliseconds elapsedTime() const {
        return tickMilliseconds()-_startTime;
    }

    /// Check if a function run into a time-out after the given number of milliseconds.
    ///
    /// If you just check for a time-out, try the `Deadline` class.
    /// The time-out is limited to the resolution of the 32bit value. Therefore
    /// after 596 hours or 24 days the timeout "resets".
    ///
    /// @return `true` if the timeout was reached.
    ///
    inline bool hasTimeout(const Milliseconds timeout) const {
        return elapsedTime() >= timeout;
    }

private:
    Milliseconds _startTime; ///< The start time.
};


/// A simple timer to check a timeout in milliseconds.
///
/// This timer only works up the resolution of the 32bit value, which
/// is 596 hours or 24 days. This class should be only used for very
/// short time measurements.
///
class Deadline
{
public:
    /// Create a new deadline timer with a given timeout.
    ///
    /// @param timeout in milliseconds.
    ///
    inline Deadline(const Milliseconds timeout) noexcept
        : _endTime(tickMilliseconds()+timeout)
    {
    }
    
    /// Restart the timer.
    ///
    inline void restart(const Milliseconds timeout) noexcept {
        _endTime = (tickMilliseconds()+timeout);
    }

    /// Check if this timer has expired.
    ///
    inline bool hasTimeout() const noexcept {
        return tickMilliseconds().deltaTo(_endTime) <= 0;
    }

    /// Check if the timer is still in time.
    ///
    inline bool isInTime() const noexcept {
        return tickMilliseconds().deltaTo(_endTime) > 0;
    }

private:
    Milliseconds _endTime; ///< The end time.
};


}
}