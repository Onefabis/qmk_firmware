// Copyright 2021 Christian Eiden, cykedev
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.	 If not, see <http://www.gnu.org/licenses/>.

#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "string.h"
#include "quantum.h"
#include "keyboard.h"
#include "pointing_device.h"
#include "transactions.h"
#include "drivers/sensors/pmw3360.h"
#include "drivers/haptic/DRV2605L.h"
#include "print.h"


#define CUSTOM_SAFE_RANGE SAFE_RANGE
#define COMBO_KEYS_COUNT 31
#define COMBO_MAX_SIZE 3
#define COMBO_STACK_MAX_SIZE 3
#define COMBO_WAIT_TIME 100
#define LANG_CHANGE_DEFAULT LANG_CHANGE_ALT_SHIFT


bool ledToggleState = 1;
bool enc1Button = 0;
bool enc2Button = 0;
uint8_t ledBrightness = 205;
uint8_t currentLedBrightness = 205;
uint8_t redHUE = 252;
uint8_t blueHUE = 146;
uint8_t yellowHUE = 15;
uint8_t greenHUE = 48;
uint8_t cyanHUE = 107;
uint8_t currentHUE = 146;
uint8_t purpleHUE = 196;


#    ifdef TAPPING_TERM_PER_KEY
#        define TAP_CHECK get_tapping_term(KC_BTN1, NULL)
#    else
#        ifndef TAPPING_TERM
#            define TAPPING_TERM 200
#        endif
#        define TAP_CHECK TAPPING_TERM
#    endif

keyboard_config_t keyboard_config;
uint16_t          dpi_array[] = TRACKBALL_DPI_OPTIONS;
#define DPI_OPTION_SIZE (sizeof(dpi_array) / sizeof(uint16_t))

bool     BurstState  = false;  // init burst state for Trackball module
bool     scrollMode  = false;
uint16_t MotionStart = 0;      // Timer for accel, 0 is resting state
static uint16_t mouse_timer           = 0;
static uint16_t mouse_debounce_timer  = 0;
static uint8_t  mouse_keycode_tracker = 0;
bool            tap_toggling          = false;
int16_t delta_x = 0;
int16_t delta_y = 0;
int16_t scroll_timer = 0;
bool scroll_direction = false;
bool force_trackball_arrows = false;
uint8_t qwerty_toggle = 0;
bool extra_sensor_button = false;


#define BACKLIGHT_TIMEOUT 5    // in minutes
static uint16_t idle_timer = 0;
static uint8_t halfmin_counter = 0;
bool ledDim = false;
bool mouseToggle = false;
bool skipKeyDebounce = false;
uint8_t response[RAW_EPSIZE];
uint8_t thisHand;
uint8_t thatHand;

/*
#define _EN 0
#define _EN_SH 1
#define _RU 2
#define _RU_SH 3
#define _EXTRAZ 3
*/

void keyboard_pre_init_user(void) {
	rgblight_disable();
};

#include "lang_shift/include.h"
#include "arbitrary_keycode/include.h"
#include "combo/include.h"
int basicLayerNum = 0;
// Defines names for use in layer keycodes and the keymap
enum layer_names {
	_EN = 0,
	_EN_SH = 1,
	_RU = 2,
	_RU_SH= 3,
	_EXTRAZ = 4,
	_MOUSE_LEFT = 5,
	_MOUSE_RIGHT = 6,
	_QWERTY = 7,
};

enum custom_keycodes {
  KEYCODES_START = CUSTOM_SAFE_RANGE,

  CT_C,
  CT_X,
  CT_V,
  CT_BSPC,
  CT_DEL,
  CT_Z,
  CT_Y,
  CT_PLUS,
  CT_MINUS,
  CT_F,
  CT_LT,
  CT_RT,
  CT_UP,
  CT_DN,
  LT_GT,
  EQ_EQ,
  GT_EQ,
  LT_EQ,
  DB_BR,
  DB_RN,
  DB_SB,
  LG_SW,
  KC_FSPC,
  LED_PLUS,
  LED_MINUS,
  LED_TOGGLE,
  SCROLL,
  MOUSE_OFF,
  SPACE,
  DB_AG,
};

#define LGUI_0 LGUI(KC_0)
#define LGUI_1 LGUI(KC_1)
#define LGUI_2 LGUI(KC_2)
#define LGUI_3 LGUI(KC_3)
#define LGUI_4 LGUI(KC_4)
#define LGUI_5 LGUI(KC_5)
#define LGUI_6 LGUI(KC_6)
#define LGUI_7 LGUI(KC_7)
#define LGUI_8 LGUI(KC_8)
#define LGUI_9 LGUI(KC_9)
#define LGUI_10 LGUI(KC_F6)
#define CT_A_0 LCA(KC_0)
#define CT_A_1 LCA(KC_1)
#define CT_A_2 LCA(KC_2)
#define CT_A_3 LCA(KC_3)
#define CT_A_4 LCA(KC_4)
#define CT_A_5 LCA(KC_5)

