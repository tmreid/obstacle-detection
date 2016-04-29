/*** This code is currently for testing purposes only! ***/

#ifndef alert_h
#define alert_h

#include <Arduino.h>
#include <Wire.h>

namespace Alert {
    
    enum Sensor {head_left, head_right, hip};
    enum Critical {low, medium, high};
    enum Warning_Type {low_obstacle, high_obstacle, drop_off, high_and_low, unknown};
    enum Direction {left, right, forward, left_and_right};
    enum Distance {d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, distant};
    
    class Warning{
        
    private:
        unsigned long long loops_since_detection;
        double angle;
        bool* triggered_sensors;
        int* sensor_readings;
        int critical_level;
        int warning_type;
        int direction;
        int distance;
        int updates;
        void setWarningInfo(bool drop);
        
    public:
        Warning();
        Warning(int angle, int* sensor_readings, bool* triggered_sensors, bool drop);
        void set (int* sensor_readings, bool* triggered_sensors, bool drop);
        void set (int* sensor_readings, bool drop);
        void set (unsigned long long loops_since_detection, int angle, int** sensor_readings, int* triggered_sensors, bool drop);
        double calculate_v_distance(int sensor); // calculates vertical distance of a sensor in the warning
        double calculate_h_distance(int sensor); // calculates horizontal distance of a sensor in the warning
        void print_warning();
        void transmit_warning();
        unsigned long long get_loops();
        int get_distance(int sensor);
        bool* get_triggered_sensors();
        int* get_sensor_readings();
        void update_loops();
        void increment_updates();
        int get_updates();
        int get_critical();
        int get_distance();
        int get_warning_type();
    };
    
}


#endif