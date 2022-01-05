#define MAX_TRANSITIONS 60 // limited dude lack of dynamic memory
#define MAX_ALPHABET_SYMBOLS 30 // limited dude lack of dynamic memory
#define TAPE_LENGTH 128 // limited dude lack of dynamic memory
#define MAX_MACHINES 10 // limited dude lack of dynamic memory

// States
#define START -1
#define MACHINE_SELECTION 0
#define INSERT_INPUT 1
#define RUN 2
#define HALTED 3

// Symbols
#define BLANK '*'

/****************** LIBRARIES ******************/
// sd card
#include <SPI.h>
#include <SD.h>

// liquid cristal display
#include <LiquidCrystal.h>
// initialize liquid crystal display library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

/****************** CONSTANTS ******************/
// buttons
const int8_t b_back = 22;
const int8_t b_left = 23;
const int8_t b_right = 24;
const int8_t b_enter = 25;
const int8_t b_next_state = 26;

//rgb led
int red_light_pin = 9;
int green_light_pin = 10;
int blue_light_pin = 11;

/****************** STRUCTS ******************/
// names of read machines
struct machines {
    String names[MAX_MACHINES];
    int size;
};

// accepted alphabet
struct alphabet {
    char symbols[MAX_ALPHABET_SYMBOLS];
    int8_t size;
};

// represents a transition
struct transition {
    String currentState; // current state of the machine
    char readSymbol; // symbol that the read head has read
    String nextState; // next state
    char writeSymbol; // symbol to write
    char dir; // direction to move
};

// used to store the transition table
struct transitions {
    transition transitions[MAX_TRANSITIONS];
    short size;
};

/****************** CLASSES ******************/
// represents the Turing machine
class Machine {
    public:
        char tape[TAPE_LENGTH];
        int headPos;
        String currentState;
        alphabet alphabet;
        transitions transition_table;
};

/****************** VARIABLES ******************/
// turing machine instance
Machine machine;

// phase where the program is
int8_t phase = START;

// phase of selecting machines
machines machines;
int8_t selectedMachine = 0;
File file;

// input phase variables
int8_t inputCursor = 0; // cursor position in the sreen
boolean selectSymbolMode = false; // true <=> selecting symbol. Otherwise, traversing the tape
int8_t selectedCharIndex = 0; // cycle between alphabet

long drawMillis = 0; // Millis tracker for drawing
long buttonMillis = 0; // Millis tracker for buttons
short interval = 500; // Interval at which update (milliseconds)

/**
 * Initialize the buttons, the lcd, reset the tape and build the machine
 */
void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    RGB_color(255, 165, 0);
    
    // initialise buttons
    pinMode(b_back, INPUT);
    pinMode(b_left, INPUT);
    pinMode(b_right, INPUT);
    pinMode(b_enter, INPUT);
    pinMode(b_next_state, INPUT);

    // intialise RGB led
    pinMode(red_light_pin, OUTPUT);
    pinMode(green_light_pin, OUTPUT);
    pinMode(blue_light_pin, OUTPUT); 

    // select number of rows and collumns that the lcd will use
    lcd.begin(16, 2);
    
    // If sd card is not found
    if (!SD.begin(53)) {
        lcd.setCursor(0, 0);
        lcd.write("SD card");
        lcd.setCursor(0, 1);
        lcd.write("not found");
        RGB_color(255, 0, 0);
        while(1);
    }

    // After detecting the SD cards select the machine
    lcd.setCursor(0, 0);
    lcd.write("Reading machines");
    lcd.setCursor(0,1);
    lcd.write("from SD card...");
    delay(1000);

    importMachineNames();

    lcd.clear();
    
    // stop if there are no machines to use
    if (machines.size == 0) {
        lcd.setCursor(0, 0);
        lcd.write("No machine found");
        RGB_color(255, 0, 0);
        while(1);
    }

    // enter MACHINE_SELECTION phase
    phase = MACHINE_SELECTION;
    RGB_color(0, 255, 0);
    selectedMachine = 0;

    // reset the tape
    resetTape();
}

/**
 * Reads file names from the SD Card
 */