const ComboWithKeycode combos[] PROGMEM = {
  CHORD(AG_MINS,		CMB_000),
  CHORD(AG_SCLN,		CMB_001),
  CHORD(AG_EQL,			CMB_002),
  CHORD(AG_COLN,		CMB_003),
  CHORD(EN_QUOT,		CMB_004),
  CHORD(AG_CMSP,		CMB_005),
  CHORD(AG_SLSH,		CMB_006),
  CHORD(AG_PLUS,		CMB_007),

  IMMEDIATE_CHORD(SFT_N, SFT_N,	CMB_008),
  IMMEDIATE_CHORD(CTRL_0, CTRL_0, CMB_009),
  IMMEDIATE_CHORD(ALT_0,ALT_0, CMB_010),
  CHORD(CT_C,	   		CMB_011),

  CHORD(LA_CHNG,		CMB_012),
  CHORD(KC_SPC,			CMB_013),
  CHORD(AG_DOT,			CMB_014),
  CHORD(KC_DEL,			CMB_015),
  CHORD(KC_BSPC,		CMB_016),

  CHORD(DB_SB,			CMB_017),
  CHORD(LT_EQ,			CMB_018),
  CHORD(EQ_EQ,			CMB_019),
  CHORD(GT_EQ,			CMB_020),
  CHORD(DB_RN,			CMB_021),
  CHORD(EN_LT,			CMB_022),
  CHORD(LT_GT,			CMB_023),
  CHORD(EN_GT,			CMB_024),
  CHORD(DB_BR,			CMB_025),
  CHORD(AG_EXCL,		CMB_026),
  CHORD(AG_QUES,		CMB_027),
  CHORD(MO(_EXTRAZ),CMB_028),
  CHORD(EN_LPRN,		CMB_029),
  CHORD(AG_RPRN,		CMB_030),

  /* top row symbols BASIC LAYER left side*/
  CHORD(AG_DQUO, CMB_000, CMB_001),
  CHORD(EN_LBRC, CMB_001, CMB_002),
  CHORD(EN_RBRC, CMB_002, CMB_003),
  CHORD(EN_TILD, CMB_004, CMB_005),
  CHORD(EN_CIRC, CMB_005, CMB_006),
  CHORD(EN_AMPR, CMB_006, CMB_007),
  CHORD(CT_X,	 CMB_011, CMB_009),
  /* top row symbols BASIC LAYER right side*/

 /* chords	BASIC LAYER both sides*/
  CHORD(DB_AG, CMB_000, CMB_009),
  CHORD(DB_SB, CMB_001, CMB_009),
  CHORD(DB_BR, CMB_002, CMB_009),
  CHORD(DB_RN, CMB_029, CMB_009),
 /* chords	BASIC LAYER botj sides*/

  /* thumb cluster left side */
  CHORD(CT_V, 		 CMB_010, CMB_011),
  CHORD(TG(_EXTRAZ), CMB_008, CMB_028),

   /* thumb cluster right side */
  CHORD(AG_SDOT, 	 CMB_013, CMB_014),
  CHORD(AG_SDOT, 	 CMB_008, CMB_014),
  CHORD(WIN_0, 		 CMB_015, CMB_016),
  CHORD(LA_SYNC, 	 CMB_012, CMB_013),
  CHORD(CT_BSPC, 	 CMB_013, CMB_016),
  CHORD(CT_DEL, 	 CMB_012, CMB_015),

  /* top row combo symbols BASIC LAYER left side*/
  CHORD(AG_BSLS,	 CMB_000, CMB_002),
  CHORD(EN_DLR, 	 CMB_001, CMB_003),
  CHORD(EN_PIPE,	 CMB_000, CMB_003),

  /* top row combo symbols BASIC LAYER right side*/
  CHORD(EN_LCBR, 	 CMB_004, CMB_006),
  CHORD(EN_RCBR, 	 CMB_005, CMB_007),
  CHORD(EN_GRV, 	 CMB_004, CMB_007),

	/* thumb cluster both sides */
  CHORD(KC_TAB, 	 CMB_008, CMB_013),
  
 };
const uint8_t combos_size = sizeof(combos)/sizeof(ComboWithKeycode);



