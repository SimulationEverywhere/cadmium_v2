/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Jon
 * Copyright (c) 2023-present Román Cárdenas Rodríguez
 */

/**
* Jon Menard
* ARSLab - Carleton University
*
* Digital Input:
* Model to interface with a digital Input pin for Embedded Cadmium.
*/



#ifndef RT_DIGITALPIN_HPP
#define RT_DIGITALPIN_HPP



#include <iostream>
#include <optional>

#include <thread>
#include <mutex>
#include <chrono>
#include <windows.h>
#include <stdio.h>
#include <string.h>


using namespace std;
namespace cadmium {
  
  class DigitalPin {
    mutable std::mutex input_mutex;
    mutable bool keyPressed;

    public:
    DigitalPin(){
      std::unique_lock<std::mutex> mutex_lock(input_mutex, std::defer_lock);
      mutex_lock.lock();
      keyPressed = false;
      // _keyCode = keyCode;
      mutex_lock.unlock();
      std::thread(&DigitalPin::listen, this).detach();
    };

    bool checkPin(){
      bool pin;
      std::unique_lock<std::mutex> mutex_lock(input_mutex, std::defer_lock);
      mutex_lock.lock();
      pin = keyPressed;
      keyPressed = false;
      
      mutex_lock.unlock();

      return pin;
    }

    void listen(){
      std::unique_lock<std::mutex> mutex_lock(input_mutex, std::defer_lock);
      string str;
      while(1){
        getline(cin, str);
        if(str == ""){
          mutex_lock.lock();
          keyPressed = true;
          mutex_lock.unlock();
        }
      }
    }

  };
} // namespace cadmium

#endif // RT_DIGITALPIN_HPP