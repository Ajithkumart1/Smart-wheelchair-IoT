// Configuration Bits
#pragma config FOSC = HS        // HS oscillator
#pragma config WDTE = OFF       // Watchdog Timer disabled
#pragma config PWRTE = ON       // Power-up Timer enabled
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config LVP = OFF        // Low Voltage Programming disabled
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection disabled
#pragma config WRT = OFF        // Flash Program Memory Write disabled
#pragma config CP = OFF         // Flash Program Memory Code Protection disabled

#define _XTAL_FREQ 20000000
#include <xc.h>
#include <string.h>
#include <stdio.h>

// LCD Module Connection
#define RS RD4
#define EN RD5
#define D4 RD0
#define D5 RD1
#define D6 RD2
#define D7 RD3

// Hardware Connections
#define TRIG RB0
#define ECHO RB1
#define BUZZER RB7
#define JOYSTICK_X RA0
#define JOYSTICK_Y RA1
#define JOYSTICK_SW RB3
#define PULSE_SENSOR RA2
#define PULSE_LED RC0
#define IN1 RC3
#define IN2 RC4
#define IN3 RC5
#define IN4 RB2
#define ENA RC1
#define ENB RC2

// Constants
#define MIN_DISTANCE 25
#define JOYSTICK_CENTER 512
#define JOYSTICK_THRESHOLD 200
#define PULSE_THRESHOLD 512
#define BAUD_RATE 9600
#define DISPLAY_CYCLE_DELAY 2000  // 2 seconds per display
#define WIFI_UPDATE_INTERVAL 1000 // Send data every 1 second

// UART Commands
#define CMD_FORWARD 'F'
#define CMD_BACKWARD 'B'
#define CMD_LEFT 'L'
#define CMD_RIGHT 'R'
#define CMD_STOP 'S'

// Global Variables
unsigned int distance = 0;
char uart_received_data = 0;
unsigned char motor_speed = 150;
unsigned char obstacle_detected = 0;
char direction[16] = "Stopped";
unsigned int beats_per_minute = 0;
unsigned long last_display_change = 0;
unsigned char current_display = 0; // 0=distance, 1=BPM, 2=direction
unsigned long last_wifi_update = 0;

// Pulse Sensor Variables
unsigned int pulse_signal;
unsigned int pulse_previous = 0;
unsigned int last_beat_time = 0;
unsigned int beat_times[10] = {0};
unsigned char beat_index = 0;
unsigned char beats_counted = 0;
unsigned int pulse_max = 512;
unsigned int pulse_min = 512;
unsigned int threshold = 512;

// LCD Functions
void LCD_Command(unsigned char cmd) {
    RS = 0;
    D4 = (cmd & 0x10) >> 4;
    D5 = (cmd & 0x20) >> 5;
    D6 = (cmd & 0x40) >> 6;
    D7 = (cmd & 0x80) >> 7;
    EN = 1; __delay_us(100); EN = 0;
    
    D4 = (cmd & 0x01);
    D5 = (cmd & 0x02) >> 1;
    D6 = (cmd & 0x04) >> 2;
    D7 = (cmd & 0x08) >> 3;
    EN = 1; __delay_us(100); EN = 0;
}

void LCD_Char(unsigned char dat) {
    RS = 1;
    D4 = (dat & 0x10) >> 4;
    D5 = (dat & 0x20) >> 5;
    D6 = (dat & 0x40) >> 6;
    D7 = (dat & 0x80) >> 7;
    EN = 1; __delay_us(100); EN = 0;
    
    D4 = (dat & 0x01);
    D5 = (dat & 0x02) >> 1;
    D6 = (dat & 0x04) >> 2;
    D7 = (dat & 0x08) >> 3;
    EN = 1; __delay_us(100); EN = 0;
}