void importMachineNames() {
    RGB_color(255, 165, 0);
    // open root directory
    File root = SD.open("/");

    int i = 0;
    while (i < MAX_MACHINES) {
        File entry = root.openNextFile();
        if (!entry) break; // no more entries
        machines.names[i] = entry.name();
        i++;
    }
    machines.size = i;
}

/**
 * Restart the tape with empty '*' symbols
 */
void resetTape() {
    // mark all cells as blank
    for (int i = 0; i < TAPE_LENGTH; i++) machine.tape[i] = BLANK;
}

/**
   Add the alphabet and transition function of the alphabet
*/
void buildMachine() {
    RGB_color(255, 165, 0);
    lcd.setCursor(0, 0);
    lcd.print("Building the");
    lcd.setCursor(0,1);
    lcd.print("machine. Wait...");

    char c;
    String token;
    
    // set machine current state
    machine.currentState = "0";

    // Check if file name is correct
    file = SD.open(machines.names[selectedMachine]);

    if (file) {
        // read number of symbol in the alphabet
        token = "";
        while ((c = file.read()) != '\r') {
            token += c;
        }
        machine.alphabet.size = token.toInt();
        
        // skip end of line \r\n
        file.read();
        
        // read accepted alphabet
        int i = 0;
        while ((c = file.read()) != '\r') {    
            if (c != ' ') {
                machine.alphabet.symbols[i] = c;
                i++;
            }
        }

        file.read(); // skip end of line

        // get the number of transitions
        token = "";
        while ((c = file.read()) != '\r') token += c;
        machine.transition_table.size = token.toInt();

        // skip carriage return and end of line
        file.read();
        
        // read transition table
        for (int i = 0; i < machine.transition_table.size; i++) {
            // current state
            token = "";
            while ((c = file.read()) != ' ') token += c;
            machine.transition_table.transitions[i].currentState = token;

            // read symbol
            machine.transition_table.transitions[i].readSymbol = file.read();
            file.read();
            
            // next state
            token = "";
            while ((c = file.read()) != ' ') token += c;
            machine.transition_table.transitions[i].nextState = token;

            // symbol to write
            machine.transition_table.transitions[i].writeSymbol = file.read();
            
            file.read();

            // direction to move
            machine.transition_table.transitions[i].dir = file.read(); 

            // skip carriage return and end of line
            file.read();
            file.read();
        }          
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("File not found");
        while(1);
    }
    Serial.println(machine.transition_table.size);
    // set initial position
    machine.headPos = TAPE_LENGTH / 2;

    delay(2000);
    phase = INSERT_INPUT;
    lcd.clear();
    lcd.cursor();
    
    for (int k = 0; k < machine.transition_table.size; k++) {
        Serial.println(machine.transition_table.transitions[k].nextState);
    }
}

/**
 * Interrupt function of the back button
 */
void b_back_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
            case MACHINE_SELECTION:
                break;
            case INSERT_INPUT:
                selectSymbolMode = false;
                lcd.noBlink();
                selectedCharIndex = 0;
                break;
            case RUN:
                break;
        }
        buttonMillis = millis();
    }
}

/**
 * Interrupt function of the left button
 */
void b_left_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
        case MACHINE_SELECTION:
            // Left control to machine selection menu
            if (selectedMachine == 0) selectedMachine = machines.size-1;
            else selectedMachine--;
            break;
        case INSERT_INPUT:
            // If tape cell is selected, select the character
            if (selectSymbolMode) {
            if (selectedCharIndex == 0) selectedCharIndex = machine.alphabet.size - 1;
            else selectedCharIndex--;
            machine.tape[machine.headPos] = machine.alphabet.symbols[selectedCharIndex];
            } else {
            if (machine.headPos > 0) {
            machine.headPos--; // otherwise move along the tape
            if (inputCursor > 0) inputCursor--; // move the cursor on the screen
            }
            }
            break;
        case RUN:
            break;
        case HALTED:
            break;
        }
        buttonMillis = millis();
    }
}

/**
 * Interrupt function of the right button
 */
