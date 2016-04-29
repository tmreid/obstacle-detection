/*** This code is currently for testing purposes only! ***/

#include <alert.h>
#include <Wire.h>

using namespace Alert;

#define ARRAY_SIZE 10

const int anPin1 = 0; // head left
const int anPin2 = 1; // head right
const int anPin3 = 2; // hip

double calc_v_distance(int distance, int angle);        // calculates vertical distance given total distance and angle of sensor
double calc_h_distance(int distance, int angle);        // calculates horizontal distance given total distance and angle of sensor
double calc_running_avg(int* readings, int num);        // calculates the average value of a group of sensor readings
void read_sensors();                                    // gets sensor readings
void send_warning();                                    // sends next warning
void increment_loops();                                 // updates loop counter in all warnings

int distance_hl = 0;                                    // left head sensor distance
int distance_hr = 0;                                    // right head sensor distance
int distance_hp = 0;                                    // hip sensor distance
int current_readings_index = 0;
int warnings_index = 0;

const int head_threshold = 50;                          // distances below threshold will trigger a warning
const int low_obstacle_threshold = 12;                  // objects closer than this distance away from the original floor height will trigger a warning
const int floor_change_threshold = 3;                   // floor height changes larger than threshold will trigger a warning
const int angle = 40;                                   // hip sensor angle below horizontal 
const int min_updates = 6;                              // specifies the number of times a warning must be updated before it can be output
double avg_floor_height = 0;
double original_avg_floor = 0;
double original_avg_floor_v = 0;

int current_readings[ARRAY_SIZE];                       // keeps track of hip sensor readings
Warning generated_warnings[ARRAY_SIZE];                 // list of current warnings in system

bool first_time_setup = true;

void setup() {

    Serial.begin(9600);
    Wire.begin();
}

void loop() {

  if(first_time_setup)
  {
    first_time_setup = false;

    Serial.println("Reading average floor height: ");

    // tell audio controller to output startup message
    Serial.println("Transmitting...");
    Wire.beginTransmission(8);
    Wire.write('S');
    Wire.endTransmission();

    for(int i = 0; i < 50; i++)
    {
        read_sensors();

        current_readings[i % ARRAY_SIZE] = distance_hp;

        delay(150);
    }
    
    // calculate avg floor height (leave as diagonal distance)
    avg_floor_height = calc_running_avg(current_readings, ARRAY_SIZE);
    original_avg_floor = avg_floor_height;
    original_avg_floor_v = calc_v_distance(original_avg_floor, angle);

    Serial.println("Average Floor Height: " + (String)original_avg_floor_v);

    // tell audio controller to output ending startup message
    Wire.beginTransmission(8);
    Wire.write('W');
    Wire.endTransmission();
    Serial.println("Ending Transmission...");

    delay(500);
    
    Serial.println("Starting Sensors:");
  }

  read_sensors();

  current_readings[current_readings_index] = distance_hp;
  
  if(current_readings_index < ARRAY_SIZE - 1)
  {
      current_readings_index++;
  }
  else
  {
      current_readings_index = 0;
  }
  
  bool is_drop_off = false;
  if((calc_v_distance(distance_hp, angle) - original_avg_floor_v) > floor_change_threshold)
  {
    is_drop_off = true;
  }
        
  // if any (horizontal) sensor reading is less than the threshold or if the
  // difference between current floor height and the original average floor height
  // is greater than the accepted threshold, create a warning
  if(distance_hl < head_threshold || distance_hr < head_threshold 
                             || calc_h_distance(distance_hp, angle) < (calc_h_distance(original_avg_floor, angle) - low_obstacle_threshold)
                             || is_drop_off)
  {
      // map the sensors to their current reading
      int m[3] = {distance_hl, distance_hr, distance_hp};
      
      // check which sensors are detecting objects
      bool triggered_sensors[3] = {false, false, false};
      
      if(distance_hl < head_threshold)
      {
          triggered_sensors[head_left] = true;
      }
      if(distance_hr < head_threshold)
      {
          triggered_sensors[head_right] = true;
      }
      if(calc_h_distance(distance_hp, angle) < (calc_h_distance(original_avg_floor, angle) - low_obstacle_threshold) || is_drop_off)
      {
          triggered_sensors[hip] = true;
      }
      
      // Check if a warning with the same triggered sensors already exists
      // and if it was created recently
      // If it was, then update that warning's update counter and readings, 
      // otherwise create a new warning
      bool warning_exists = false;
      for (int i = 0; i < ARRAY_SIZE; i++)
      {
          if(generated_warnings[i].get_triggered_sensors() != NULL)
          {
              bool* ts = generated_warnings[i].get_triggered_sensors();
              unsigned long long loops_since_detection = generated_warnings[i].get_loops();
              
              if (ts[head_left] == triggered_sensors[head_left] && 
                  ts[head_right] == triggered_sensors[head_right] && 
                  ts[hip] == triggered_sensors[hip] && 
                  loops_since_detection < 50)
              {
                  warning_exists = true;
                  generated_warnings[i].set(m, is_drop_off);
                  break;
              }
          }
      }
      
      if(!warning_exists)
      {
          generated_warnings[warnings_index] = Warning(angle, m, triggered_sensors, is_drop_off);

          if(warnings_index < ARRAY_SIZE - 1)
          {
            warnings_index++;
          }
          else{
            warnings_index = 0;
          }
      }     
  }

  // output the most recently created warning that has the 
  // highest critical level and has been updated enough times 
  int highest_critical = -1;
  
  for(int i = 0; i < ARRAY_SIZE; i++)
  {
      if (generated_warnings[i].get_sensor_readings() != NULL) {
          if(generated_warnings[i].get_updates() >= min_updates && generated_warnings[i].get_critical() >= highest_critical)
          {
              highest_critical = generated_warnings[i].get_critical();
          }
      }
  }

  int index = -1;
  unsigned long long min_loops = INFINITY;
  
  for(int i = 0; i < ARRAY_SIZE; i++)
  {
      if (generated_warnings[i].get_sensor_readings() != NULL) {
          if(generated_warnings[i].get_updates() >= min_updates && 
             generated_warnings[i].get_loops() <= min_loops && 
             generated_warnings[i].get_critical() == highest_critical)
          {
              index = i;
              min_loops = generated_warnings[i].get_loops();
          }
      }
  }

  // Send Warning
  if(index != -1)
  {
      generated_warnings[index].print_warning();      // FOR DEBUGGING
      generated_warnings[index].transmit_warning();   // send warning to audio controller
      generated_warnings[index] = Warning();          // overwrite with null data
  }

  increment_loops();
  delay(150);
  
} // END MAIN LOOP

double calc_v_distance(int distance, int angle){
    double angle_rad = PI*angle/180.0;
    double vertical_dist = (double) (sin(angle_rad) * distance);
    return vertical_dist;
}

double calc_h_distance(int distance, int angle){
    double angle_rad = PI*angle/180.0;
    double horizontal_dist = (double) (cos(angle_rad) * distance);
    return horizontal_dist;
}

double calc_running_avg(int* readings, int num)
{
    int t = 0;
    
    for (int i = 0; i < num; i++)
    {
        t += current_readings[i];
    }
    
    t /= num;
    
    return t;
}

void read_sensors()
{
    distance_hl = 1000;
    distance_hr = 1000;
    distance_hp = analogRead(anPin3)/2;
}

void increment_loops()
{
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        generated_warnings[i].update_loops();
    }
}
