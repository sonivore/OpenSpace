/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <openspace/util/timemanager.h>

#include <openspace/engine/openspaceengine.h>
#include <openspace/engine/wrapper/windowwrapper.h>
#include <openspace/network/parallelpeer.h>
#include <openspace/util/timeline.h>

namespace openspace {

using datamessagestructures::TimeKeyframe;

void TimeManager::preSynchronization(double dt) {
    removeKeyframesBefore(_latestConsumedTimestamp);
    progressTime(dt);

    // Notify observers about time changes if any.
    double newTime = time().j2000Seconds();
    double newDeltaTime = _deltaTime;
    if (newTime != _lastTime) {
        for (const auto& it : _timeChangeCallbacks) {
            it.second();
        }
    }
    if (newDeltaTime != _lastDeltaTime) {
        for (const auto& it : _deltaTimeChangeCallbacks) {
            it.second();
        }
    }
    _lastTime = newTime;
    _lastDeltaTime = newDeltaTime;
}

void TimeManager::progressTime(double dt) {
    // Frames     |    1                    2          |
    //            |------------------------------------|
    // Keyframes  | a     b             c       d   e  |

    // 1: Previous frame
    // 2: Current frame (now)
    // lastPastKeyframe: c
    // firstFutureKeyframe: d
    // _latestConsumedTimestamp: a.timestamp

    if (_shouldSetTime) {
        // Setting the time using `setTimeNextFrame`
        // will override any timeline operations.
        time().setTime(_timeNextFrame.j2000Seconds());
        _shouldSetTime = false;
        return;
    }

    const double now = OsEng.windowWrapper().applicationTime();
    const std::deque<Keyframe<TimeKeyframeData>>& keyframes = _timeline.keyframes();

    auto firstFutureKeyframe = std::lower_bound(
        keyframes.begin(),
        keyframes.end(),
        now,
        &compareKeyframeTimeWithTime
    );

    // Check if there is a time jump between the latest consumed timestamp and 
    // the first future keyframe (non-inclusive interval).
    // I.e. are we consuming a time jump this frame?
    const bool consumingTimeJump = std::find_if(
        keyframes.begin(),
        firstFutureKeyframe,
        [this](const Keyframe<TimeKeyframeData>& f) {
            return f.timestamp > _latestConsumedTimestamp && f.data.jump;
        }
    ) != firstFutureKeyframe;
    
    const bool hasFutureKeyframes = firstFutureKeyframe != keyframes.end();
    const bool hasPastKeyframes = firstFutureKeyframe != keyframes.begin();

    const auto lastPastKeyframe = hasPastKeyframes ?
        (firstFutureKeyframe - 1) :
        keyframes.end();

    const bool hasConsumedLastPastKeyframe = hasPastKeyframes ?
        (lastPastKeyframe->timestamp <= _latestConsumedTimestamp) :
        true;

    if (hasFutureKeyframes && hasPastKeyframes && !firstFutureKeyframe->data.jump) {
        // If keyframes exist before and after this frame,
        // and the next keyframe is not a time jump, interpolate between those.
        interpolate(*lastPastKeyframe, *firstFutureKeyframe, now);
    } else if (!hasConsumedLastPastKeyframe) {
        applyKeyframe(*lastPastKeyframe);
    } else if (!isPaused()) {
        // If there are no keyframes to consider
        // and time is not paused, just advance time.
        time().advanceTime(dt * _deltaTime);
    }

    if (hasPastKeyframes) {
        _latestConsumedTimestamp = lastPastKeyframe->timestamp;
    }
}


void TimeManager::interpolate(
    const Keyframe<TimeKeyframeData>& past, 
    const Keyframe<TimeKeyframeData>& future,
    double appTime)
{
    // https://en.wikipedia.org/wiki/Spline_interpolation
    // q = (1 - t)y1 + t*y2 + t(1 - t)(a(1 - t) + bt), where
    // a = k1 * deltaAppTime - deltaSimTime,
    // b = -k2 * deltaAppTime + deltaSimTime,
    // with y1 = pastTime, y2 = futureTime, k1 = past dt, k2 = future dt.

    const double pastSimTime = past.data.time.j2000Seconds();
    const double futureSimTime = future.data.time.j2000Seconds();
    const double pastDerivative = past.data.pause ? 0.0 : past.data.delta;
    const double futureDerivative = future.data.pause ? 0.0 : future.data.delta;

    const double deltaAppTime = future.timestamp - past.timestamp;
    const double deltaSimTime = futureSimTime - pastSimTime;

    const double t = (appTime - past.timestamp) / deltaAppTime;
    const double a = pastDerivative * deltaAppTime - deltaSimTime;
    const double b = -futureDerivative * deltaAppTime + deltaSimTime;

    const double q = (1 - t)*pastSimTime + t*futureSimTime + t*(1 - t)*(a*(1 - t) + b*t);
    time().setTime(q);

    /*
    const double secondsOffTolerance = OsEng.parallelPeer().timeTolerance();
    double predictedTime = time().j2000Seconds() + time().deltaTime() *
        (next.timestamp - now);
    bool withinTolerance = std::abs(predictedTime - nextTime.j2000Seconds()) <
        std::abs(nextTime.deltaTime() * secondsOffTolerance);

    if (nextTime.deltaTime() == time().deltaTime() && withinTolerance) {
        time().advanceTime(dt);
        return;
    }*/

    /*
    double t0 = now - dt;
    double t1 = now;
    double t2 = next.timestamp;

    double parameter = (t1 - t0) / (t2 - t0);

    double y0 = time().j2000Seconds();
    // double yPrime0 = time().deltaTime();

    double y2 = nextTime.j2000Seconds();
    // double yPrime2 = nextTime.deltaTime();

    double y1 = (1 - parameter) * y0 + parameter * y2;
    double y1Prime = (y1 - y0) / dt;

    time().setDeltaTime(y1Prime);
    time().setTime(y1);
    */
}

void TimeManager::applyKeyframe(const Keyframe<TimeKeyframeData>& keyframe) {
    const Time& currentTime = keyframe.data.time;
    time().setTime(currentTime.j2000Seconds());
    _deltaTime = keyframe.data.delta;
}

void TimeManager::addKeyframe(double timestamp, TimeKeyframeData time) {
    _timeline.addKeyframe(timestamp, time);
}

void TimeManager::removeKeyframesAfter(double timestamp, bool inclusive) {
    _timeline.removeKeyframesAfter(timestamp, inclusive);
}


void TimeManager::removeKeyframesBefore(double timestamp, bool inclusive) {
    _timeline.removeKeyframesBefore(timestamp, inclusive);
}

void TimeManager::clearKeyframes() {
    _timeline.clearKeyframes();
}

void TimeManager::setTimeNextFrame(Time t) {
    _shouldSetTime = true;
    _timeNextFrame = t;
}

size_t TimeManager::nKeyframes() const {
    return _timeline.nKeyframes();
}

Time& TimeManager::time() {
    return _currentTime;
}

std::vector<Syncable*> TimeManager::getSyncables() {
    return{ &_currentTime };
}

TimeManager::CallbackHandle TimeManager::addTimeChangeCallback(TimeChangeCallback cb) {
    CallbackHandle handle = _nextCallbackHandle++;
    _timeChangeCallbacks.push_back({ handle, std::move(cb) });
    return handle;
}

TimeManager::CallbackHandle TimeManager::addDeltaTimeChangeCallback(TimeChangeCallback cb)
{
    CallbackHandle handle = _nextCallbackHandle++;
    _deltaTimeChangeCallbacks.push_back({ handle, std::move(cb) });
    return handle;
}

TimeManager::CallbackHandle TimeManager::addTimeJumpCallback(TimeChangeCallback cb) {
    CallbackHandle handle = _nextCallbackHandle++;
    _timeJumpCallbacks.push_back({ handle, std::move(cb) });
    return handle;
}

void TimeManager::removeTimeChangeCallback(CallbackHandle handle) {
    auto it = std::find_if(
        _timeChangeCallbacks.begin(),
        _timeChangeCallbacks.end(),
        [handle](const std::pair<CallbackHandle, std::function<void()>>& cb) {
            return cb.first == handle;
        }
    );

    ghoul_assert(
        it != _timeChangeCallbacks.end(),
        "handle must be a valid callback handle"
    );

    _timeChangeCallbacks.erase(it);
}

void TimeManager::removeDeltaTimeChangeCallback(CallbackHandle handle) {
    auto it = std::find_if(
        _deltaTimeChangeCallbacks.begin(),
        _deltaTimeChangeCallbacks.end(),
        [handle](const std::pair<CallbackHandle, std::function<void()>>& cb) {
            return cb.first == handle;
        }
    );

    ghoul_assert(
        it != _deltaTimeChangeCallbacks.end(),
        "handle must be a valid callback handle"
    );

    _deltaTimeChangeCallbacks.erase(it);
}

void TimeManager::removeTimeJumpCallback(CallbackHandle handle) {
    auto it = std::find_if(
        _timeJumpCallbacks.begin(),
        _timeJumpCallbacks.end(),
        [handle](const std::pair<CallbackHandle, std::function<void()>>& cb) {
        return cb.first == handle;
    }
    );

    ghoul_assert(
        it != _timeJumpCallbacks.end(),
        "handle must be a valid callback handle"
    );

    _timeJumpCallbacks.erase(it);
}

bool TimeManager::isPaused() const {
    return _timePaused;
}

double TimeManager::deltaTime() const {
    return _deltaTime;
}

void TimeManager::setDeltaTime(double newDeltaTime, double interpolationDuration) {
    if (newDeltaTime == _deltaTime) {
        return;
    }

    if (_timePaused) {
        _deltaTime = newDeltaTime;
        return;
    }

    const double now = OsEng.windowWrapper().applicationTime();
    Time newTime = time().j2000Seconds() +
        (_deltaTime + newDeltaTime) * interpolationDuration / 2.0;

    TimeKeyframeData currentKeyframe = { time(), _deltaTime, false, false };
    TimeKeyframeData futureKeyframe = { newTime, newDeltaTime, false, false };

    _deltaTime = newDeltaTime;

    clearKeyframes();
    addKeyframe(now, currentKeyframe);
    addKeyframe(now + interpolationDuration, futureKeyframe);
}

void TimeManager::setPause(bool pause, double interpolationDuration) {
    if (pause == _timePaused) {
        return;
    }

    const double now = OsEng.windowWrapper().applicationTime();
    Time newTime = time().j2000Seconds() + _deltaTime * interpolationDuration / 2.0;

    TimeKeyframeData currentKeyframe = { time(), _deltaTime, _timePaused, false };
    TimeKeyframeData futureKeyframe = { newTime, _deltaTime, pause, false };
    _timePaused = pause;

    clearKeyframes();
    addKeyframe(now, currentKeyframe);
    addKeyframe(now + interpolationDuration, futureKeyframe);
}

bool TimeManager::togglePause(double interpolationDuration) {
    setPause(!_timePaused, interpolationDuration);
    return _timePaused;
}


} // namespace openspace
