/**
 * Double Time class for Discrete Event Simulation purposes
 * Copyright (C) 2022  Guillermo G. Trabes
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

#ifndef TIME_DOUBLE
#define TIME_DOUBLE

class Double : public Time {

private:
    Double time;

public:

    Double operator+(const Double& t) override {
    	Double result;
    	result = this->time + t;
    	return result;
    }

    Double operator-(const Double& t) override {
    	Double result;
    	result = this->time - t;
    	return result;
    }

    bool operator==(const Double& t) override {
    	bool result;
    	if(this.time == t.time){
    		result = true;
    	} else {
    		result = false;
    	}
    	return result;
    }

    Double operator+=(const Double& t) override {
    	Double result;
    	result = this->time + t;
    	return result;Double operator+(const Double& t) override {
            Double result;
            result = this->time + t;
            return result;
        }
    }

    Double operator-(const Double& t) override {
    	Double result;
    	result = this->time - t;
        return res;
    }

    void assing_infinity(){

    }




    Double& operator+( t) {
            this->_hours += a;
    }

    Box operator+(const Box& b) {
             Box box;
             box.length = this->length + b.length;
             box.breadth = this->breadth + b.breadth;
             box.height = this->height + b.height;
             return box;
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


    Double& operator+=(Time& t){
    	;
    }

    virtual Time& operator-=(const Time& o) noexcept = 0;

    virtual Time operator+(const Time& rhs) const = 0;

    virtual operator-(const Time& rhs) const = 0;

    virtual bool operator==(const Time& rhs) const = 0;

    virtual bool operator!=(const Time& rhs) const = 0;

    virtual bool operator<(const Time& rhs) const = 0;

    virtual bool operator>(const NDTime& rhs) const = 0;

    virtual bool operator<=(const NDTime& rhs) const = 0;

    virtual bool operator>=(const NDTime& rhs) const = 0;

    void set_to_infinity(){
    	this->infinity=true;

    }


}

#endif
