/**
 * NDTime (Natural Deep Time) class for Discrete Event Simulation purposes
 * Copyright (C) 2016  Laouen Mayal Louan Belloli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NDTIME
#define NDTIME

#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>
#include <cstdlib>
//#include <boost/algorithm/string.hpp>
//#include <boost/functional/hash.hpp>


class NDTime : public Time {
private:
    bool _inf;
    bool _possitive;
    int _hours; // 3600 seconds
    int _minutes; // 60 seconds
    int _seconds; // 10^0 seconds
    int _milliseconds; 	 // 10^-3 seconds
    int _microseconds; // 10^-6 seconds
    int _nanoseconds; // 10^-9 seconds
    int _picoseconds; // 10^-12 seconds
    int _femtoseconds; // 10^-15 seconds

    void add_hours(int a) {
        this->_hours += a;
    }

    void add_minutes(int a) {
        this->_minutes += a;

        while (this->_minutes >= 60) {
            this->add_hours(1);
            this->_minutes -= 60;
        }

        while (this->_minutes < 0) {
            this->add_hours(-1);
            this->_minutes += 60;
        }
    }

    void add_seconds(int a) {
        this->_seconds += a;

        while (this->_seconds >= 60) {
            this->add_minutes(1);
            this->_seconds -= 60;
        }

        while (this->_seconds < 0) {
            this->add_minutes(-1);
            this->_seconds += 60;
        }
    }

    void add_milliseconds(int a) {
        this->_milliseconds += a;

        while (this->_milliseconds >= 1000) {
            this->add_seconds(1);
            this->_milliseconds -= 1000;
        }

        while (this->_milliseconds < 0) {
            this->add_seconds(-1);
            this->_milliseconds += 1000;
        }
    }

    void add_microseconds(int a) {
        this->_microseconds += a;

        while (this->_microseconds >= 1000) {
            this->add_milliseconds(1);
            this->_microseconds -= 1000;
        }

        while (this->_microseconds < 0) {
            this->add_milliseconds(-1);
            this->_microseconds += 1000;
        }
    }

    void add_nanoseconds(int a) {
        this->_nanoseconds += a;

        while (this->_nanoseconds >= 1000) {
            this->add_microseconds(1);
            this->_nanoseconds -= 1000;
        }

        while (this->_nanoseconds < 0) {
            this->add_microseconds(-1);
            this->_nanoseconds += 1000;
        }
    }

    void add_picoseconds(int a) {
        this->_picoseconds += a;

        while (this->_picoseconds >= 1000) {
            this->add_nanoseconds(1);
            this->_picoseconds -= 1000;
        }

        while (this->_picoseconds < 0) {
            this->add_nanoseconds(-1);
            this->_picoseconds += 1000;
        }
    }

    void add_femtoseconds(int a) {
        this->_femtoseconds += a;

        while (this->_femtoseconds >= 1000) {
            this->add_picoseconds(1);
            this->_femtoseconds -= 1000;
        }

        while (this->_femtoseconds < 0) {
            this->add_picoseconds(-1);
            this->_femtoseconds += 1000;
        }
    }

    static bool deepView(bool togle) {
        static bool deep_precision = false;
        if (togle)
            deep_precision = !deep_precision;
        return deep_precision;
    }

public:

    static bool deep_precision;

    /******************************************/
    /************** CONSTRUCTORS **************/
    /******************************************/

    NDTime() { // default constructor

        this->resetToZero();
    }

    NDTime(const NDTime& o) { // copy constructor
        _inf = o._inf;
        _possitive = o._possitive;
        _hours = o._hours;
        _minutes = o._minutes;
        _seconds = o._seconds;
        _milliseconds = o._milliseconds;
        _microseconds = o._microseconds;
        _nanoseconds = o._nanoseconds;
        _picoseconds = o._picoseconds;
        _femtoseconds = o._femtoseconds;
    }

    NDTime(std::initializer_list<int> a_args) { // variable unit amount specification constructor

        this->resetToZero();
        int a = 0;

        for (auto i: a_args) {

            switch(a) {
                case 0:
                    this->_hours = abs(i);
                    this->_possitive = (i >= 0);
                    break;
                case 1:
                    if (this->_hours == 0) assert(abs(i) < 60);
                    else assert(0 <= i && i < 60);
                    this->_minutes = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 2:
                    if (this->_minutes == 0) assert(abs(i) < 60);
                    else assert(0 <= i && i < 60);
                    this->_seconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 3:
                    if (this->_seconds == 0) assert(abs(i) < 1000);
                    else assert(0 <= i && i < 1000);
                    this->_milliseconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 4:
                    if (this->_milliseconds == 0) assert(abs(i) < 1000);
                    else assert(0 <= i && i < 1000);
                    this->_microseconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 5:
                    if (this->_microseconds == 0) assert(abs(i) < 1000);
                    else assert(0 <= i && i < 1000);
                    this->_nanoseconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 6:
                    if (this->_nanoseconds == 0) assert(abs(i) < 1000);
                    else assert(0 <= i && i < 1000);
                    this->_picoseconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
                case 7:
                    if (this->_picoseconds == 0) assert(abs(i) < 1000);
                    else assert(0 <= i && i < 1000);
                    this->_femtoseconds = abs(i);
                    this->_possitive = (i >= 0) && this->_possitive;
                    break;
            }
            a++;
        }
    }

    // valid options are "inf", "infinity", "-inf" "-infinity" and hh:mm:ss:mmss:mcs:nnss:ppss:ffss
    // NOTE: it is not necessary to specify all the units.
    NDTime(const std::string a) {
        int v;
        unsigned i;
        std::vector<std::string> strs;

        this->resetToZero();

        if (a == "inf" || a == "infinity") {
            this->_inf = true;
            this->_possitive = true;
        } else if (a == "-inf" || a == "-infinity") {
            this->_inf = true;
            this->_possitive = false;
        } else {
            //boost::split(strs,a,boost::is_any_of(":"));

            char space_char = ':';
            stringstream sstream(a);
            string word;
            while (std::getline(sstream, word, space_char)) {
            	words.push_back(word);
            }

            for(i=0; i<strs.size(); ++i) {
                v = std::stoi(strs[i]);
                switch(i) {
                    case 0:
                        this->_possitive = v >= 0;
                        this->_hours = abs(v);
                        break;
                    case 1: assert(v >= 0 && v < 60); this->_minutes = v; break;
                    case 2: assert(v >= 0 && v < 60); this->_seconds = v; break;
                    case 3: assert(v >= 0 && v < 1000); this->_milliseconds = v; break;
                    case 4: assert(v >= 0 && v < 1000); this->_microseconds = v; break;
                    case 5: assert(v >= 0 && v < 1000); this->_nanoseconds = v; break;
                    case 6: assert(v >= 0 && v < 1000); this->_picoseconds = v; break;
                    case 8: assert(v >= 0 && v < 1000); this->_femtoseconds = v; break;
                }
            }
        }
    }

    static void startDeepView() {
        if (!NDTime::deepView(false))
            NDTime::deepView(true);
    }

    static void stopDeepView() {
        if (NDTime::deepView(false))
            NDTime::deepView(true);
    }

    static NDTime infinity() noexcept {
        return NDTime("inf");
    }

    static NDTime minus_infinity() noexcept {
        return NDTime("-inf");
    }

    static NDTime zero() noexcept {
        return NDTime({0});
    }

    void resetToZero() {
        this->_inf = false;
        this->_possitive = true;
        this->_hours = 0;
        this->_minutes = 0;
        this->_seconds = 0;
        this->_milliseconds = 0;
        this->_microseconds = 0;
        this->_nanoseconds = 0;
        this->_picoseconds = 0;
        this->_femtoseconds = 0;
    }

    NDTime& operator+=(const NDTime& o) noexcept {
        bool this_positive, o_positive, greater_positive;
        NDTime greater, lower;
        NDTime other = o;

        if (this->_inf && other._inf) {
            if (this->_possitive != other._possitive) {
                this->resetToZero();
            }

        } else if (other._inf) {
            this->resetToZero();
            this->_inf = true;
            this->_possitive = other._possitive;

        } else if (this->_possitive != other._possitive) {

            this_positive = this->_possitive;
            o_positive = other._possitive;

            this->_possitive = other._possitive = true;

            if (*this == other) {
                this->resetToZero();
                return *this;

            } else if (*this > other) {
                greater_positive = this_positive;
                greater = *this;
                lower = other;
            } else {
                greater_positive = o_positive;
                greater = other;
                lower = *this;
            }

            greater.add_hours(-1 * lower._hours);
            greater.add_minutes(-1 * lower._minutes);
            greater.add_seconds(-1 * lower._seconds);
            greater.add_milliseconds(-1 * lower._milliseconds);
            greater.add_microseconds(-1 * lower._microseconds);
            greater.add_nanoseconds(-1 * lower._nanoseconds);
            greater.add_picoseconds(-1 * lower._picoseconds);
            greater.add_femtoseconds(-1 * lower._femtoseconds);

            greater._possitive = greater_positive;
            *this = greater;
        } else {

            this->add_hours(other._hours);
            this->add_minutes(other._minutes);
            this->add_seconds(other._seconds);
            this->add_milliseconds(other._milliseconds);
            this->add_microseconds(other._microseconds);
            this->add_nanoseconds(other._nanoseconds);
            this->add_picoseconds(other._picoseconds);
            this->add_femtoseconds(other._femtoseconds);

        }

        return *this;
    }

    NDTime& operator-=(const NDTime& o) noexcept {
        NDTime minus_o = o;
        minus_o._possitive = !o._possitive;
        *this += minus_o;
        return *this;
    }

    NDTime operator+(const NDTime& rhs) const {
        NDTime res = *this;
        res += rhs;
        return res;
    }

    NDTime operator-(const NDTime& rhs) const {
        NDTime res = *this;
        res -= rhs;
        return res;
    }

    bool operator==(const NDTime& rhs) const {
        if (this->_possitive != rhs._possitive) return false;
        else if (this->_inf && rhs._inf) return true;
        else if (this->_inf || rhs._inf) return false;
        else {
            bool res = true;
            res = res && (this->_hours == rhs._hours);
            res = res && (this->_minutes == rhs._minutes);
            res = res && (this->_seconds == rhs._seconds);
            res = res && (this->_milliseconds == rhs._milliseconds);
            res = res && (this->_microseconds == rhs._microseconds);
            res = res && (this->_nanoseconds == rhs._nanoseconds);
            res = res && (this->_picoseconds == rhs._picoseconds);
            res = res && (this->_femtoseconds == rhs._femtoseconds);
            return res;
        }
    }

    bool operator!=(const NDTime& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const NDTime& rhs) const {
        if (*this == rhs) return false;
        else if (*this == NDTime::infinity() || rhs == NDTime::minus_infinity()) return false;
        else if (rhs == NDTime::infinity() || *this == NDTime::minus_infinity()) return true;
        else if (this->_possitive != rhs._possitive) return !this->_possitive;
        else if (this->_possitive) {

            if (this->_hours < rhs._hours) return true;
            else if (this->_hours > rhs._hours) return false;

            if (this->_minutes < rhs._minutes) return true;
            else if (this->_minutes > rhs._minutes) return false;

            if (this->_seconds < rhs._seconds) return true;
            else if (this->_seconds > rhs._seconds) return false;

            if (this->_milliseconds < rhs._milliseconds) return true;
            else if (this->_milliseconds > rhs._milliseconds) return false;

            if (this->_microseconds < rhs._microseconds) return true;
            else if (this->_microseconds > rhs._microseconds) return false;

            if (this->_nanoseconds < rhs._nanoseconds) return true;
            else if (this->_nanoseconds > rhs._nanoseconds) return false;

            if (this->_picoseconds < rhs._picoseconds) return true;
            else if (this->_picoseconds > rhs._picoseconds) return false;

            if (this->_femtoseconds < rhs._femtoseconds) return true;
            else if (this->_femtoseconds > rhs._femtoseconds) return false;

            assert(false); // Should never reach this code
        } else {

            if (this->_hours > rhs._hours) return true;
            else if (this->_hours < rhs._hours) return false;

            if (this->_minutes > rhs._minutes) return true;
            else if (this->_minutes < rhs._minutes) return false;

            if (this->_seconds > rhs._seconds) return true;
            else if (this->_seconds < rhs._seconds) return false;

            if (this->_milliseconds > rhs._milliseconds) return true;
            else if (this->_milliseconds < rhs._milliseconds) return false;

            if (this->_microseconds > rhs._microseconds) return true;
            else if (this->_microseconds < rhs._microseconds) return false;

            if (this->_nanoseconds > rhs._nanoseconds) return true;
            else if (this->_nanoseconds < rhs._nanoseconds) return false;

            if (this->_picoseconds > rhs._picoseconds) return true;
            else if (this->_picoseconds < rhs._picoseconds) return false;

            if (this->_femtoseconds > rhs._femtoseconds) return true;
            else if (this->_femtoseconds < rhs._femtoseconds) return false;

        }
        assert(false); // Should never reach this code
    }

    bool operator>(const NDTime& rhs) const {
        return !(*this < rhs || *this == rhs);
    }

    bool operator<=(const NDTime& rhs) const {
        return !(*this > rhs);
    }

    bool operator>=(const NDTime& rhs) const {
        return !(*this < rhs);
    }

  	int getHours() const {
  		return this->_hours;
  	}

  	int getMinutes() const {
  		return this->_minutes;
  	}

  	int getSeconds() const {
  		return this->_seconds;
  	}

  	int getMilliseconds() const {
  		return this->_milliseconds;
  	}

  	int getMicroseconds() const {
  		return this->_microseconds;
  	}

  	int getNanoseconds() const {
  		return this->_nanoseconds;
  	}

  	int getPicoseconds() const {
  		return this->_picoseconds;
  	}

  	int getFemtoseconds() const {
  		return _femtoseconds;
  	}

    // a useful general-purpose accessor
  	auto as_tuple() const {
        return std::make_tuple(_inf, _possitive, _hours, _minutes, _seconds, _milliseconds,
                               _microseconds, _nanoseconds, _picoseconds, _femtoseconds);
    }

    friend std::ostream& operator<<(std::ostream& os, const NDTime& t);
    friend std::istream& operator>>(std::istream& is, NDTime& t);
};

