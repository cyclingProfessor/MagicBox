/////////////////////////////////////////////////////////////////////////////////////////////////////////
// NUM_LIGHTS switches attached to ports switch_port[] control four lights attached to ports light_port[]
// Switch at port switch_port[S] control the lights in bit mask switches[S].
// No two switches ever control the same light.
//
// Behaviour of lights is controlled by PHASE.  PHASE changes when the switches have all been seen turned ON (even momentarilty) and then have
// all been left OFF for PHASE_TIME.
//
// In PHASE_NORMAL all lights are controlled as would be expected.
// In PHASE_LIGHT_ORDER the first switch pressed will control light_port[0], the second will control light_port[1] etc.,
// PHASE_SWITCH_ORDER is identical to PHASE_LIGHT_ORDER.
// PHASE_BROKEN_LIGHT continues the previous switch allocation except that no switch control light_port[FAKE_BROKEN_LIGHT_ONE]
// PHASE_BROKEN_SWITCH_AND_LIGHT continues the previous switch allocation except that switch_port[0] controls light_port[FAKE_BROKEN_LIGHT_ONE].
// PHASE_BROKEN_SWITCH_AND_TWO_LIGHTS continues the previous switch allocation except that switch_port[0] controls both light_port[FAKE_BROKEN_LIGHT_ONE] and light_port[FAKE_BROKEN_LIGHT_TWO]
// PHASE_BROKEN_SWITCH_AND_TWO_LIGHTS continues the previous switch allocation except that switch_port[0] controls all lights
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Hardware configuration
const int NUM_LIGHTS = 4;
int switch_port[NUM_LIGHTS] = {10,12,14,16};
int light_port[NUM_LIGHTS] = {9,7,5,3};  // from left to right as you look at the battery/computer end of the box.

//////////////////////////////////////////////////////////////////////
// Control configuration
const int PHASE_TIME = 5000;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long off_time;  // How long have all switches been off?
int learning = -1; // Which light are we "learning" now? Could be determined by the switches[] array during learning.
bool broken = false; // Are we learning about a new "broken" light?
int normal = NUM_LIGHTS - 1; // Are we remembering the order of the lights to come on in a learning phase
int broken_light; // which light has just been "broken"
int broken_switch; // which switch does the "broken" lights
int light_order[NUM_LIGHTS]; // The order to use when learning the new colours.
int switches[NUM_LIGHTS]; // NUM_LIGHTS bit masks indicating the lights each switch controls. Bit X set means control light_port[X]

const int PHASE_NORMAL = 0;
const int PHASE_LIGHT_ORDER = 1;
const int PHASE_SWITCH_ORDER = 2;
const int PHASE_BROKEN_LIGHT = 3;
const int PHASE_BROKEN_SWITCH_AND_LIGHT = 4;
const int PHASE_BROKEN_SWITCH_AND_TWO_LIGHTS = 5;
const int PHASE_BROKEN_SWITCH_AND_ALL_LIGHTS = 6;
const int PHASE_NORMAL_AGAIN = 7;

const int NUM_PHASES = 8;
int phase = PHASE_NORMAL;

// Modes are do with waiting for all switches to have been on before we can time how long they all been off.
const int MODE_WAITING_FOR_ALL = 0; // only set the mask.  Wait till its full
const int MODE_WAITING_FOR_NONE = 1; // set and clear.  Wait till its empty
const int MODE_TIMING = 2; // mask must be empty

int mode = MODE_WAITING_FOR_ALL;
int switches_this_mode = 0;  // A bit mask storing whcih switches have been ON since the last MODE change. Bit X set means that we have seen switch_port[X] ON. 
int MASK_ALL = 0;

