#include <C:\你的文件地址\servos.h>
#include <BluetoothSerial.h>


BluetoothSerial SerialBT; //Object for Bluetooth


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

/* Servos --------------------------------------------------------------------*/
//define 12 servos for 4 legs
//Servo servo[4][3];
//define servos' ports
const int servo_pin[4][3] = {{0, 1, 2}, {4, 5, 6}, {8, 9, 10}, {12, 13, 14}};
/* Size of the robot ---------------------------------------------------------*/
const float length_a = 55;
const float length_b = 77.5;
const float length_c = 27.5;
const float length_side = 71;
const float z_absolute = -28;
/* Constants for movement ----------------------------------------------------*/
const float z_default = -50, z_up = -30, z_boot = z_absolute;
const float x_default = 62, x_offset = 0;
const float y_start = 0, y_step = 40;
const float y_default = x_default;
/* variables for movement ----------------------------------------------------*/
typedef struct {
    float site_now[4][3];     //real-time coordinates of the end of each leg
    float site_expect[4][3];  //expected coordinates of the end of each leg
    float temp_speed[4][3];   //each axis' speed, needs to be recalculated before each movement
    int32_t FRFoot = 0;
    int32_t FRElbow = 0;
    int32_t FRShdr = 0;
    int32_t FLFoot = 0;
    int32_t FLElbow = 0;
    int32_t FLShdr = 0;
    int32_t RRFoot = 0;
    int32_t RRElbow = 0;
    int32_t RRShdr = 0;
    int32_t RLFoot = 0;
    int32_t RLElbow = 0;
    int32_t RLShdr = 0;
    int32_t rest_counter = 0;  //+1/0.02s, for automatic rest
} service_status_t;

service_status_t sst;

float move_speed = 1.4;            //movement speed
float speed_multiple = 1;          //movement speed multiple
const float spot_turn_speed = 4;
const float leg_move_speed = 8;
const float body_move_speed = 3;
const float stand_seat_speed = 1;
//functions' parameter
const float KEEP = 255;
//define PI for calculation
const float pi = 3.1415926;
/* Constants for turn --------------------------------------------------------*/
//temp length
const float temp_a = sqrt(pow(2 * x_default + length_side, 2) + pow(y_step, 2));
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt(pow(2 * x_default + length_side, 2) + pow(2 * y_start + y_step + length_side, 2));
const float temp_alpha = acos((pow(temp_a, 2) + pow(temp_b, 2) - pow(temp_c, 2)) / 2 / temp_a / temp_b);
//site for turn
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin(temp_alpha) - turn_y1 - length_side;
/* ---------------------------------------------------------------------------*/
String lastComm = "";
boolean print_reach = false;
/*
  - wait one of end points move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_reach(int leg) {
    while (1){
        if(print_reach){
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][0], sst.site_expect[leg][0]);
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][1], sst.site_expect[leg][1]);
            Serial.printf("%i now:%f exp:%f\n", leg, sst.site_now[leg][2], sst.site_expect[leg][2]);
        }
        if (sst.site_now[leg][0] == sst.site_expect[leg][0]){
            if (sst.site_now[leg][1] == sst.site_expect[leg][1]){
                if (sst.site_now[leg][2] == sst.site_expect[leg][2]){
                    break;
                }
            }
        }
    }
}

/*
  - wait all of end points move to expect site
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_all_reach(void) {
    for (int i = 0; i < 4; i++)
        wait_reach(i);
}

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
    double pulselength;
    pulselength = 1000000;  // 1,000,000 us per second
    pulselength /= 60;      // 60 Hz
    pulselength /= 4096;    // 12 bits of resolution
    pulse *= 1000000;       // convert to us
    pulse /= pulselength;
    pwm.setPWM(n, 0, pulse);
}

/*
  - set one of end points' expect site
  - this founction will set temp_speed[4][3] at same time
  - non - blocking function
   ---------------------------------------------------------------------------*/