inline std::ostream& operator<<(std::ostream& os, const NDTime& t) {

    if (!t._possitive) os << "-";
    if (t._inf) {
        os << "inf";
    } else {
        os << ((t._hours < 10) ? "0":"") << t._hours << ":";
        os << ((t._minutes < 10) ? "0":"") << t._minutes << ":";
        os << ((t._seconds < 10) ? "0":"") << t._seconds << ":";
        os << ((t._milliseconds < 10) ? "0":"") << ((t._milliseconds < 100) ? "0":"") << t._milliseconds;
        if (NDTime::deepView(false)) {
            os << ":";
            os << ((t._microseconds < 10) ? "0":"") << ((t._microseconds < 100) ? "0":"") << t._microseconds << ":";
            os << ((t._nanoseconds < 10) ? "0":"") << ((t._nanoseconds < 100) ? "0":"") << t._nanoseconds << ":";
            os << ((t._picoseconds < 10) ? "0":"") << ((t._picoseconds < 100) ? "0":"") << t._picoseconds << ":";
            os << ((t._femtoseconds < 10) ? "0":"") << ((t._femtoseconds < 100) ? "0":"") << t._femtoseconds;
        }
    }
    return os;
}

inline std::istream& operator>>(std::istream& is, NDTime& t) {
    std::string a;
    is >> a;
    t = NDTime(a);
    return is;
}