///////////////////////////////////////////////////////////////////////////////////
void setup() {
  int mask = 1;
  for (int index = 0; index < NUM_LIGHTS ; index++) {
    pinMode(light_port[index], OUTPUT);
    pinMode(switch_port[index], INPUT_PULLUP);
    switches[index] = mask; // all switches work... and do the normal thing
    MASK_ALL |= mask;
    mask <<= 1;
    
  }
  Serial.begin(9600);
  Serial.print("Magic is Loaded");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Check status of all switches, and build a mask of lights to be on.
  // Also learn the light association if we are in a "learning" phase
  int mask = 1;
  int temp;
  int light_mask = 0;
  for (int index = 0 ; index < NUM_LIGHTS ; index ++) {
    if (digitalRead(switch_port[index]) == false) {  // Inverted logic because we have PULLUP defaul.
      // Serial.print("Switch ON: "); Serial.println(index);
      if ((switches_this_mode & mask) == 0) {
        switches_this_mode |= mask;  // Notice that a new switch is ON
        if (mode == MODE_WAITING_FOR_ALL) {
          // Check if a new switch has been pressed this mode.....(there will be only one new switch)
          if (learning >= 0) {
            // assign the new switch if we are "learning"
            switches[index] = light_order[learning--];
            Serial.print("Mask for switch "); Serial.print(index); Serial.print(" is now "); Serial.println(switches[index]);
          } else if (broken > 0) {
            // Use the new switch just once if we are in a "broken" phase.
            // Only do this once
            broken = false;
            switch (phase) {
              case PHASE_BROKEN_LIGHT:
                // here first switch is the broken light switch - remember the light and set the switch mask to 0
                broken_light = switches[index];
                switches[index] = 0;
                break;
              case PHASE_BROKEN_SWITCH_AND_LIGHT:
                // Here the first switch must do the broken light
                temp = switches[index];
                switches[index] = broken_light;
                broken_light = temp;  // need to remember this in case we press the "broken_switch" to get the second bulb  
                broken_switch = index;
                break;
              case PHASE_BROKEN_SWITCH_AND_TWO_LIGHTS:
                // Here first switch is the second broken light....
                // Current switch does not do the broken light
                // Since broken light may equal index, we cannot set it to zero.
                if (broken_switch != index) {
                  broken_light = switches[index];
                  switches[index] &= ~broken_light;
                }
                switches[broken_switch] |= broken_light;
                break;
              default:
                // do nothing
                break;
            }
          } else if (normal >= 0) {
            // Otherwise just remember the switch order, but change nothing
            Serial.print("Remembering"); Serial.print(normal); Serial.print(" is now "); Serial.println(switches[index]);
            light_order[normal--] = switches[index];
          }
        }
      }

      light_mask |= switches[index]; // remember to switch on the right lights (including new learnt light)
    } else { // switch is OFF.
      // Store this if we are not WAITING_FOR_ALL.
      if (mode != MODE_WAITING_FOR_ALL) {
        switches_this_mode &= ~mask;
      }
    }
    mask <<= 1;
  }
  
  // Now switch on the appropriate lights
  mask = 1;
  for (int index = 0 ; index < NUM_LIGHTS ; index ++) {
    digitalWrite(light_port[index], light_mask & mask);
    mask <<= 1;
  }

  switch (mode) {
    case MODE_WAITING_FOR_ALL:
      if (switches_this_mode == MASK_ALL) {
        mode = MODE_WAITING_FOR_NONE;
        Serial.print("Mode is now "); Serial.println(mode);
      }
      break;
    case MODE_WAITING_FOR_NONE:
      if (switches_this_mode == 0) {
        // We have no switches ON - start timing.
        off_time = millis();
        mode = MODE_TIMING;
        Serial.print("Mode is now "); Serial.println(mode);
      }
      break;
    case MODE_TIMING:
      if (switches_this_mode != 0) {
        mode = MODE_WAITING_FOR_NONE; // A switch has been turned ON - stop timing and wait again until there are none on.
        Serial.print("Mode is now "); Serial.println(mode);
      } else if (millis() - off_time > PHASE_TIME) {
        mode = MODE_WAITING_FOR_ALL;  // This new phase cannot end until we have seen all switches ON at least once.
        Serial.print("Mode is now "); Serial.println(mode);
        phase = (phase + 1) % NUM_PHASES;
;
        learning = -1; // By default we are not learning!
        broken = false; // By default we are not finding which switch/light is broken
        normal = -1 ; // We are not remembering the tricky light order either
        switch(phase) {
          case PHASE_NORMAL:
          case PHASE_NORMAL_AGAIN:
            mask = 1;
            for (int index = 0 ; index < NUM_LIGHTS; index++) {
              switches[index] = mask; // all switches work... and do the normal thing
              mask <<= 1;
            }
            normal = NUM_LIGHTS - 1;  // used to remember the order so that learning is more interesting
            break;
          case PHASE_BROKEN_LIGHT:
          case PHASE_BROKEN_SWITCH_AND_LIGHT:
          case PHASE_BROKEN_SWITCH_AND_TWO_LIGHTS:
            broken = true;
            break;
          case PHASE_BROKEN_SWITCH_AND_ALL_LIGHTS:
            for (int index = 0 ; index < NUM_LIGHTS; index ++) {
              switches[index] = 0;
            }
            switches[broken_switch] = MASK_ALL;
            break;
          
          case PHASE_LIGHT_ORDER:
          case PHASE_SWITCH_ORDER:
            // The "magician" must now turn the four switches ON in the order of the light colours from the left.
            // The first switch turned ON turns on the the leftmost light (and remembers it), etc.,
            for (int index = 0 ; index < NUM_LIGHTS; index++) {
              switches[index] = 0; // No switches are assigned a value - this will be done dynamically
            }
            Serial.println("Now learning.....");
            learning = NUM_LIGHTS - 1;
        }
      }
  }
}
