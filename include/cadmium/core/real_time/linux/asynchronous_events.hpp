
#ifndef CADMIUM_ASYNCHRONOUS_EVENT_HPP
#define CADMIUM_ASYNCHRONOUS_EVENT_HPP

#include <cadmium/core/modeling/port.hpp>
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
                // std::vector <class AsyncEvent *> getSubject() {
                //     return sub;
                // }
            };

}
#endif // CADMIUM_ASYNCHRONOUS_EVENT_HPP