const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_QWERTY] = LAYOUT_5x6_5(
	KC_ESC,  AG_4, AG_3, AG_2, AG_1, AG_0,																						AG_9, AG_8, AG_7, AG_6, AG_5, AG_PERC,
	CMB_029, EN_Q, EN_W, EN_R, EN_T, EN_Y,																						EN_U, EN_I, EN_O, EN_P, EN_LCBR, EN_RCBR,
	XXXXXXX, EN_A, EN_S, EN_D, EN_F, EN_G,																						EN_H, EN_J, EN_K, EN_L, AG_COLN, EN_AT,
	CT_Z, EN_PIPE, EN_Z, EN_X, EN_C, EN_V,																						EN_B, EN_N, EN_M, EN_LT, EN_GT, AG_QUES,
				TG(_MOUSE_RIGHT), LGUI_0,				CT_F, CMB_008, LGUI(KC_F6),	CMB_012, CMB_013, CMB_014,	EN_LCBR, EN_RCBR,
																							CMB_010, LGUI(EN_W),	CMB_015, CMB_016
	),
	

	[_EXTRAZ] = LAYOUT_5x6_5(
	KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,																		KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,   KC_F12,
	XXXXXXX, XXXXXXX, LGUI_7, LGUI_8, LGUI_9, XXXXXXX,																KC_INS,  KC_HOME, KC_END,  KC_PGUP, KC_PGDN,  LED_PLUS,
	XXXXXXX, EN_LCBR, LGUI_4, LGUI_5, LGUI_6, XXXXXXX,																KC_PAUS, KC_LEFT, KC_UP,   KC_DOWN, KC_RGHT,  LED_MINUS,
	XXXXXXX, EN_RCBR, LGUI_1, LGUI_2, LGUI_3, XXXXXXX,																KC_TAB,  CMB_025, CMB_022, CMB_023, CMB_024,  LED_TOGGLE,
				   TG(_MOUSE_RIGHT), LGUI_0,					CT_F, CMB_008, LGUI(KC_F6),	CMB_012, CMB_013, CMB_014,	XXXXXXX, CMB_028,
																										CMB_010, LGUI(EN_W),	CMB_015, CMB_016
	),

	[_EN] = LAYOUT_5x6_5(
	KC_ESC,  CMB_000, CMB_001, CMB_002, CMB_003, CMB_026,															CMB_027, CMB_004, CMB_005, CMB_006, CMB_007, EN_AT,
	CMB_029, EN_B,    EN_Y,    EN_O,    EN_U,    XXXXXXX,															XXXXXXX, EN_L,    EN_D,    EN_W,    EN_V,    XXXXXXX,
	SFT_N_O, EN_C,    EN_I,    EN_E,    EN_A,    XXXXXXX,															EN_Z,    EN_H,    EN_T,    EN_S,    EN_N,    EN_Q,
	CT_Z,    EN_G,    EN_X,    EN_J,    EN_K,    XXXXXXX,															XXXXXXX, EN_R,    EN_M,    EN_F,    EN_P,    XXXXXXX,
			     TG(_MOUSE_RIGHT), EN_HASH,XXXXXXX,	KC_ENT, CMB_008, CMB_009,	CMB_012, CMB_013, CMB_014,		EN_UNDS, CMB_028,
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),

	[_EN_SH] = LAYOUT_5x6_5(
	KC_ESC,  AG_5,   AG_4,    AG_3,   AG_2,   AG_1,   																AG_0, AG_9,   AG_8,   AG_7,   AG_6,       AG_PERC,
	EN_RPRN, EN_S_G, EN_S_Y, EN_S_O, EN_S_U, XXXXXXX,																	EN_S_Q, EN_S_L, EN_S_D, EN_S_W, XXXXXXX, XXXXXXX,
	SFT_N_O, EN_S_C, EN_S_I, EN_S_E, EN_S_A, XXXXXXX,																	EN_S_V, EN_S_H, EN_S_T, EN_S_S, EN_S_N,  EN_S_B,
	CT_Y,    EN_S_X, EN_S_J, EN_S_K, XXXXXXX,XXXXXXX,																	EN_S_Z, EN_S_R, EN_S_M, EN_S_F, EN_S_P,  XXXXXXX,
			     TG(_MOUSE_RIGHT),AG_ASTR,	  			KC_ENT, CMB_008, CMB_009,	CMB_012, CMB_013, CMB_014,	RU_NUME, CMB_028, 
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),

	[_RU] = LAYOUT_5x6_5(
	KC_ESC,  CMB_000, CMB_001, CMB_002, CMB_003, CMB_026,															CMB_027, CMB_004, CMB_005, CMB_006, CMB_007, EN_AT,
	CMB_029, RU_F,    RU_SF,   RU_H,    RU_JA,   RU_Y,																RU_Z,    RU_V,    RU_K,    RU_D,    RU_CH,   RU_SH,
	SFT_N_O, RU_U,    RU_I,    RU_JE,   RU_O,    RU_A,																RU_L,    RU_N,    RU_T,    RU_S,    RU_R,    RU_J,
	CT_Z,    RU_JO,		RU_HD,   RU_E,    RU_JU,   RU_TS,																RU_B,    RU_M,    RU_P,    RU_G,    RU_ZH,   RU_SC,
			     TG(_MOUSE_RIGHT), EN_HASH,					KC_ENT, CMB_008, CMB_009,	CMB_012, CMB_013, CMB_014,		RU_UNDS, CMB_028,
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),

	[_RU_SH] = LAYOUT_5x6_5(
	KC_ESC,  AG_5,   AG_4,    AG_3,   AG_2,    AG_1,   																  AG_0,   AG_9,   AG_8,   AG_7,   AG_6,   AG_PERC,
	EN_RPRN, RU_S_F, RU_S_SF, RU_S_H, RU_S_JA, RU_S_Y,																  RU_S_Z, RU_S_V, RU_S_K, RU_S_D, RU_S_CH,RU_S_SH,
	SFT_N_O, RU_S_U, RU_S_I,  RU_S_JE,RU_S_O,  RU_S_A,																	RU_S_L, RU_S_N, RU_S_T, RU_S_S, RU_S_R, RU_S_J,
	CT_Y,    RU_S_JO,RU_S_HD, RU_S_E, RU_S_JU, RU_S_TS,																	RU_S_B, RU_S_M, RU_S_P, RU_S_G, RU_S_ZH,RU_S_SC,
				   TG(_MOUSE_RIGHT), AG_ASTR,				 KC_ENT, CMB_008, CMB_009,	CMB_012, CMB_013, CMB_014,		RU_NUME,CMB_028,
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),

	[_MOUSE_RIGHT] = LAYOUT_5x6_5(
	KC_ESC, CMB_000, CMB_001, CMB_002, CMB_003, CMB_026,															CMB_027, CMB_004, CMB_005, CMB_006, CMB_007, CT_Z,
	CMB_029, XXXXXXX, EN_Y, EN_O, EN_U, XXXXXXX,																			KC_MS_BTN3, KC_MS_BTN1, KC_MS_BTN4, KC_MS_BTN2, KC_MS_BTN5, CMB_030,
	SFT_N_O, EN_C, EN_I, EN_E, EN_A, XXXXXXX,																					KC_MS_BTN3, KC_MS_BTN1, KC_MS_BTN4, KC_MS_BTN2, KC_MS_BTN5, XXXXXXX,
	CT_Z, EN_G, EN_X, EN_J, EN_K, XXXXXXX,																						EN_Z, MOUSE_OFF, XXXXXXX, XXXXXXX, XXXXXXX, DPI_CONFIG,
				TG(_MOUSE_RIGHT), EN_HASH,					KC_ENT, CMB_008, CMB_009,	CMB_012, SPACE, CMB_014,		   EN_UNDS, CMB_028,
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),
	
	[_MOUSE_LEFT] = LAYOUT_5x6_5(
	CT_Z, CMB_007, CMB_006, CMB_005, CMB_004, CMB_027,      													CMB_026, CMB_003, CMB_002, CMB_001, CMB_000, KC_ESC,
	CMB_030, KC_MS_BTN5, KC_MS_BTN2, KC_MS_BTN4, KC_MS_BTN1, KC_MS_BTN3, 							XXXXXXX, EN_U, EN_O, EN_Y, XXXXXXX, CMB_029,
	XXXXXXX, KC_MS_BTN5, KC_MS_BTN2, KC_MS_BTN4, KC_MS_BTN1, KC_MS_BTN3,  						XXXXXXX, EN_A, EN_E, EN_I, EN_C, SFT_N_O,
	DPI_CONFIG, XXXXXXX, XXXXXXX, XXXXXXX, MOUSE_OFF, EN_P,													  XXXXXXX, EN_K, EN_J, EN_X, EN_G, CT_Z,
				TG(_MOUSE_RIGHT), EN_HASH,					KC_ENT, CMB_008, CMB_009,	CMB_012, SPACE, CMB_014,		 EN_UNDS, CMB_028,
																										CMB_010, CMB_011,	CMB_015, CMB_016
	),

};