void b_right_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
            case MACHINE_SELECTION:
                // Right control to machine selection menu
                if (selectedMachine == machines.size-1) selectedMachine = 0;
                else selectedMachine++;
                break;
            case INSERT_INPUT:
                // If tape cell is selected, select the character
                if (selectSymbolMode) {
                    if (selectedCharIndex == machine.alphabet.size - 1) selectedCharIndex = 0;
                    else selectedCharIndex++;
                    machine.tape[machine.headPos] = machine.alphabet.symbols[selectedCharIndex];
                } else  {
                    if (machine.headPos < TAPE_LENGTH - 1) {
                        machine.headPos++; // otherwise move along the tape
                        if (inputCursor < 15) inputCursor++; // move the cursor on the screen
                    }
                }
                break;
            case RUN:
                break;
            case HALTED:
                break;
        }
        buttonMillis = millis();
    }
}

/**
 * Interrupt function of the enter button
 */
void b_enter_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
            case MACHINE_SELECTION:
                phase = INSERT_INPUT;
                lcd.clear();
                buildMachine();
                RGB_color(0, 255, 0);
                break;
            case INSERT_INPUT:
                selectSymbolMode = true;
                lcd.blink();
                break;
            case RUN:
                break;
            case HALTED:
                break;
        }
        buttonMillis = millis();
    }
}

/**
 * Interrupt function of the next state button
 */
void b_next_state_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
            case MACHINE_SELECTION:
                phase = INSERT_INPUT;
                lcd.clear();
                buildMachine();
                RGB_color(0, 255, 0);
                break;
            case INSERT_INPUT:
                phase = RUN;
                lcd.noBlink();
                lcd.clear();
                break;
            case RUN:
                break;
            case HALTED:
                break;
        }
        buttonMillis = millis();
    }
}

/**
 * Routine used to compute the input (phase = RUN)
 */
void run() {
    RGB_color(255, 165, 0);
    boolean foundNextState = false;
    int i = 0;
    // loop until next state is found or there is no next state
    while (!foundNextState && i < machine.transition_table.1size) {
        transition transition = machine.transition_table.transitions[i];
        if (machine.currentState == transition.currentState && transition.readSymbol == machine.tape[machine.headPos]) {
            foundNextState = true;
            // write on the tape
            machine.tape[machine.headPos] = transition.writeSymbol;
            // move onto next state
            machine.currentState = transition.nextState;
            // move head position
            if (transition.dir == 'r') machine.headPos++;
            else if (transition.dir == 'l') machine.headPos--;
        }
        i++;
    }
    
    // if there is no next state, halt
    if (!foundNextState) {
        phase = HALTED;
        RGB_color(0, 255, 0);
    }
}

/**
 * Draws the tape near the head
 */
void draw() {
    switch (phase) {
        case MACHINE_SELECTION:
            lcd.setCursor(0,0);
            lcd.print("Select a machine");
            lcd.setCursor(0, 1);         
            lcd.print(machines.names[selectedMachine]);
            lcd.print("                ");
            break;
        case INSERT_INPUT:
            lcd.setCursor(0, 0);
            
            // Change the tape display depending on where the cursor is
            if (inputCursor == 0) for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i]);
            else if (inputCursor == 15) for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - 15]);
            else for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - inputCursor]);
         
            lcd.setCursor(2, 1);
            lcd.print("Insert input");
            lcd.setCursor(inputCursor, 0);
            break;
        case RUN:
            lcd.setCursor(0, 0);
            for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - 7]);
            
            lcd.setCursor(0, 1);
            lcd.print("Processing...");
            lcd.setCursor(7, 0);
            break;
         case HALTED:
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print("Halted");
            lcd.setCursor(7, 0);
            break;
    }
}

/**
 * Changes the rgb led color
 */
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
    analogWrite(red_light_pin, red_light_value);
    analogWrite(green_light_pin, green_light_value);
    analogWrite(blue_light_pin, blue_light_value);
}

/**
 * Interruption handling and lcd drawing
 */
void loop() {
    
    if (millis() - drawMillis > interval) {
        draw();
        if (phase == RUN) run();
        drawMillis = millis();
    }
    
    if (digitalRead(b_back) == HIGH)b_back_pressed();
    if (digitalRead(b_left) == HIGH) b_left_pressed();
    if (digitalRead(b_right) == HIGH) b_right_pressed();
    if (digitalRead(b_enter) == HIGH) b_enter_pressed();
    if (digitalRead(b_next_state) == HIGH) b_next_state_pressed();
    
}