// specialize numeric_limits
namespace std {
    template<>
    class numeric_limits<NDTime>{
    public:
        static constexpr bool is_specialized = true;
        static NDTime min() noexcept { return NDTime({numeric_limits<int>::min(),59,59,999,999,999,999,999}); }
        static NDTime max() noexcept { return NDTime({numeric_limits<int>::max(),59,59,999,999,999,999,999}); }
        static NDTime lowest() noexcept { return NDTime({numeric_limits<int>::min(),59,59,999,999,999,999,999}); }

        static constexpr int  digits = numeric_limits<int>::digits;
        static constexpr int  digits10 = numeric_limits<int>::digits10;
        static constexpr int  max_digits10 = numeric_limits<int>::max_digits10;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = true;
        static constexpr int radix = 2; // trash_value

        static constexpr int  min_exponent = numeric_limits<int>::min(); // trash_value
        static constexpr int  min_exponent10 = min_exponent/radix; // trash_value
        static constexpr int  max_exponent = numeric_limits<int>::max(); // trash_value
        static constexpr int  max_exponent10 = max_exponent/radix; // trash_value

        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr float_denorm_style has_denorm = denorm_indeterminate;
        static constexpr bool has_denorm_loss = false;
        static NDTime infinity() noexcept { return NDTime::infinity(); }

        static constexpr bool is_iec559 = false;
        static constexpr bool is_bounded = false;
        static constexpr bool is_modulo = false;

        static constexpr bool traps = false;
        static constexpr bool tinyness_before = false;
    };
}

// hash_value implemented in terms of tuple, for consistency and simplicity
std::size_t hash_value(const NDTime& t) {
    //return boost::hash_value(t.as_tuple());
	return std::hash(t.as_tuple());
}

//// specialize hash
//template<>
//struct std::hash<NDTime> : boost::hash<NDTime> {};

#endif
