/*** This code is currently for testing purposes only! ***/

#include "alert.h"

using namespace Alert;

Warning::Warning() : loops_since_detection(0), updates(0), angle(0), distance(-1), sensor_readings(NULL), triggered_sensors(NULL)
{}

Warning::Warning(int angle, int* sensor_readings,
                 bool* triggered_sensors, bool drop) : loops_since_detection(0), updates(0), angle(angle*M_PI/180.0), sensor_readings(sensor_readings), triggered_sensors(triggered_sensors)
{
    // set critical levels and determine warning type
    
    setWarningInfo(drop);
    
}

void Warning::set(int* sensor_readings, bool* triggered_sensors, bool drop)
{
    this->sensor_readings = sensor_readings;
    this->triggered_sensors = triggered_sensors;
    
    // redetermine critical level and warning type
    
    setWarningInfo(drop);
    
    updates++;
}

void Warning::set(int* sensor_readings, bool drop)
{
    this->sensor_readings = sensor_readings;
    
    // redetermine critical level and warning type
    
    setWarningInfo(drop);
    
    updates++;
}

double Warning::calculate_v_distance(int sensor)
{
    if (sensor != hip) {
        return -1;
    }
    else
    {
        int distance = sensor_readings[sensor];
        double vertical_dist = (double) (sin(angle) * distance);
        return vertical_dist;
    }
    
}

double Warning::calculate_h_distance(int sensor)
{
    int distance = sensor_readings[sensor];
    double horizontal_dist = (double) (cos(angle) * distance);
    return horizontal_dist;
}

void Warning::print_warning()
{
    if(sensor_readings != NULL)
    {
        Serial.println("**Warning**");
        Serial.print("Loops since detection: ");
        Serial.println((int)loops_since_detection);
        Serial.print("Updates: ");
        Serial.println(updates);
        Serial.print("Critical Level: ");
        
        switch (critical_level) {
            case low:
                Serial.println("low");
                break;
            case medium:
                Serial.println("medium");
                break;
            case high:
                Serial.println("high");
                break;
            default:
                break;
        }
        
        Serial.println("Sensor:        Distance:");
        Serial.println("Head Left        " + (String)sensor_readings[head_left]);
        Serial.println("Head Right       " + (String)sensor_readings[head_right]);
        Serial.println("Hip              " + (String)sensor_readings[hip]);
        Serial.print("Warning Type: ");
        
        switch (warning_type) {
            case high_obstacle:
                Serial.println("High Obstacle");
                break;
            case low_obstacle:
                Serial.println("Low Obstacle");
                break;
            case unknown:
                Serial.println("Unknown");
                break;
            case high_and_low:
                Serial.println("High and Low");
                break;
            case drop_off:
                Serial.println("Drop off");
                break;
            default:
                break;
        }
        
        Serial.print("Direction: ");
        
        switch (direction) {
            case left:
                Serial.println("Left");
                break;
            case right:
                Serial.println("Right");
                break;
            case left_and_right:
                Serial.println("Left and Right");
                break;
            case forward:
                Serial.println("Forward");
                break;
            default:
                break;
        }
        
        Serial.print("Displayed Distance: ");

        switch (distance) {
            case 0:
                Serial.println("0ft");
                break;
            case 1:
                Serial.println("1ft");
                break;
            case 2:
                Serial.println("2ft");
                break;
            case 3:
                Serial.println("3ft");
                break;
            case 4:
                Serial.println("4ft");
                break;
            case 5:
                Serial.println("5ft");
                break;
            case 6:
                Serial.println("6ft");
                break;
            case 7:
                Serial.println("7ft");
                break;
            case 8:
                Serial.println("8ft");
                break;
            case 9:
                Serial.println("9ft");
                break;
            default:
                Serial.println("Distant");
                break;
        }

     
        Serial.println();
    }
    
}


void Warning::transmit_warning()
{
    Serial.println("Transmitting...");
    Wire.begin();
    Wire.beginTransmission(8);
    switch (warning_type) {
        case high_obstacle:
            Wire.write('H');
            break;
        case low_obstacle:
            Wire.write('L');
            break;
        case high_and_low:
            Wire.write('B');
            break;
        case drop_off:
            Wire.write('D');
            break;
        default:
            Wire.write('U');
            break;
    }
    switch (distance) {
        case d0:
            Wire.write('0');
            break;
        case d1:
            Wire.write('1');
            break;
        case d2:
            Wire.write('2');
            break;
        case d3:
            Wire.write('3');
            break;
        case d4:
            Wire.write('4');
            break;
        case d5:
            Wire.write('5');
            break;
        case d6:
            Wire.write('6');
            break;
        case d7:
            Wire.write('7');
            break;
        case d8:
            Wire.write('8');
            break;
        case d9:
            Wire.write('9');
            break;
        default:
            Wire.write('D');
            break;
    }
    switch (direction) {
        case left:
            Wire.write('L');
            break;
        case right:
            Wire.write('R');
            break;
        case left_and_right:
            Wire.write('B');
            break;
        default:
            Wire.write('U');
            break;
    }

    Wire.endTransmission();
    Serial.println("Ending Transmission...");

}