void set_site(int leg, float x, float y, float z) {
    float length_x = 0, length_y = 0, length_z = 0;

    if (x != KEEP)
        length_x = x - sst.site_now[leg][0];
    if (y != KEEP)
        length_y = y - sst.site_now[leg][1];
    if (z != KEEP)
        length_z = z - sst.site_now[leg][2];

    float length = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));

    sst.temp_speed[leg][0] = length_x / length * move_speed * speed_multiple;
    sst.temp_speed[leg][1] = length_y / length * move_speed * speed_multiple;
    sst.temp_speed[leg][2] = length_z / length * move_speed * speed_multiple;

    if (x != KEEP)
        sst.site_expect[leg][0] = x;
    if (y != KEEP)
        sst.site_expect[leg][1] = y;
    if (z != KEEP)
        sst.site_expect[leg][2] = z;
}

/*
  - is_stand
   ---------------------------------------------------------------------------*/
bool is_stand(void) {
    if (sst.site_now[0][2] == z_default)
        return true;
    else
        return false;
}

/*
  - sit
  - blocking function
   ---------------------------------------------------------------------------*/
void sit(void) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_boot);
    }
    wait_all_reach();
    Serial.println("Body twist right");
}

/*
  - stand
  - blocking function
   ---------------------------------------------------------------------------*/
void stand(void) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) {
        set_site(leg, KEEP, KEEP, z_default);
    }
    wait_all_reach();
    
}

/*
  - Body init
  - blocking function
   ---------------------------------------------------------------------------*/
void b_init(void) {
    //stand();
    set_site(0, x_default, y_default, z_default);
    set_site(1, x_default, y_default, z_default);
    set_site(2, x_default, y_default, z_default);
    set_site(3, x_default, y_default, z_default);
    wait_all_reach();
    Serial.println("Body twist right");
}

/*
  - spot turn to left
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_left(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            //leg 3&1 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start, z_up);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 0&2 move
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_up);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - spot turn to right
  - blocking function
  - parameter step steps wanted to turn
   ---------------------------------------------------------------------------*/