void LCD_Init(void) {
    __delay_ms(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    LCD_Command(0x80);
}

void LCD_String(const char *str) {
    while(*str) LCD_Char(*str++);
}

void LCD_Clear(void) {
    LCD_Command(0x01);
    __delay_ms(2);
    LCD_Command(0x80);
}

// Sensor Functions
void ADC_Init() {
    ADCON0 = 0x81;
    ADCON1 = 0x8E;
}

unsigned int ADC_Read(unsigned char channel) {
    if (channel > 7) return 0;
    ADCON0 &= 0xC5;
    ADCON0 |= (channel << 3);
    __delay_us(20);
    GO_nDONE = 1;
    while(GO_nDONE);
    return ((ADRESH << 8) + ADRESL);
}

unsigned int measure_distance(void) {
    unsigned int pulse_width = 0;
    TRIG = 1;
    __delay_us(10);
    TRIG = 0;
    
    unsigned int timeout = 1000;
    while(ECHO == 0 && --timeout > 0);
    if(timeout == 0) return 0;
    
    TMR1 = 0;
    T1CON = 0x01;
    timeout = 23200;
    while(ECHO == 1 && --timeout > 0);
    T1CON = 0x00;
    pulse_width = TMR1;
    
    return (pulse_width > 0) ? (pulse_width / 290) : 0;
}

// UART/WiFi Functions
void UART_Init(void) {
    unsigned int spbrg_value = (_XTAL_FREQ / (64UL * BAUD_RATE)) - 1;
    if (spbrg_value > 255) {
        spbrg_value = (_XTAL_FREQ / (16UL * BAUD_RATE)) - 1;
        BRGH = 1;
    }
    SPBRG = spbrg_value;
    SYNC = 0;
    SPEN = 1;
    CREN = 1;
    TXEN = 1;
}

void UART_WriteChar(char data) {
    while(!TXIF);
    TXREG = data;
}

char UART_ReadChar(void) {
    if(OERR) { CREN = 0; CREN = 1; }
    while(!RCIF);
    return RCREG;
}

void UART_WriteString(const char *text) {
    while(*text) UART_WriteChar(*text++);
}

void send_wifi_data(void) {
    char wifi_msg[50];
    sprintf(wifi_msg, "DIST:%dcm,BPM:%d,DIR:%s,SPD:%d\r\n", 
           distance, beats_per_minute, direction, motor_speed);
    UART_WriteString(wifi_msg);
}

// Motor Control Functions
void move_forward(void) {
    IN1 = 1; IN2 = 0; IN3 = 1; IN4 = 0;
    strcpy(direction, "Forward");
}

void move_backward(void) {
    IN1 = 0; IN2 = 1; IN3 = 0; IN4 = 1;
    strcpy(direction, "Backward");
}

void turn_left(void) {
    IN1 = 1; IN2 = 0; IN3 = 0; IN4 = 1;
    strcpy(direction, "Left");
}

void turn_right(void) {
    IN1 = 0; IN2 = 1; IN3 = 1; IN4 = 0;
    strcpy(direction, "Right");
}

void stop_motors(void) {
    IN1 = 0; IN2 = 0; IN3 = 0; IN4 = 0;
    strcpy(direction, "Stopped");
}

void set_motor_speed(unsigned char speed) {
    CCPR2L = speed >> 2;
    CCPR1L = speed >> 2;
}

// Pulse Sensor Processing
void process_pulse_sensor() {
    pulse_signal = ADC_Read(2);
    
    if (pulse_signal > pulse_max) pulse_max = pulse_signal;
    if (pulse_signal < pulse_min) pulse_min = pulse_signal;
    threshold = pulse_min + (pulse_max - pulse_min) / 2;
    
    if (pulse_signal > threshold && pulse_previous <= threshold) {
        unsigned int current_time = TMR0;
        unsigned int time_since_last_beat;
        
        if (current_time < last_beat_time) {
            time_since_last_beat = (65536 - last_beat_time) + current_time;
        } else {
            time_since_last_beat = current_time - last_beat_time;
        }
        
        if (time_since_last_beat > 20 && time_since_last_beat < 200) {
            beat_times[beat_index] = time_since_last_beat;
            beat_index = (beat_index + 1) % 10;
            if (beats_counted < 10) beats_counted++;
            
            if (beats_counted >= 3) {
                unsigned long total_time = 0;
                for (int i = 0; i < beats_counted; i++) {
                    total_time += beat_times[i];
                }
                unsigned long avg_time = total_time / beats_counted;
                beats_per_minute = (unsigned int)(1171875UL / avg_time);
                
                if (beats_per_minute < 50) beats_per_minute = 50;
                if (beats_per_minute > 180) beats_per_minute = 180;
            }
            
            last_beat_time = current_time;
            PULSE_LED = 1;
            __delay_ms(20);
            PULSE_LED = 0;
        }
    }
    
    pulse_previous = pulse_signal;
    
    static unsigned char reset_counter = 0;
    if (++reset_counter >= 100) {
        reset_counter = 0;
        pulse_max = pulse_signal;
        pulse_min = pulse_signal;
    }
}

// Joystick Processing
void process_joystick(void) {
    unsigned int x_value = ADC_Read(0);
    unsigned int y_value = ADC_Read(1);
    
    if (y_value < (JOYSTICK_CENTER - JOYSTICK_THRESHOLD)) {
        move_forward();
    } 
    else if (y_value > (JOYSTICK_CENTER + JOYSTICK_THRESHOLD)) {
        move_backward();
    }
    else if (x_value < (JOYSTICK_CENTER - JOYSTICK_THRESHOLD)) {
        turn_left();
    }
    else if (x_value > (JOYSTICK_CENTER + JOYSTICK_THRESHOLD)) {
        turn_right();
    }
    else if (!JOYSTICK_SW) {
        stop_motors();
        __delay_ms(300);
    }
}

// Display Management - REVISED LCD DISPLAY CODE
void update_display(void) {
    static unsigned long last_change = 0;
    char buf[16];
    
    // Check if it's time to change the display
    if((TMR0 - last_change) >= (DISPLAY_CYCLE_DELAY/10)) {
        current_display = (current_display + 1) % 3;
        last_change = TMR0;
        LCD_Clear();
    }
    
    // Show appropriate display with parameter on first line and value on second line
    LCD_Command(0x80); // First line
    switch(current_display) {
        case 0: // Distance display
            LCD_String("Distance:");
            LCD_Command(0xC0); // Second line
            if(obstacle_detected) {
                sprintf(buf, "%dcm STOP!", distance);
            } else {
                sprintf(buf, "%d cm", distance);
            }
            LCD_String(buf);
            break;
            
        case 1: // BPM display
            LCD_String("Heart Rate:");
            LCD_Command(0xC0); // Second line
            sprintf(buf, "%d BPM", beats_per_minute);
            LCD_String(buf);
            break;
            
        case 2: // Direction display
            LCD_String("Direction:");
            LCD_Command(0xC0); // Second line
            LCD_String(direction);
            break;
    }
}

void main(void) {
    // Initialize ports
    TRISC = 0x80;  // RC7 as input (UART RX), RC6 as output (UART TX)
    TRISD = 0x00;  // PORTD as output (LCD)
    TRISB = 0x0A;  // RB1(ECHO) and RB3(SW) as input
    TRISA = 0x07;  // RA0-RA2 as analog inputs
    TRISE = 0x00;  // PORTE as output (for IN4)
    
    // Initialize Timer0
    OPTION_REG = 0x07;
    
    // Initialize Timer1
    T1CON = 0x00;
    
    // Initialize Timer2 for PWM
    PR2 = 0xFF;
    CCP1CON = 0x0C;
    CCP2CON = 0x0C;
    T2CON = 0x04;
    
    // Initialize modules
    LCD_Init();
    ADC_Init();
    UART_Init();
    
    // Initialize pins
    TRIG = 0;
    BUZZER = 0;
    PULSE_LED = 0;
    IN1 = IN2 = IN3 = IN4 = 0;
    
    LCD_Clear();
    LCD_String("Smart Wheelchair");
    LCD_Command(0xC0);
    LCD_String("Initializing...");
    __delay_ms(500);
    LCD_Clear();
    
    UART_WriteString("SYSTEM:READY\r\n");
    
    while(1) {
        // Measure distance
        distance = measure_distance();
        
        // Check for obstacles
        if (distance > 0 && distance < MIN_DISTANCE) {
            BUZZER = 1;
            obstacle_detected = 1;
            stop_motors();
        } else {
            BUZZER = 0;
            obstacle_detected = 0;
        }
        
        // Process joystick if no obstacle
        if (!obstacle_detected) {
            process_joystick();
        }
        
        // Process pulse sensor
        process_pulse_sensor();
        
        // Handle UART commands
        if (RCIF) {
            uart_received_data = UART_ReadChar();
            
            if (!obstacle_detected) {
                switch(uart_received_data) {
                    case CMD_FORWARD: move_forward(); break;
                    case CMD_BACKWARD: move_backward(); break;
                    case CMD_LEFT: turn_left(); break;
                    case CMD_RIGHT: turn_right(); break;
                    case CMD_STOP: stop_motors(); break;
                    case '0'...'9': 
                        motor_speed = (uart_received_data - '0') * 25 + 30;
                        set_motor_speed(motor_speed);
                        break;
                }
            }
        }
        
        // Update display
        update_display();
        
        // Send data to WiFi periodically
        if((TMR0 - last_wifi_update) >= (WIFI_UPDATE_INTERVAL/10)) {
            send_wifi_data();
            last_wifi_update = TMR0;
        }
        __delay_ms(100); 
    }
}