int Warning::get_distance(int sensor)
{
    return sensor_readings[sensor];
}

bool* Warning::get_triggered_sensors()
{
    return triggered_sensors;
}

int* Warning::get_sensor_readings()
{
    return sensor_readings;
}

unsigned long long Warning::get_loops()
{
    return loops_since_detection;
}

void Warning::update_loops()
{
    if (sensor_readings != NULL) {
        loops_since_detection++;
    }
}

void Warning::increment_updates()
{
    updates++;
}

int Warning::get_updates()
{
    return updates;
}

int Warning::get_critical()
{
    return critical_level;
}

int Warning::get_warning_type()
{
    return warning_type;
}

int Warning::get_distance()
{
    return distance;
}

void Warning::setWarningInfo(bool drop)
{
    // set type and critical level

    double h_hip_dist = calculate_h_distance(hip);
    
    if(sensor_readings[head_left] < 24 || sensor_readings[head_right] < 24 || sensor_readings[hip] < 24)
    {
        critical_level = high;
    }
    else if(sensor_readings[head_left] < 42 || sensor_readings[head_right] < 42 || sensor_readings[hip] < 42)
    {
        critical_level = medium;
    }
    else{
        critical_level = low;
    }
    
    if (drop) {
        warning_type = drop_off;
        critical_level = high;
    }
    
    else{
        
        warning_type = unknown;
        
        if ((triggered_sensors[head_left] || triggered_sensors[head_right]) && triggered_sensors[hip]) {
            warning_type = high_and_low;
        }
        else if((triggered_sensors[head_left] || triggered_sensors[head_right]) && !triggered_sensors[hip])
        {
            warning_type = high_obstacle;
        }
        else if(!triggered_sensors[head_left] && !triggered_sensors[head_right] && triggered_sensors[hip])
        {
            warning_type = low_obstacle;
        }
        
//        if((h_hip_dist - sensor_readings[head_left] > 18 || h_hip_dist - sensor_readings[head_right] > 18) && (triggered_sensors[head_left] || triggered_sensors[head_right]))
//        {
//            warning_type = high_obstacle;
//        }
//        
//        else if(sensor_readings[head_left] - h_hip_dist > 18 && sensor_readings[head_right] - h_hip_dist > 18 && triggered_sensors[hip])
//        {
//            warning_type = low_obstacle;
//        }
        
        
        
        
    }
    
    // set direction
    if (warning_type == drop_off || warning_type == low_obstacle) {
        direction = forward;
    }
    else if(triggered_sensors[head_left] && triggered_sensors[head_right]){
        direction = left_and_right;
    }
    else if(triggered_sensors[head_left])
    {
        direction = left;
    }
    else if(triggered_sensors[head_right])
    {
        direction = right;
    }
    
    //set distance to the smallest sensor reading
    double distance_temp = sensor_readings[0];
    for(int i = 1; i < 3; i++)
    {
        if (sensor_readings[i] < distance_temp) {
            distance_temp = sensor_readings[i];
        }
    }
    distance_temp = (double) (cos(angle) * distance_temp); // convert to horizontal distance
    distance_temp /= 12; // convert to feet
    
    if((int)(distance_temp * 10) % 10 >= 5)
    {
        distance = (int)distance_temp + 1; // round up to the nearest foot
    }
    else
    {
        distance = (int)distance_temp; // round down distance to nearest foot
    }
    
    
    switch (distance) {
        case 0:
            distance = d0;
            break;
        case 1:
            distance = d1;
            break;
        case 2:
            distance = d2;
            break;
        case 3:
            distance = d3;
            break;
        case 4:
            distance = d4;
            break;
        case 5:
            distance = d5;
            break;
        case 6:
            distance = d6;
            break;
        case 7:
            distance = d7;
            break;
        case 8:
            distance = d8;
            break;
        case 9:
            distance = d9;
            break;
        default:
            distance = distant;
            break;
    }
    
}