void turn_right(unsigned int step) {
    move_speed = spot_turn_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            //leg 2&0 move
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x0 - x_offset, turn_y0, z_default);
            set_site(1, turn_x1 - x_offset, turn_y1, z_default);
            set_site(2, turn_x0 + x_offset, turn_y0, z_up);
            set_site(3, turn_x1 + x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(2, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_default);
            set_site(1, turn_x1 + x_offset, turn_y1, z_default);
            set_site(2, turn_x0 - x_offset, turn_y0, z_default);
            set_site(3, turn_x1 - x_offset, turn_y1, z_default);
            wait_all_reach();

            set_site(0, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_up);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 1&3 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(0, turn_x1 + x_offset, turn_y1, z_default);
            set_site(1, turn_x0 + x_offset, turn_y0, z_up);
            set_site(2, turn_x1 - x_offset, turn_y1, z_default);
            set_site(3, turn_x0 - x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(1, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(0, turn_x1 - x_offset, turn_y1, z_default);
            set_site(1, turn_x0 - x_offset, turn_y0, z_default);
            set_site(2, turn_x1 + x_offset, turn_y1, z_default);
            set_site(3, turn_x0 + x_offset, turn_y0, z_default);
            wait_all_reach();

            set_site(3, turn_x0 + x_offset, turn_y0, z_up);
            wait_all_reach();

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();

            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - go forward
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_forward(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[2][1] == y_start) {
            //leg 2&1 move
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start, z_default);
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //      leg 0&3 move
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start, z_default);
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

/*
  - go back
  - blocking function
  - parameter step steps wanted to go
   ---------------------------------------------------------------------------*/
void step_back(unsigned int step) {
    move_speed = leg_move_speed;
    while (step-- > 0) {
        if (sst.site_now[3][1] == y_start) {
            //leg 3&0 move
            set_site(3, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(1, x_default + x_offset, y_start, z_default);
            set_site(2, x_default - x_offset, y_start + y_step, z_default);
            set_site(3, x_default - x_offset, y_start + y_step, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(0, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        } else {
            //leg 1&2 move
            set_site(1, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
            wait_all_reach();

            move_speed = body_move_speed;

            set_site(0, x_default - x_offset, y_start + y_step, z_default);
            set_site(1, x_default - x_offset, y_start + y_step, z_default);
            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
            set_site(3, x_default + x_offset, y_start, z_default);
            wait_all_reach();

            move_speed = leg_move_speed;

            set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_up);
            wait_all_reach();
            set_site(2, x_default + x_offset, y_start, z_default);
            wait_all_reach();
        }
    }
}

// add by RegisHsu

void body_left(int i) {
    set_site(0, sst.site_now[0][0] + i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] + i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] - i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] - i, KEEP, KEEP);
    wait_all_reach();
}

void body_right(int i) {
    set_site(0, sst.site_now[0][0] - i, KEEP, KEEP);
    set_site(1, sst.site_now[1][0] - i, KEEP, KEEP);
    set_site(2, sst.site_now[2][0] + i, KEEP, KEEP);
    set_site(3, sst.site_now[3][0] + i, KEEP, KEEP);
    wait_all_reach();
}

void hand_wave(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(2, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, turn_x1, turn_y1, 50);
            wait_all_reach();
            set_site(0, turn_x0, turn_y0, 50);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void hand_shake(int i) {
    float x_tmp;
    float y_tmp;
    float z_tmp;
    move_speed = 1;
    if (sst.site_now[3][1] == y_start) {
        body_right(15);
        x_tmp = sst.site_now[2][0];
        y_tmp = sst.site_now[2][1];
        z_tmp = sst.site_now[2][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(2, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(2, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(2, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_left(15);
    } else {
        body_left(15);
        x_tmp = sst.site_now[0][0];
        y_tmp = sst.site_now[0][1];
        z_tmp = sst.site_now[0][2];
        move_speed = body_move_speed;
        for (int j = 0; j < i; j++) {
            set_site(0, x_default - 30, y_start + 2 * y_step, 55);
            wait_all_reach();
            set_site(0, x_default - 30, y_start + 2 * y_step, 10);
            wait_all_reach();
        }
        set_site(0, x_tmp, y_tmp, z_tmp);
        wait_all_reach();
        move_speed = 1;
        body_right(15);
    }
}

void head_up(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] - i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] + i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] - i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] + i);
    wait_all_reach();
}

void head_down(int i) {
    set_site(0, KEEP, KEEP, sst.site_now[0][2] + i);
    set_site(1, KEEP, KEEP, sst.site_now[1][2] - i);
    set_site(2, KEEP, KEEP, sst.site_now[2][2] + i);
    set_site(3, KEEP, KEEP, sst.site_now[3][2] - i);
    wait_all_reach();
}

void body_dance(int i) {
    float body_dance_speed = 2;
    sit();
    move_speed = 1;
    set_site(0, x_default, y_default, KEEP);
    set_site(1, x_default, y_default, KEEP);
    set_site(2, x_default, y_default, KEEP);
    set_site(3, x_default, y_default, KEEP);
    print_reach = true;
    wait_all_reach();
    stand();
    set_site(0, x_default, y_default, z_default - 20);
    set_site(1, x_default, y_default, z_default - 20);
    set_site(2, x_default, y_default, z_default - 20);
    set_site(3, x_default, y_default, z_default - 20);
    wait_all_reach();
    move_speed = body_dance_speed;
    head_up(30);
    for (int j = 0; j < i; j++) {
        if (j > i / 4)
            move_speed = body_dance_speed * 2;
        if (j > i / 2)
            move_speed = body_dance_speed * 3;
        set_site(0, KEEP, y_default - 20, KEEP);
        set_site(1, KEEP, y_default + 20, KEEP);
        set_site(2, KEEP, y_default - 20, KEEP);
        set_site(3, KEEP, y_default + 20, KEEP);
        wait_all_reach();
        set_site(0, KEEP, y_default + 20, KEEP);
        set_site(1, KEEP, y_default - 20, KEEP);
        set_site(2, KEEP, y_default + 20, KEEP);
        set_site(3, KEEP, y_default - 20, KEEP);
        wait_all_reach();
    }
    move_speed = body_dance_speed;
    head_down(30);
    b_init();
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char *command) {
    Serial.println("What?");
}


void action_cmd (void) {
    char val;
    int  n_step=1; //这里设置默认每次1步
    val = SerialBT.read();
    servos_cmd (val, n_step);
}


void servos_cmd(int action_mode, int n_step) {

    switch (action_mode) {
        case '0':
            Serial.println("Step forward");

            SerialBT.println("forward");

            if (!is_stand())
                stand();
            step_forward(n_step);
            break;

        case '1':
            Serial.println("Step back");

            SerialBT.println("backward");

            if (!is_stand())
                stand();
            step_back(n_step);
            break;

        case '2':
            Serial.println("Turn left");

            SerialBT.println("turn left");

           
            if (!is_stand())
                stand();
            turn_left(n_step);
            break;

        case '3':
            Serial.println("Turn right");

            SerialBT.println("turn right");

            if (!is_stand())
                stand();
            turn_right(n_step);
            break;

        case '4':
            Serial.println("1:up,0:dn");

            SerialBT.println(" sit");

          
                sit();
            break;

        case '5':
            Serial.println("Hand shake");

            SerialBT.println("hand shake");

            hand_shake(n_step);
            break;

        case '6':
            Serial.println("Hand wave");

            SerialBT.println("hand wave");

            hand_wave(n_step);
            break;

        case '7':
            Serial.println("Lets rock baby");

            SerialBT.println("dance");

            body_dance(10);
            break;

        case '8':
            Serial.println("Reset");

            sst.FLElbow = 0;
            sst.FRElbow = 0;
            sst.RLElbow = 0;
            sst.RRElbow = 0;
            sst.FLFoot = 0;
            sst.FRFoot = 0;
            sst.RLFoot = 0;
            sst.RRFoot = 0;
            sst.FLShdr = 0;
            sst.FRShdr = 0;
            sst.RLShdr = 0;
            sst.RRShdr = 0;
            stand();
            break;

        case '9':
            Serial.println("Higher");

            SerialBT.println("higher");

            sst.FLElbow -= 4;
            sst.FRElbow -= 4;
            sst.RLElbow -= 4;
            sst.RRElbow -= 4;
            sst.FLFoot += 4;
            sst.FRFoot += 4;
            sst.RLFoot += 4;
            sst.RRFoot += 4;
            stand();
            break;

        case '10':
            Serial.println("Lower");

            SerialBT.println("lower");

            sst.FLElbow += 4;
            sst.FRElbow += 4;
            sst.RLElbow += 4;
            sst.RRElbow += 4;
            sst.FLFoot -= 4;
            sst.FRFoot -= 4;
            sst.RLFoot -= 4;
            sst.RRFoot -= 4;
            stand();
            break;

        case W_HEAD_UP:
            Serial.println("Head up");

            SerialBT.println("head up");

            sst.FLElbow -= 4;
            sst.FRElbow -= 4;
            sst.RLElbow += 4;
            sst.RRElbow += 4;
            sst.FLFoot += 4;
            sst.FRFoot += 4;
            sst.RLFoot -= 4;
            sst.RRFoot -= 4;
            stand();
            break;

        case W_HEAD_DOWN:
            Serial.println("Head down");

            SerialBT.println("head down");

            sst.FLElbow += 4;
            sst.FRElbow += 4;
            sst.RLElbow -= 4;
            sst.RRElbow -= 4;
            sst.FLFoot -= 4;
            sst.FRFoot -= 4;
            sst.RLFoot += 4;
            sst.RRFoot += 4;
            stand();
            break;

        case W_B_RIGHT:
            Serial.println("body right");

            SerialBT.println("body right");

            if (!is_stand()) stand();
            sst.FLElbow -= 4;
            sst.FRElbow += 4;
            sst.RLElbow -= 4;
            sst.RRElbow += 4;
            sst.FLFoot += 4;
            sst.FRFoot -= 4;
            sst.RLFoot += 4;
            sst.RRFoot -= 4;
            stand();
            break;

        case W_B_LEFT:
            Serial.println("body left");

            SerialBT.println("body left");

            if (!is_stand()) stand();
            sst.FLElbow += 4;
            sst.FRElbow -= 4;
            sst.RLElbow += 4;
            sst.RRElbow -= 4;
            sst.FLFoot -= 4;
            sst.FRFoot += 4;
            sst.RLFoot -= 4;
            sst.RRFoot += 4;
            stand();
            break;

        case W_B_INIT:
            Serial.println("Body init");

            SerialBT.println("body init");

            sit();
            b_init();
            sst.FLElbow = 0;
            sst.FRElbow = 0;
            sst.RLElbow = 0;
            sst.RRElbow = 0;
            sst.FLFoot = 0;
            sst.FRFoot = 0;
            sst.RLFoot = 0;
            sst.RRFoot = 0;
            sst.FLShdr = 0;
            sst.FRShdr = 0;
            sst.RLShdr = 0;
            sst.RRShdr = 0;
            stand();
            break;

        case W_TW_R:
            Serial.println("Body twist right");

            SerialBT.println("twist right");

            sst.FLShdr -= 4;
            sst.FRShdr += 4;
            sst.RLShdr += 4;
            sst.RRShdr -= 4;
            stand();
            break;

        case W_TW_L:
            Serial.println("Body twist left");

            SerialBT.println("twist left");

            sst.FLShdr += 4;
            sst.FRShdr -= 4;
            sst.RLShdr -= 4;
            sst.RRShdr += 4;
            stand();
            break;

        default:
            Serial.println("Error");

            SerialBT.println("error");

            break;
    }
}

/*
  - trans site from cartesian to polar
  - mathematical model 2/2
   ---------------------------------------------------------------------------*/
void cartesian_to_polar(float &alpha, float &beta, float &gamma, float x, float y, float z) {
    //calculate w-z degree
    float v, w;
    w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
    v = w - length_c;
    alpha = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) / 2 / length_a / sqrt(pow(v, 2) + pow(z, 2)));
    beta = acos((pow(length_a, 2) + pow(length_b, 2) - pow(v, 2) - pow(z, 2)) / 2 / length_a / length_b);
    //calculate x-y-z degree
    gamma = (w >= 0) ? atan2(y, x) : atan2(-y, -x);
    //trans degree pi->180
    alpha = alpha / pi * 180.0;
    beta = beta / pi * 180.0;
    gamma = gamma / pi * 180.0;
}

