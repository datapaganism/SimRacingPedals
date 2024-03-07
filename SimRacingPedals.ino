#include <Joystick.h>
#include <HX711.h>

#define calibration_factor 2000  // Brake Loadcell: change this value to increase or decrease the range of the brake
#define dt 20
#define sck 21

const size_t multisampling = 1;
const size_t adc_bits = 12;
const int adc_max = (1 << adc_bits) -1;

const int joystick_min_range = 0;
const int joystick_max_range = (1 << 10) -1;
const int brake_load_cell_max_reading = joystick_max_range;


char buffer[50] = {0,};


HX711 load_cell;

enum PEDALS {
  throttle = 26,
  clutch = 27,
};

int clutch_init_reading = 0;
int throttle_init_reading = 0;


void setup() {
  Serial.begin(115200);

  analogReadResolution(adc_bits);

  pinMode(throttle, INPUT);
  pinMode(clutch, INPUT);

  load_cell.begin(dt, sck);
  load_cell.set_scale(calibration_factor);
  load_cell.tare();

  clutch_init_reading = analogRead(PEDALS::clutch);
  throttle_init_reading = analogRead(PEDALS::throttle);

  Joystick.begin();
  Joystick.use8bit(false);
  Joystick.useManualSend(true);

}


int readAnalogPedal(const PEDALS pin, const bool invert, const char print_char, const int init_reading = 0) {
  int read_raw = 0;

  for (size_t i = 0; i < multisampling; i++) {
    read_raw += analogRead(pin);
  }
  read_raw /= multisampling;

  if (init_reading > 0)
  {
    read_raw -= init_reading;
    if (read_raw < 0)
    {
      read_raw = 0;
    }
  }

  long map_begin = 0;
  long map_end = adc_max;

  if (invert == true) {
    map_begin = adc_max;
    map_end = 0;
  }
  int read_scaled = map(read_raw, map_begin, map_end, joystick_min_range, joystick_max_range);

  sprintf(buffer, "%c: %i\t %i\n", print_char, read_raw, read_scaled);
  // Serial.print(buffer);

  return read_scaled;
}

int readBrakePedal() {
  int read_raw = 0;

  // for (size_t i = 0; i < multisampling; i++) {
    read_raw += -load_cell.get_units();
  // }
  // read_raw /= multisampling;
 
  if (read_raw < 1) {
    read_raw = 0;
  }
  int read_scaled = map(read_raw, 0, brake_load_cell_max_reading, joystick_min_range, joystick_max_range);

  sprintf(buffer, "B: %i\t %i\n", read_raw, read_scaled);
  // Serial.print(buffer);

  return read_scaled;
}


void loop() {
  

  Joystick.Z(readAnalogPedal(PEDALS::throttle, false, 'A', throttle_init_reading));
  Joystick.Zrotate(readBrakePedal());
  Joystick.slider(readAnalogPedal(PEDALS::clutch, false, 'C', clutch_init_reading));

  Joystick.send_now();
}