bool usb_vbus_state(void) {
    setPinInputLow(USB_VBUS_PIN);
    wait_us(5);
    return readPin(USB_VBUS_PIN);
}

bool get_ignore_mod_tap_interrupt(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
	default:
	  return false;
  }
};

void persistent_default_layer_set(uint16_t default_layer) {
  default_layer_set(default_layer);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
	if (record->event.pressed) {
    #ifdef RGBLIGHT_ENABLE
        if (ledDim && ledToggleState) {
		ledDim = false;
    rgblight_enable_noeeprom(); // enables Rgb, without saving settings
		rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
		rgblight_mode_noeeprom(1);
        }
    #endif
    idle_timer = timer_read();
    halfmin_counter = 0;
    if(layer_state_is(_EXTRAZ)&&keycode==LGUI(EN_W)){
    	scroll_direction = true;
    	return false;
		}
  } else {
  	if(layer_state_is(_EXTRAZ)){
    	scroll_direction = false;
		}
  }
	if (!combo_process_record(keycode, record)) {
		return false;
	} else {
		if (!layer_state_is(_MOUSE_RIGHT)&&!layer_state_is(_MOUSE_LEFT)&&keycode!=ALT_0){
			mouse_keycode_tracker = 0;
			mouse_debounce_timer  = timer_read();
		}
	}
	if (!lang_shift_process_record(keycode, record)) {
		return false;
	} else {
		if (!layer_state_is(_MOUSE_RIGHT)&&!layer_state_is(_MOUSE_LEFT)&&keycode!=ALT_0){
			mouse_keycode_tracker = 0;
			mouse_debounce_timer  = timer_read();
		}
	}
	switch (keycode) {
		case LG_SW:
			if (record->event.pressed) {
				if (basicLayerNum==0){
					//MO(_EN);
				} else if (basicLayerNum==2){
					//MO(_RU);
				}
			}
			return false;
			break;
		case LED_PLUS:
			if (record->event.pressed && ledToggleState) {
				ledBrightness += 50;
				if (ledBrightness>255){
					ledBrightness = 255;
				}
				rgblight_enable_noeeprom(); // enables Rgb, without saving settings
				rgblight_sethsv_noeeprom(currentHUE, 255, ledBrightness); // sets the color to teal/cyan without saving
				rgblight_mode_noeeprom(1);
			}
			return false;
			break;
		case LED_MINUS:
			if (record->event.pressed && ledToggleState ){
				ledBrightness -= 50;
				if (ledBrightness<5){
					ledBrightness = 5;
				}
				rgblight_enable_noeeprom(); // enables Rgb, without saving settings
				rgblight_sethsv_noeeprom(currentHUE, 255, ledBrightness); // sets the color to teal/cyan without saving
				rgblight_mode_noeeprom(1);
			}
			return false;
			break;
		case LED_TOGGLE:
			if (record->event.pressed) {
				ledToggleState =! ledToggleState;
				rgblight_enable_noeeprom(); // enables Rgb, without saving settings
				if (ledToggleState){
					rgblight_sethsv_noeeprom(currentHUE, 255, ledBrightness); // sets the color to teal/cyan without saving
				} else {
					rgblight_sethsv_noeeprom(currentHUE, 255, 0); // sets the color to teal/cyan without saving
				}
				rgblight_mode_noeeprom(1);
			}
			return false;
			break;
		case SCROLL:
			if (record->event.pressed) {
			  if (scrollMode == false){
				  scrollMode = true;
			  } else {
				  scrollMode = false;
			  }
			}
			return false;
			break;
		case DB_AG:
			if (record->event.pressed) {
			  lang_shift_tap_key(AG_DQUO);
			  lang_shift_tap_key(AG_DQUO);
			  tap_code(KC_LEFT);
			}
			return false;
			break;
		case DB_SB:
			if (record->event.pressed) {
			  lang_shift_tap_key(EN_LBRC);
			  lang_shift_tap_key(EN_RBRC);
			  tap_code(KC_LEFT);
			}
			return false;
			break;
		case DB_RN:
			if (record->event.pressed) {
			  lang_shift_tap_key(AG_LPRN);
			  lang_shift_tap_key(AG_RPRN);
			  tap_code(KC_LEFT);
			}
			return false;
			break;
		case DB_BR:
			if (record->event.pressed) {
			  lang_shift_tap_key(EN_LCBR);
			  lang_shift_tap_key(EN_RCBR);
			  tap_code(KC_LEFT);
			}
			return false;
			break;
		case LT_EQ:
			if (record->event.pressed) {
			  lang_shift_tap_key(EN_LT);
			  lang_shift_tap_key(AG_EQL);
			}
			return false;
			break;
		case GT_EQ:
			if (record->event.pressed) {
			  lang_shift_tap_key(EN_GT);
			  lang_shift_tap_key(AG_EQL);
			}
			return false;
			break;
		case EQ_EQ:
			if (record->event.pressed) {
			  lang_shift_tap_key(AG_EQL);
			  lang_shift_tap_key(AG_EQL);
			}
			return false;
			break;
		case LT_GT:
			if (record->event.pressed) {
			  lang_shift_tap_key(EN_LT);
			  lang_shift_tap_key(EN_GT);
			}
			return false;
			break;
		case CT_PLUS:
			if (record->event.pressed) {
			  register_code16(C(KS_PLUS));
			  unregister_code16(C(KS_PLUS));
			}
			return false;
			break;
		case CT_MINUS:
			if (record->event.pressed) {
			  register_code16(C(KS_MINS));
			  unregister_code16(C(KS_MINS));
			}
			return false;
			break;
		case CT_UP:
			if (record->event.pressed) {
				register_code16(C(KC_UP));
				unregister_code16(C(KC_UP));
			}
			return false;
			break;
		case CT_DN:
			if (record->event.pressed) {
				register_code16(C(KC_DOWN));
				unregister_code16(C(KC_DOWN));
			}
			return false;
			break;
		case CT_LT:
			if (record->event.pressed) {
				register_code16(C(KC_LEFT));
				unregister_code16(C(KC_LEFT));
			}
			return false;
			break;
		case CT_RT:
			if (record->event.pressed) {
				register_code16(C(KC_RGHT));
				unregister_code16(C(KC_RGHT));
			}
			return false;
			break;
		case CT_F:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_F));
				unregister_code16(C(KC_F));
			}
			return false;
			break;
		case CT_C:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_C));
				unregister_code16(C(KC_C));
			}
			return false;
			break;
		case CT_X:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_X));
				unregister_code16(C(KC_X));
			}
			return false;
			break;
		case CT_V:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_V));
				unregister_code16(C(KC_V));
			}
			return false;
			break;
		case CT_BSPC:
			if (record->event.pressed) {
				register_code16(C(KC_BSPC));
				unregister_code16(C(KC_BSPC));
			}
			return false;
			break;
		case CT_DEL:
			if (record->event.pressed) {
				register_code16(C(KC_DEL));
				unregister_code16(C(KC_DEL));
			}
			return false;
			break;
		case CT_Z:
			if (record->event.pressed) {
				lang_activate(0);
				register_code16(C(KC_Z));
				unregister_code16(C(KC_Z));
			}
			return false;
			break;
		case CT_Y:
			if (record->event.pressed) {
				shift_activate(0);
				register_code16(C(KC_Y));
				unregister_code16(C(KC_Y));
			}
			return false;
			break;
		case SPACE:
			if (record->event.pressed) {
				if(layer_state_is(_MOUSE_RIGHT)){
					layer_off(_MOUSE_RIGHT);
				};
				if(layer_state_is(_MOUSE_LEFT)){
					layer_off(_MOUSE_LEFT);
				};
				if(layer_state_is(_EXTRAZ)){
					layer_off(_EXTRAZ);
				};
				if(layer_state_is(_QWERTY)){
					layer_off(_QWERTY);
				};

			}
			return false;
			break;
		case MOUSE_OFF:
			if (record->event.pressed) {
				layer_off(_MOUSE_RIGHT);
				layer_off(_MOUSE_LEFT);
			}
			return false;
			break;
		case TG(_MOUSE_RIGHT):
			if (record->event.pressed) {
				if (layer_state_is(_MOUSE_RIGHT)){
					mouse_keycode_tracker=0;
					mouseToggle=false;
				} else {
					mouse_keycode_tracker=1;
					mouseToggle=true;
				}

			}
			mouse_timer = timer_read();
			break;
		case TG(_MOUSE_LEFT):
			if (record->event.pressed) {
				if (layer_state_is(_MOUSE_LEFT)){
					mouse_keycode_tracker=0;
					mouseToggle=false;
				} else {
					mouse_keycode_tracker=1;
					mouseToggle=true;
				};
			}
			mouse_timer = timer_read();
			break;
		case ALT_0:
		case _EXTRAZ:
		case SFT_N:
		case CTRL_0:
		case LA_CHNG:
		case DPI_CONFIG:
    case KC_MS_UP ... KC_MS_WH_RIGHT:
			if (record->event.pressed) {
				mouse_keycode_tracker++;
				mouseToggle = true;
			} else {
				mouse_keycode_tracker--;
				mouseToggle = false;
			};
      mouse_timer = timer_read();
      break;
    default:
			if (IS_NOEVENT(record->event)) break;
      if (!mouse_keycode_tracker) {
				if(layer_state_is(_MOUSE_RIGHT)||layer_state_is(_MOUSE_LEFT)){
					layer_off(_MOUSE_RIGHT);
					layer_off(_MOUSE_LEFT);
				};
			};
			skipKeyDebounce = false;
      mouse_keycode_tracker = 0;
      mouse_debounce_timer  = timer_read();
      break;
	};
	return true;
};