void print_final_PWM (int pin, uint16_t off){
#ifdef TIMER_INTERRUPT_DEBUG
    Serial.printf("[PWM]\tP:%i\toff:%u\n",pin,off);
#endif
}

/*
  - trans site from polar to microservos
  - mathematical model map to fact
  - the errors saved in eeprom will be add
   ---------------------------------------------------------------------------*/
void polar_to_servo(int leg, float alpha, float beta, float gamma) {
    // 定义腿部名称数组（修正顺序）
    const char* legNames[] = {"FR", "RR", "FL", "RL"};
    
    // 输出原始角度
    if(print_reach) {
        String debugMsg = String("[") + legNames[leg] + String("] 原始角度:\n") +
                         "肩关节(gamma): " + String(gamma) + "°\n" +
                         "肘关节(alpha): " + String(alpha) + "°\n" +
                         "腿关节(beta):  " + String(beta) + "°";
        customSerial(debugMsg.c_str());
    }

    float AL, BE, GA;  // 使用与cankao.cpp相同的变量名

    // 根据不同腿部应用基准角度和校准值（修正顺序）
    switch(leg) {
        case 0: // FR (右前腿)
            AL = 85 - alpha - sst.FRElbow;   // 基准角度85°
            BE = beta + 40 - sst.FRFoot;      // 基准角度40°
            GA = gamma + 115 - sst.FRShdr;    // 基准角度115°
            break;
        case 1: // RR (右后腿)
            AL = alpha + 90 + sst.RRElbow;    // 基准角度90°
            BE = 115 - beta + sst.RRFoot;     // 基准角度115°
            GA = 115 - gamma + sst.RRShdr;    // 基准角度115°
            break;
        case 2: // FL (左前腿)
            AL = alpha + 75 + sst.FLElbow;    // 基准角度75°
            BE = 140 - beta + sst.FLFoot;     // 基准角度140°
            GA = 115 - gamma + sst.FLShdr;    // 基准角度115°
            break;
        case 3: // RL (左后腿)
            AL = 90 - alpha - sst.RLElbow;    // 基准角度90°
            BE = beta + 50 - sst.RLFoot;      // 基准角度50°
            GA = gamma + 100 - sst.RLShdr;    // 基准角度100°
            break;
    }

    // 使用与cankao.cpp完全相同的PWM设置方式
    int pwm_AL = angleToPWM(constrain(AL, 0, 180));
    print_final_PWM(servo_pin[leg][0], pwm_AL);
    pwm.setPWM(servo_pin[leg][0], 0, pwm_AL);

    int pwm_BE = angleToPWM(constrain(BE, 0, 180));
    print_final_PWM(servo_pin[leg][1], pwm_BE);
    pwm.setPWM(servo_pin[leg][1], 0, pwm_BE);

    int pwm_GA = angleToPWM(constrain(GA, 0, 180));
    print_final_PWM(servo_pin[leg][2], pwm_GA);
    pwm.setPWM(servo_pin[leg][2], 0, pwm_GA);
}

