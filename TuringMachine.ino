#define MAX_TRANSITIONS 50
#define MAX_ALPHABET_SYMBOLS 27
#define TAPE_LENGTH 128

// States
#define SETUP 0
#define INSERT_INPUT 1
#define RUN 2
#define HALTED 3

// Symbols
#define BLANK '*'

/****************** LIBRARIES ******************/
// library for liquid cristal display
#include <LiquidCrystal.h>
// initialize liquid crystal display library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/****************** CONSTANTS ******************/
// buttons
const int b_back = 6;
const int b_left = 7;
const int b_right = 8;
const int b_enter = 9;
const int b_next_state = 10;

/****************** STRUCTS ******************/
// accepted alphabet
struct alphabet {
    char symbols[MAX_ALPHABET_SYMBOLS];
    int size;
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
    int size;
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
// Turing machine instance
Machine machine;

// phase where the program is
int phase;

// input phase variables
int inputCursor; // cursor position in the sreen
boolean selectSymbolMode = false; // true <=> selecting symbol. Otherwise, traversing the tape
int selectedCharIndex = 0; // cycle between alphabet

long drawMillis = 0; // Millis tracker for drawing
long buttonMillis = 0; // Millis tracker for buttons
long interval = 500; // Interval at which update (milliseconds)

/**
 * Initialize the buttons, the lcd, reset the tape and build the machine
 */
void setup() {
    Serial.begin(9600);
    
    // initialise buttons as inputs
    pinMode(b_back, INPUT);
    pinMode(b_left, INPUT);
    pinMode(b_right, INPUT);
    pinMode(b_enter, INPUT);
    pinMode(b_next_state, INPUT);

    phase = 1;
    inputCursor = 0;
    
    // select number of rows and collumns that the lcd will use
    lcd.begin(16, 2);
    // enable display of the cursor in the lcd screen
    lcd.cursor();

    // reset the tape
    resetTape();
    
    // add machine alphabet and transition table
    buildMachine();
}

void resetTape() {
    // mark all cells as blank
    for (int i = 0; i < TAPE_LENGTH; i++) machine.tape[i] = BLANK;
}

/**
   Add the alphabet and transition function of the alphabet
*/
void buildMachine() { 
    // set machine current state
    machine.currentState = "0";
    
    // set initial position
    machine.headPos = TAPE_LENGTH / 2;
    
    // get accepted alphabet
    machine.alphabet.size = 3;
    machine.alphabet.symbols[0] = 'a';
    machine.alphabet.symbols[1] = 'b';
    machine.alphabet.symbols[2] = '#';    
    
    // get transition table  
    machine.transition_table.size = 29;
    
    machine.transition_table.transitions[0] = {.currentState = "0", .readSymbol = 'a', .nextState = "0", .writeSymbol = 'a', .dir = 'r'};
    Serial.println(machine.transition_table.transitions[0].currentState);
    Serial.println(machine.transition_table.transitions[0].currentState);
    Serial.println(machine.transition_table.transitions[0].currentState);
    machine.transition_table.transitions[1] = {.currentState = "0", .readSymbol = 'b', .nextState = "0", .writeSymbol = 'b', .dir = 'r'};   
    machine.transition_table.transitions[2] = {.currentState = "0", .readSymbol = BLANK, .nextState = "1", .writeSymbol = '#', .dir = 'l'};
    Serial.println(machine.transition_table.transitions[0].readSymbol);
    
    machine.transition_table.transitions[3] = {.currentState = "1", .readSymbol = 'a', .nextState = "2", .writeSymbol = '#', .dir = 'r'};
    machine.transition_table.transitions[4] = {.currentState = "1", .readSymbol = 'b', .nextState = "5", .writeSymbol = '#', .dir = 'r'};
    machine.transition_table.transitions[5] = {.currentState = "1", .readSymbol = BLANK, .nextState = "8", .writeSymbol = BLANK, .dir = 'r'};
    
    machine.transition_table.transitions[6] = {.currentState = "2", .readSymbol = 'a', .nextState = "2", .writeSymbol = 'a', .dir = 'r'};
    machine.transition_table.transitions[7] = {.currentState = "2", .readSymbol = 'b', .nextState = "2", .writeSymbol = 'b', .dir = 'r'};
    machine.transition_table.transitions[8] = {.currentState = "2", .readSymbol = '#', .nextState = "2", .writeSymbol = '#', .dir = 'r'};
    machine.transition_table.transitions[9] = {.currentState = "2", .readSymbol = BLANK, .nextState = "3", .writeSymbol = 'a', .dir = 'l'};
    
    machine.transition_table.transitions[10] = {.currentState = "3", .readSymbol = 'a', .nextState = "3", .writeSymbol = 'a', .dir = 'l'};
    machine.transition_table.transitions[11] = {.currentState = "3", .readSymbol = 'b', .nextState = "3", .writeSymbol = 'b', .dir = 'l'};
    machine.transition_table.transitions[12] = {.currentState = "3", .readSymbol = '#', .nextState = "4", .writeSymbol = '#', .dir = 'l'};
    
    machine.transition_table.transitions[13] = {.currentState = "4", .readSymbol = 'a', .nextState = "4", .writeSymbol = 'a', .dir = 'l'};
    machine.transition_table.transitions[14] = {.currentState = "4", .readSymbol = 'b', .nextState = "4", .writeSymbol = 'b', .dir = 'l'};
    machine.transition_table.transitions[15] = {.currentState = "4", .readSymbol = '#', .nextState = "1", .writeSymbol = 'a', .dir = 'l'};
    
    machine.transition_table.transitions[16] = {.currentState = "5", .readSymbol = 'a', .nextState = "5", .writeSymbol = 'a', .dir = 'r'};
    machine.transition_table.transitions[17] = {.currentState = "5", .readSymbol = 'b', .nextState = "5", .writeSymbol = 'b', .dir = 'r'};
    machine.transition_table.transitions[18] = {.currentState = "5", .readSymbol = '#', .nextState = "5", .writeSymbol = '#', .dir = 'r'};
    machine.transition_table.transitions[19] = {.currentState = "5", .readSymbol = BLANK, .nextState = "6", .writeSymbol = 'b', .dir = 'l'};
    
    machine.transition_table.transitions[20] = {.currentState = "6", .readSymbol = 'a', .nextState = "6", .writeSymbol = 'a', .dir = 'l'};
    machine.transition_table.transitions[21] = {.currentState = "6", .readSymbol = 'b', .nextState = "6", .writeSymbol = 'b', .dir = 'l'};
    machine.transition_table.transitions[22] = {.currentState = "6", .readSymbol = '#', .nextState = "7", .writeSymbol = '#', .dir = 'l'};
    
    machine.transition_table.transitions[23] = {.currentState = "7", .readSymbol = 'a', .nextState = "7", .writeSymbol = 'a', .dir = 'l'};
    machine.transition_table.transitions[24] = {.currentState = "7", .readSymbol = 'b', .nextState = "7", .writeSymbol = 'b', .dir = 'l'};
    machine.transition_table.transitions[25] = {.currentState = "7", .readSymbol = '#', .nextState = "1", .writeSymbol = 'b', .dir = 'l'};
    
    machine.transition_table.transitions[26] = {.currentState = "8", .readSymbol = 'a', .nextState = "8", .writeSymbol = BLANK, .dir = 'r'};
    machine.transition_table.transitions[27] = {.currentState = "8", .readSymbol = 'b', .nextState = "8", .writeSymbol = BLANK, .dir = 'r'};
    machine.transition_table.transitions[28] = {.currentState = "8", .readSymbol = '#', .nextState = "halt", .writeSymbol = BLANK, .dir = 'r'};
}

/**
 * Interrupt function of the back button
 */
void b_back_pressed() {
    if (millis() - buttonMillis > interval) {
        switch (phase) {
            case SETUP:
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
        case SETUP:
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
            case SETUP:
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
            case SETUP:
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
            case SETUP:
                break;
            case INSERT_INPUT:
                phase = RUN;
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
    boolean foundNextState = false;
    int i = 0;
    // loop until next state is found or there is no next state
    while (!foundNextState && i < machine.transition_table.size) {
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
        //Serial.println("\nState: " + machine.currentState);
        //Serial.println(String(machine.tape[machine.headPos]));
        //Serial.println("[*] HALTED [*]");
        phase = HALTED;
    }
}

/**
 * Draws the tape near the head
 */
void draw() {
    switch (phase) {
        case SETUP:
            break;
        case INSERT_INPUT:
            lcd.setCursor(0, 0);
            
            // Change draw depending on where is the cursor is
            if (inputCursor == 0) for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i]);
            else if (inputCursor == 15) for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - 15]);
            else for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - inputCursor]);
         
            lcd.setCursor(2, 1);
            lcd.write("Insert input");
            lcd.setCursor(inputCursor, 0);
            break;
        case RUN:
            lcd.setCursor(0, 0);
            for (int i = 0; i < 16; i++) lcd.write(machine.tape[machine.headPos + i - 7]);
            
            lcd.setCursor(0, 1);
            lcd.write("Processing...");
            lcd.setCursor(7, 0);
            break;
         case HALTED:
            lcd.setCursor(0, 1);
            lcd.write("                ");
            lcd.setCursor(0, 1);
            lcd.write("Halted");
            lcd.setCursor(7, 0);
            break;
    }
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