bool dip_switch_update_user(uint8_t index, bool active) { 
    switch (index) {
        case 0:
            if(active) {
            	//layer_on(_QWERTY);
            	if (qwerty_toggle == 2){
            		layer_off(_QWERTY);
            		qwerty_toggle = 0;
            	} else {
            		layer_on(_QWERTY);
            		qwerty_toggle = 1;
            	}
            } else {
            	if (qwerty_toggle == 1){
            		qwerty_toggle = 2;
            	} 
            	//layer_off(_QWERTY);
            }
            break;
        case 1:
            if(active) {
            	extra_sensor_button = true;
            	//layer_on(_QWERTY);
            } else {
            	extra_sensor_button = false;
            	//layer_off(_QWERTY);
            }
            break;  
        case 2:
            if(active) {
            	layer_on(_QWERTY);
            } else {
            	layer_off(_QWERTY);
            }
            break;
        case 3:
            if(active) {
            	layer_on(_QWERTY);
            } else {
            	layer_off(_QWERTY);
            }
            break;  
    }
    return true;
}

void user_timer(void) {
	combo_user_timer();
	lang_shift_user_timer();
};

void matrix_scan_user(void) {
	user_timer();
	if (idle_timer == 0) idle_timer = timer_read();

    #ifdef RGBLIGHT_ENABLE
        if ( ledBrightness >= 5 && timer_elapsed(idle_timer) > 30000) {
            halfmin_counter++;
            idle_timer = timer_read();
        }
        if ( !ledDim && ledToggleState && halfmin_counter >= BACKLIGHT_TIMEOUT * 2) {
			rgblight_enable_noeeprom();
			ledDim = true;
			rgblight_sethsv_noeeprom(currentHUE, 255, 1);
			rgblight_mode_noeeprom(1);
            halfmin_counter = 0;
        }
    #endif

	if (timer_elapsed(mouse_timer) > 650 && !mouse_keycode_tracker && !mouseToggle) {
		if (layer_state_is(_MOUSE_RIGHT)||layer_state_is(_MOUSE_LEFT)){
			layer_off(_MOUSE_RIGHT);
			layer_off(_MOUSE_LEFT);
		};
    };

	if (timer_elapsed(scroll_timer) > 350){
		scroll_timer=0;
	}
};