SemaphoreHandle_t Semaphore;

void servos_service(void * data) {
    // service_status_t sst = *(service_status_t *)data;
    for (;;) {
        float alpha, beta, gamma;
        xSemaphoreTake(Semaphore, portMAX_DELAY);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                if (abs(sst.site_now[i][j] - sst.site_expect[i][j]) >= abs(sst.temp_speed[i][j]))
                    sst.site_now[i][j] += sst.temp_speed[i][j];
                else
                    sst.site_now[i][j] = sst.site_expect[i][j];
            }
            cartesian_to_polar(alpha, beta, gamma, sst.site_now[i][0], sst.site_now[i][1], sst.site_now[i][2]);
            polar_to_servo(i, alpha, beta, gamma);
        }
        sst.rest_counter++;
        xSemaphoreGive(Semaphore);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        
#ifdef TIMER_INTERRUPT_DEBUG
        Serial.printf("%05lu counter: %lu\n",(unsigned long)millis(),(unsigned long)sst.rest_counter);
        Serial.printf("[OUT]\tA:%f\tB:%f\tG:%f\n",alpha, beta, gamma);
#endif
    }
}





void servos_start() {
    sit();
    b_init();
}

TaskHandle_t Task0;

void servos_init() {   //定义舵机服务
    // Options are: 240 (default), 160, 80, 40, 20 and 10 MHz
    setCpuFrequencyMhz(80);   //设置单片机核心运行速度
	int cpuSpeed = getCpuFrequencyMhz();
	Serial.println("CPU Running at " + String(cpuSpeed) + "MHz");
    Serial.println("starting PWM Library..");
    Wire.begin(SDA_PIN,SCL_PIN);
    pwm.begin();
    pwm.setPWMFreq(60);  // Analog servos ru n at ~60 Hz updates
    //initialize default parameter
    Serial.println("servo parameters:");
    set_site(2, x_default - x_offset, y_start + y_step, z_boot);
    set_site(3, x_default - x_offset, y_start + y_step, z_boot);
    set_site(0, x_default + x_offset, y_start, z_boot);
    set_site(1, x_default + x_offset, y_start, z_boot);
    Serial.printf("X:%f\tY:%f\tZ:%f\n",x_default,y_start,z_boot);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            sst.site_now[i][j] = sst.site_expect[i][j];
        }
    }
    Serial.println("starting servos service..");
    // Semafor only for service writing
	Semaphore = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        servos_service,   // Function that should be called
        "ServoService",  // Name of the task (for debugging)
        10000,          // Stack size (bytes)
        (void *)&sst,    // Parameter to pass
        1,               // Task priority
        &Task0,          // Task handle
        1
        );

    //initialize servos
    servos_start();
    Serial.println("Servos initialized.");

    Serial.println("Starting Bluetooth Library..");
   
    SerialBT.begin("ESP32test");
    Serial.println("BT Serial ready");
    Serial.println("Robot initialization Complete");
}

uint32_t ledPulse = 0;
uint32_t ledSpeed = LED_SPEED;

void servos_loop() {

    //-----------led blink status
    /*
    if (ledPulse <= ledSpeed) {
        digitalWrite(LED_BUILTIN, HIGH);
    }
    if (ledPulse > ledSpeed*3) {
        digitalWrite(LED_BUILTIN, LOW);
    }
    if (ledPulse >= ledSpeed*4) {
        ledPulse = 0;

        if (SerialBT.hasClient()) ledSpeed = LED_SPEED*3;
        else ledSpeed = LED_SPEED;

    }

    ledPulse++;
    */
    if (SerialBT.available()) {
   action_cmd();}
   

#ifdef TIMER_INTERRUPT_DEBUG
    Serial.printf("%05lu loop counter: %lu\n",(unsigned long)millis(),(unsigned long)sst.rest_counter);
#endif

}

int angleToPWM(int angle) {
    // 修改为与cankao.cpp一致的计算方式
    int pwm = ((850 / 180) * angle);
    return constrain(pwm, 125, 580);
}
