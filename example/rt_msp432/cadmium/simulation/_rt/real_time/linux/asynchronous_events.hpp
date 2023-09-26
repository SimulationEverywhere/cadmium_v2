/**
 * Copyright (C) 2022  Jon Menard
 * ARSLab - Carleton University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef CADMIUM_ASYNCHRONOUS_EVENT_HPP
#define CADMIUM_ASYNCHRONOUS_EVENT_HPP

#include "cadmium/core/modeling/port.hpp"
namespace cadmium {
        
        
        class AsyncEvent{
        public:
            
            Port<bool> triggerExternal;
            bool interrupted = false;

            std::vector <class AsyncEventObserver *> views;
            AsyncEvent(){

            }

            void setPort(Port<bool> & port){
                triggerExternal = port;
            }

            Port<bool> & getPort(){
                return triggerExternal;
            }

            void attach(AsyncEventObserver *obs) {
                views.push_back(obs);
            }
            
            void notify();
            
        };

        class AsyncEventObserver {
                std::vector<std::shared_ptr<AsyncEvent>>  asyncSubjects;
                
            public:
                AsyncEventObserver(std::vector<std::shared_ptr<AsyncEvent>> subs) {
                    for(auto s : subs){
                    	s->attach(this);
                        asyncSubjects.push_back(s);
                    } 

                }

                std::vector<std::shared_ptr<AsyncEvent>>& getAsyncSubjects() {
        	        return asyncSubjects;
                }
                
                virtual void update() = 0;

            protected:
                bool interrupted;
            };

}
#endif // CADMIUM_ASYNCHRONOUS_EVENT_HPP