void combo_max_count_error(void) {
};

void combo_max_size_error(void) {
};

void keyboard_post_init_user(void) {
	rgblight_enable_noeeprom(); // enables Rgb, without saving settings
	rgblight_sethsv_noeeprom(blueHUE, 255, ledBrightness); // sets the color to teal/cyan without saving
	rgblight_mode_noeeprom(1);
}

layer_state_t layer_state_set_user(layer_state_t state) {
	if (ledToggleState){
		switch (get_highest_layer(state)) {
		case _EN:
			currentHUE = blueHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _EN_SH:
			currentHUE = blueHUE;
			currentLedBrightness = (int)(ledBrightness*0.7);
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _RU:
			currentHUE = redHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _RU_SH:
			currentHUE = redHUE;
			currentLedBrightness = (int)(ledBrightness*0.7);
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255,  currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _EXTRAZ:
			currentHUE = yellowHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _MOUSE_LEFT:
			currentHUE = cyanHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _MOUSE_RIGHT:
			currentHUE = greenHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		case _QWERTY:
			currentHUE = purpleHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		default: //	 for any other layers, or the default layer
			currentHUE = greenHUE;
			currentLedBrightness = ledBrightness;
			rgblight_enable_noeeprom(); // enables Rgb, without saving settings
			rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
			rgblight_mode_noeeprom(1);
			break;
		};
	};
	return state;
}

#ifdef RAW_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length) {
	if(data[0] == 0) {
		lang_toggle(1);
		currentHUE = redHUE;
		currentLedBrightness = ledBrightness;
		rgblight_enable_noeeprom(); // enables Rgb, without saving settings
		rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
		rgblight_mode_noeeprom(1);
	} else if(data[0] == 1) {
		lang_toggle(0);
		currentHUE = blueHUE;
		currentLedBrightness = ledBrightness;
		rgblight_enable_noeeprom(); // enables Rgb, without saving settings
		rgblight_sethsv_noeeprom(currentHUE, 255, currentLedBrightness); // sets the color to teal/cyan without saving
		rgblight_mode_noeeprom(1);
	};
};
#endif


#ifdef ENCODER_ENABLE

bool encoder_update_user(uint8_t index, bool clockwise) {
	uint8_t temp_mod = get_mods();
	uint8_t temp_osm = get_oneshot_mods();
	bool	is_ctrl	 = (temp_mod | temp_osm) & MOD_MASK_CTRL;
	//bool	  is_shift = (temp_mod | temp_osm) & MOD_MASK_SHIFT;
	//bool	  is_alt = (temp_mod | temp_osm) & MOD_MASK_ALT;

	if (index == 0) { /* First encoder */
		if (!layer_state_is(_MOUSE_LEFT)&&!layer_state_is(_MOUSE_RIGHT)){
			mouse_keycode_tracker++;
			if (is_ctrl || enc2Button) {  // If a CTRL is being held
				del_mods(MOD_MASK_CTRL);	   // Ignore all CTRL keys
				if (clockwise) {
					tap_code(KC_PGUP);		   // PGDN on clockwise turn
				} else {
					tap_code(KC_PGDN);		   // PGUP on counter-clockwise
				}
				set_mods(temp_mod);			  // Add back CTRL key(s)

			} else {					 // If no CTRL is held
				if (clockwise) {
					tap_code(KC_UP);   // VOLUMEUP on clockwise turn
				} else {
					tap_code(KC_DOWN);	 // VOLUMEDOWN on counterclockwise
				}
			}
		} else {
			mouse_keycode_tracker++;
			if (clockwise) {
				tap_code(KC_WH_U);		   // PGDN on clockwise turn
			} else {
				tap_code(KC_WH_D);		   // PGUP on counter-clockwise
			}
		}
	} else if (index == 1) { /* Second encoder */
		if (!layer_state_is(_MOUSE_LEFT)&&!layer_state_is(_MOUSE_RIGHT)){
			mouse_keycode_tracker++;
			if (is_ctrl || enc1Button) {  // If a ALT is being held
				del_mods(MOD_MASK_CTRL);	   // Ignore all ALT keys
				if (clockwise) {
					tap_code(KC_LEFT);		   // PGDN on clockwise turn
				} else {
					tap_code(KC_RGHT);		   // PGUP on counter-clockwise
				}
				set_mods(temp_mod);			  // Add back ALT key(s)
			} else {
				 if (clockwise) {
					tap_code16(C(KC_LEFT));
				} else {
					tap_code16(C(KC_RGHT));
				}
			}
		} else {
			mouse_keycode_tracker++;
			if (!clockwise) {
				tap_code(KC_WH_U);		   // PGDN on clockwise turn
			} else {
				tap_code(KC_WH_D);		   // PGUP on counter-clockwise
			}
		}
	}
	mouse_timer = timer_read();
	mouse_keycode_tracker--;
	return false;
}
#endif


bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
#ifdef POINTING_DEVICE_ENABLE
	if (keycode == DPI_CONFIG && record->event.pressed) {
		keyboard_config.dpi_config = (keyboard_config.dpi_config + 1) % DPI_OPTION_SIZE;
		return false;
	};
#endif
    return true;
};

#ifdef POINTING_DEVICE_ENABLE
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
spi_status_t pmw3360_write(uint8_t reg_addr, uint8_t data);
void pointing_device_init_kb(void) {
	if (!is_keyboard_left()) {
		pointing_device_set_cpi(4000);
        pmw3360_write(0x11, constrain(ROTATIONAL_TRANSFORM_ANGLE_RIGHT, -127, 127));
    } else if (is_keyboard_left()){
		pointing_device_set_cpi(6400);
	};
    pointing_device_init_user();
};

report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
	uint8_t temp_mod = get_mods();
	uint8_t temp_osm = get_oneshot_mods();
	//bool	is_ctrl	 = (temp_mod | temp_osm) & MOD_MASK_CTRL;
	bool	is_alt	 = (temp_mod | temp_osm) & MOD_MASK_ALT;
    if ((right_report.x || right_report.y)&&(timer_elapsed(mouse_timer) > 370||skipKeyDebounce==true)) {
			if (layer_state_is(_EXTRAZ)||layer_state_is(_QWERTY)){
				delta_x += right_report.x;
				delta_y += right_report.y;
				if (layer_state_is(_QWERTY)){
					if (delta_x > 300) {
						if (extra_sensor_button==true){
							tap_code16(C(KC_RGHT));
						} else {
							tap_code16(KC_RGHT);
						};
						delta_x = 0;
					} else if (delta_x < -300) {
						if (extra_sensor_button==true){
							tap_code16(C(KC_LEFT));
						} else {
							tap_code16(KC_LEFT);
						};
						delta_x = 0;
					};
				} else {
					if (is_alt){
						if (delta_x > 300) {
							tap_code16(KC_RGHT);
							delta_x = 0;
						} else if (delta_x < -300) {
							tap_code16(KC_LEFT);
							delta_x = 0;
						};
					} else {
						if (delta_x > 220) {
							tap_code16(KC_RGHT);
							delta_x = 0;
						} else if (delta_x < -220) {
							tap_code16(KC_LEFT);
							delta_x = 0;
						};
					};
					
				};
				right_report.x = right_report.y = 0;
			} else {
				mouse_timer = timer_read();
				if (!layer_state_is(_MOUSE_RIGHT)&&timer_elapsed(mouse_debounce_timer) > 370) {
					layer_off(_MOUSE_LEFT);
					layer_on(_MOUSE_RIGHT);
				};
			};
			skipKeyDebounce = false;
  };
	if ((left_report.x || left_report.y)&&(timer_elapsed(mouse_timer) > 370||skipKeyDebounce==true)) {
		if (layer_state_is(_EXTRAZ)||layer_state_is(_QWERTY)){
			delta_x += left_report.x;
			delta_y += left_report.y;
			if (layer_state_is(_QWERTY)){
					if (delta_y > 300) {
						if (extra_sensor_button==true){
							tap_code(KC_UP);
						} else {
							tap_code(KC_PGUP);
						};
						delta_y = 0;
					} else if (delta_y < -300) {
						if (extra_sensor_button==true){
							tap_code(KC_DOWN);
						} else {
							tap_code(KC_PGDN);
						};
						delta_y = 0;
					};
				} else {
					if (is_alt){
						if (delta_x > 60) {
							left_report.h = 1;
							delta_x = 0;
						} else if (delta_x < -60) {
							left_report.h = -1;
							delta_x = 0;
						};
					} else {
						if (delta_y > 120) {
							left_report.v = -1;
							delta_y = 0;
						} else if (delta_y < -120) {
							left_report.v = 1;
							delta_y = 0;
						};
					};
				};
			left_report.x = left_report.y = 0;
		} else {
			mouse_timer = timer_read();
			if (!layer_state_is(_MOUSE_LEFT)&&timer_elapsed(mouse_debounce_timer) > 370) {
				layer_off(_MOUSE_RIGHT);
				layer_on(_MOUSE_LEFT);
			};
		}
		skipKeyDebounce = false;
  };
  return pointing_device_combine_reports(left_report, right_report);
};

#endif

