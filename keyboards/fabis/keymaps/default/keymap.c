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

// #ifdef CONSOLE_ENABLE
// #include "print.h"
// #endif

#define CUSTOM_SAFE_RANGE SAFE_RANGE
#define COMBO_KEYS_COUNT 33
#define COMBO_MAX_SIZE 3
#define COMBO_STACK_MAX_SIZE 3
#define COMBO_WAIT_TIME 100
#define LANG_CHANGE_DEFAULT LANG_CHANGE_ALT_SHIFT

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

keyboard_config_t keyboard_config;
// uint16_t          dpi_array[] = TRACKBALL_DPI_OPTIONS;
// #define DPI_OPTION_SIZE (sizeof(dpi_array) / sizeof(uint16_t))

bool     BurstState  = false;  // init burst state for Trackball module
bool     scrollMode  = false;
uint16_t MotionStart = 0;      // Timer for accel, 0 is resting state
static uint16_t mouse_timer           = 0;
static uint16_t mouse_debounce_timer  = 0;
static uint8_t  mouse_keycode_tracker = 0;
static uint16_t haptic_timer          = 0;

bool            tap_toggling          = false;
int16_t delta_x = 0;
int16_t delta_y = 0;
int16_t scroll_timer = 0;
bool force_trackball_arrows = false;
uint8_t qwerty_toggle = 0;
bool extra_sensor_button_left = false;
bool extra_sensor_button_right = false;

#define BACKLIGHT_TIMEOUT 5    // in minutes
static uint16_t idle_timer = 0;
static uint8_t halfmin_counter = 0;
bool mouseToggle = false;
bool skipKeyDebounce = false;
uint8_t response[RAW_EPSIZE];

void keyboard_pre_init_user(void) {
	setPinInputHigh(B9);
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
	_MOUSE_L = 5,
	_MOUSE_R = 6,
	_QWERTY = 7,
};

//Tap Dance Declarations
enum {
  LPRN_RPRN,
  QUO_DBQUO,
};

enum custom_keycodes {
	KEYCODES_START = CUSTOM_SAFE_RANGE,
	CT_C,
	CT_X,
	CT_V,
	CT_ENT,
	ALT_ENT,
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
	DB_RN_EN,
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
	EX_BT_L,
	EX_BT_R,
	OSK_SHIFT,
	NVIM_YS,
	NVIM_DS,
	NVIM_CS,
	EN_EURO,
	EN_RUB,
	SDOT,
};


void LPRN_RPRN_FN(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
        	lang_shift_tap_key(AG_LPRN);
            break;
        case 2:
        	lang_shift_tap_key(AG_LPRN);
			lang_shift_tap_key(AG_RPRN);
			tap_code(KC_LEFT);
            break;
    }
};

void QUO_DBQUO_FN(tap_dance_state_t *state, void *user_data) {

    switch (state->count) {
        case 1:
        	lang_shift_tap_key(AG_DQUO);
            break;
        case 2:
        	lang_shift_tap_key(AG_DQUO);
			lang_shift_tap_key(AG_DQUO);
			tap_code(KC_LEFT);
            break;
    }
}

//Tap Dance Definitions
tap_dance_action_t tap_dance_actions[] = {
  [LPRN_RPRN] = ACTION_TAP_DANCE_FN(LPRN_RPRN_FN),
  [QUO_DBQUO] = ACTION_TAP_DANCE_FN(QUO_DBQUO_FN),
// Other declarations would go here, separated by commas, if you have them
};


const ComboWithKeycode combos[] PROGMEM = {
	CHORD(AG_MINS,		CMB_000),
	CHORD(AG_SCLN,		CMB_001),
	CHORD(AG_EQL,		CMB_002),
	CHORD(AG_COLN,		CMB_003),
	CHORD(EN_QUOT,		CMB_004),
	CHORD(AG_CMSP,		CMB_005),
	CHORD(AG_SLSH,		CMB_006),
	CHORD(AG_PLUS,		CMB_007),

	IMMEDIATE_CHORD(SFT_N, SFT_N,	CMB_008),
	//CHORD(OSM(MOD_LSFT), CMB_008),

	IMMEDIATE_CHORD(CTRL_0, CTRL_0, CMB_009),
	IMMEDIATE_CHORD(ALT_0,ALT_0, CMB_010),
	CHORD(CT_C,	   		CMB_011),

	CHORD(LA_CHNG,		CMB_012),
	CHORD(KC_SPC,			CMB_013),
	CHORD(AG_SDOT,	  CMB_014),
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
	CHORD(MO(_EXTRAZ),CMB_031),
	CHORD(KC_ENT,       CMB_032),


	/* top row symbols BASIC LAYER left side*/
	CHORD(AG_DQUO, CMB_000, CMB_001),
	CHORD(EN_LBRC, CMB_001, CMB_002),
	CHORD(EN_RBRC, CMB_002, CMB_003),
	CHORD(AG_COMM, CMB_004, CMB_005),
	CHORD(EN_CIRC, CMB_005, CMB_006),
	CHORD(EN_AMPR, CMB_006, CMB_007),
	CHORD(CT_X,	 CMB_011, CMB_009),
	/* top row symbols BASIC LAYER right side*/

 /* chords	BASIC LAYER both sides*/
	CHORD(DB_AG, CMB_000, CMB_009),
	CHORD(DB_SB, CMB_001, CMB_009),
	CHORD(DB_BR, CMB_002, CMB_009),


	CHORD(DB_RN, CMB_029, CMB_009),
	CHORD(TG(_MOUSE_R), CMB_028, CMB_031),
 /* chords	BASIC LAYER botj sides*/

	/* thumb cluster left side */
	CHORD(CT_V, 	   CMB_010, CMB_011),
	CHORD(TG(_EXTRAZ), CMB_008, CMB_028),
	CHORD(CT_ENT, 	   CMB_008, CMB_032),

	 /* thumb cluster right side */
	CHORD(AG_SDOT, 	 CMB_013, CMB_014),
	CHORD(AG_SDOT, 	 CMB_008, CMB_014),
	CHORD(WIN_0, 		 CMB_015, CMB_016),
	CHORD(LA_SYNC, 	 CMB_012, CMB_013),
	CHORD(AG_SDOT, 	 CMB_013, CMB_016),
	CHORD(CT_DEL, 	 CMB_012, CMB_015),

	/* top row combo symbols BASIC LAYER left side*/
	CHORD(AG_BSLS,	 CMB_000, CMB_002),
	CHORD(EN_TILD, 	 CMB_001, CMB_003),
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
	LCAG(KC_R),    AG_5,         AG_4,         AG_3,         AG_2,         AG_1, 				     AG_0,          AG_9,          AG_8,          AG_7,          AG_6,          KC_INS,
	LCAG(KC_E),    EN_Q,         EN_W,         EN_R,         RAG(KC_U),    EN_Y,				     EN_U,          EN_I,          EN_O,          EN_P,          KC_AMPR,       KC_CIRC,
	EN_DLR,        RAG(KC_C),    EN_S,         EN_D,         EN_F,         EN_G,				     EN_H,          RAG(KC_H),     EN_K,          RAG(KC_S),     EN_GRV,        KC_TILD,
	KC_CALC,       RAG(KC_G),    EN_Z,         RAG(KC_J),    EN_C,         EN_V,				     EN_B,          EN_N,          EN_M,          KC_MPLY,       KC_MNXT,       KC_PAUSE,

	CMB_010,       KC_PGUP,      KC_PGDN,	     KC_ENT,       CMB_008,      CMB_009,	         CTRL_0,        CMB_008,       AG_SDOT,	      KC_MPRV,       KC_MSTP,       KC_ESC,
	CMB_008,					     					                         CMB_010,      WIN_0,	           C(KC_DEL),     C(KC_BSPC),                                                 CT_ENT
	),

	[_EXTRAZ] = LAYOUT_5x6_5(
	KC_F1,         KC_F2,        KC_F3,        KC_F4,        KC_F5,        KC_F6,  				   KC_F7,         KC_F8,         KC_F9,         KC_F10,        KC_F11,        KC_F12,
	EN_BSLS,       LCAG(KC_P),   LGUI_7,       LGUI_8,       LGUI_9,       S(KC_TAB),        DB_RN,         EN_LPRN,       EN_RPRN,       AG_PERC,       KC_AMPR,       LCAG(KC_E),
	LCAG(KC_A),    EN_GRV,       LGUI_4,       LGUI_5,       LGUI_6,       KC_TAB,           DB_SB,         EN_LBRC,       EN_RBRC,       EN_LT,         EN_GT,         LCAG(KC_R),
	LCAG(KC_D),    C(KC_F),      LGUI_1,       LGUI_2,       LGUI_3,       KC_TILD,				   LCAG(KC_S),    EN_LCBR,       EN_RCBR,       EN_PIPE,       EN_CIRC,       EN_DLR,

	LGUI(KC_F6),   XXXXXXX,      LGUI_0,	     RCS(KC_V),    C(KC_C),      NVIM_DS,	         CMB_012,       CMB_013,       C(KC_SLSH),	  EN_PIPE,       XXXXXXX,       MO(_QWERTY),
	EX_BT_L,							                                   NVIM_CS,      NVIM_YS,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_EN] = LAYOUT_5x6_5(
	KC_ESC,        CMB_000,      CMB_001,      CMB_002,      CMB_003,      CMB_026,				   CMB_027,       CMB_004,       CMB_005,       CMB_006,       CMB_007,       EN_AT,
	AG_COMM,       EN_B,         EN_Y,         EN_O,         EN_U,         XXXXXXX, 			   XXXXXXX,       EN_L,          EN_D,          EN_W,          EN_V,          XXXXXXX,
	AG_DQUO,       EN_C,         EN_I,         EN_E,         EN_A,         XXXXXXX,  			   EN_Z,          EN_H,          EN_T,          EN_S,          EN_N,          EN_Q,
	CT_Z,          EN_G,         EN_X,         EN_J,         EN_K,         XXXXXXX,				   XXXXXXX,       EN_R,          EN_M,          EN_F,          EN_P,          XXXXXXX,

	MO(_QWERTY),   OSL(4),       EN_HASH,	     CMB_032,      OSL(1),       CMB_009,	         CMB_012,       CMB_013,       AG_DOT,		    EN_UNDS,       OSL(4),        MO(_QWERTY),
	EX_BT_L,					                                       CMB_010,      CMB_011,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_EN_SH] = LAYOUT_5x6_5(
	KC_ESC,        AG_5,         AG_4,         AG_3,         AG_2,         AG_1, 				     AG_0,          AG_9,          AG_8,          AG_7,          AG_6,          AG_PERC,
	AG_COMM,       EN_S_B,       EN_S_Y,       EN_S_O,       EN_S_U,       XXXXXXX,				   XXXXXXX,       EN_S_L,        EN_S_D,        EN_S_W,        EN_S_V,        XXXXXXX,
	DB_AG,         EN_S_C,       EN_S_I,       EN_S_E,       EN_S_A,       XXXXXXX,		  	   EN_S_Z,        EN_S_H,        EN_S_T,        EN_S_S,        EN_S_N,        EN_S_Q,
	CT_Y,          EN_S_G,       EN_S_X,       EN_S_J,       EN_S_K,       XXXXXXX,	         XXXXXXX,       EN_S_R,        EN_S_M,        EN_S_F,        EN_S_P,        XXXXXXX,

	MO(_QWERTY),   CMB_031,      AG_ASTR,	   CMB_032,        CMB_008,      CMB_009,	         CMB_012,       CMB_013,       SDOT,	        RU_NUME,       CMB_028,       MO(_QWERTY),
	EX_BT_L,								                                 CMB_010,      CMB_011,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_RU] = LAYOUT_5x6_5(
	KC_ESC,        CMB_000,      CMB_001,      CMB_002,      CMB_003,      CMB_026,          CMB_027,       CMB_004,       CMB_005,       CMB_006,       CMB_007,       EN_AT,
	AG_COMM,       RU_F,         RU_SF,        RU_H,         RU_JA,        RU_Y,             RU_Z,          RU_V,          RU_K,          RU_D,          RU_CH,         RU_SH,
	AG_DQUO,       RU_U,         RU_I,         RU_JE,        RU_O,         RU_A,             RU_L,          RU_N,          RU_T,          RU_S,          RU_R,          RU_J,
	CT_Z,          RU_JO,        RU_HD,        RU_E,         RU_JU,        RU_TS,            RU_B,          RU_M,          RU_P,          RU_G,          RU_ZH,         RU_SC,

	MO(_QWERTY),   OSL(4),       EN_HASH,      CMB_032,      OSL(3),       CMB_009,	         CMB_012,       CMB_013,       AG_DOT,		    RU_UNDS,       OSL(4),        MO(_QWERTY),
	EX_BT_L,												                         CMB_010,      CMB_011,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_RU_SH] = LAYOUT_5x6_5(
	KC_ESC,        AG_5,         AG_4,         AG_3,         AG_2,         AG_1,             AG_0,	        AG_9,          AG_8,          AG_7,          AG_6,          AG_PERC,
	AG_COMM,       RU_S_F,       RU_S_SF,      RU_S_H,       RU_S_JA,      RU_S_Y,           RU_S_Z,        RU_S_V,        RU_S_K,        RU_S_D,        RU_S_CH,       RU_S_SH,
	DB_AG,         RU_S_U,       RU_S_I,       RU_S_JE,      RU_S_O,       RU_S_A,           RU_S_L,        RU_S_N,        RU_S_T,        RU_S_S,        RU_S_R,        RU_S_J,
	CT_Y,          RU_S_JO,      RU_S_HD,      RU_S_E,       RU_S_JU,      RU_S_TS,          RU_S_B,        RU_S_M,        RU_S_P,        RU_S_G,        RU_S_ZH,       RU_S_SC,

	MO(_QWERTY),   CMB_031,      AG_ASTR,	     CMB_032,      CMB_008,      CMB_009,	         CMB_012,       CMB_013,       SDOT,		      RU_NUME,       CMB_028,       MO(_QWERTY),
	EX_BT_L,											                           CMB_010,      CMB_011,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_MOUSE_R] = LAYOUT_5x6_5(
	KC_ESC,        CMB_000,      CMB_001,      CMB_002,      CMB_003,      CMB_026,          CMB_027,       CMB_004,       CMB_005,       CMB_006,       CMB_007,       CT_Z,
	CMB_029,       XXXXXXX,      EN_Y,         EN_O,         EN_U,         XXXXXXX,          KC_MS_BTN3,    KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    KC_MS_BTN5,    CMB_030,
	SFT_N_O,       EN_C,         EN_I,         EN_E,         EN_A,         XXXXXXX,          KC_MS_BTN3,    KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    KC_MS_BTN5,    XXXXXXX,
	CT_Z,          CMB_028,      EN_X,         EN_J,         EN_K,         XXXXXXX,          EN_Z,          KC_MS_BTN1,    KC_MS_BTN4,    KC_MS_BTN2,    KC_MS_BTN5,    DPI_CONFIG,

	MO(_QWERTY),   CMB_031,      XXXXXXX,      CMB_032,      CMB_008,      CMB_009,          CMB_012,       SPACE,         CMB_014,       EN_UNDS,       CMB_028,       MO(_QWERTY),
	EX_BT_L,                                                 CMB_010,      CMB_011,          CMB_015,       CMB_016,                                                    EX_BT_R
	),

	[_MOUSE_L] = LAYOUT_5x6_5(
	CT_Z,          CMB_007,      CMB_006,      CMB_005,      CMB_004,      CMB_027,          CMB_026,       CMB_003,       CMB_002,       CMB_001,       CMB_000,       KC_ESC,
	CMB_030,       KC_MS_BTN5,   KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   KC_MS_BTN3,       XXXXXXX,       EN_U,          EN_O,          EN_Y,          XXXXXXX,       CMB_029,
	XXXXXXX,       KC_MS_BTN5,   KC_MS_BTN2,   KC_MS_BTN4,   KC_MS_BTN1,   KC_MS_BTN3,       XXXXXXX,       EN_A,          EN_E,          EN_I,          EN_C,          SFT_N_O,
	DPI_CONFIG,	   CMB_028,      XXXXXXX,      XXXXXXX,      MOUSE_OFF,    EN_P,             XXXXXXX,       EN_K,          EN_J,          EN_X,          EN_G,          CT_Z,

	MO(_QWERTY),   CMB_031,      XXXXXXX,      CMB_032,      CMB_008,      CMB_009,	         CMB_012,       SPACE,         CMB_014,	      EN_UNDS,       CMB_028,       MO(_QWERTY),
	EX_BT_L,												                         CMB_010,      CMB_011,	         CMB_015,       CMB_016,                                                    EX_BT_R
	),
};

// bool usb_vbus_state(void) {
// 	setPinInputLow(USB_VBUS_PIN);
// 	wait_us(5);
// 	return readPin(USB_VBUS_PIN);
// }


void persistent_default_layer_set(uint16_t default_layer) {
	default_layer_set(default_layer);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  #ifdef CONSOLE_ENABLE
    uprintf("0x%04X,%u,%u,%u,%u,0x%02X,0x%02X,%u\n", keycode, record->event.key.row, record->event.key.col, get_highest_layer(layer_state), record->event.pressed, get_mods(), get_oneshot_mods(), record->tap.count);
  #endif

	if (record->event.pressed) {
		idle_timer = timer_read();
		halfmin_counter = 0;
	}
	if (!combo_process_record(keycode, record)) {
		return false;
	} else {
		if (!layer_state_is(_MOUSE_R)&&!layer_state_is(_MOUSE_L)&&keycode!=ALT_0){
			mouse_keycode_tracker = 0;
			mouse_debounce_timer  = timer_read();
		}
	}
	if (!lang_shift_process_record(keycode, record)) {
		return false;
	} else {
		if (!layer_state_is(_MOUSE_R)&&!layer_state_is(_MOUSE_L)&&keycode!=ALT_0){
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
		case NVIM_YS:
			if (record->event.pressed) {
				lang_shift_tap_key(EN_W);
				lang_shift_tap_key(EN_S);
			}
			return false;
			break;
		case NVIM_CS:
			if (record->event.pressed) {
				lang_shift_tap_key(EN_V);
				lang_shift_tap_key(EN_S);
			}
			return false;
			break;
		case NVIM_DS:
			if (record->event.pressed) {
				lang_shift_tap_key(EN_S);
				lang_shift_tap_key(EN_S);
			}
			return false;
			break;
		case SDOT:
			if (record->event.pressed) {
				register_code16(C(KC_DOT));
				unregister_code16(C(KC_DOT));
				register_code16(C(KC_SPC));
				unregister_code16(C(KC_SPC));
				register_code16(C(OSL(1)));
				unregister_code16(C(OSL(1)));
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
				tap_code(KC_LEFT);
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
		case ALT_ENT:
			if (record->event.pressed) {
				tap_code16(A(KC_ESC));
			}
			return false;
			break;
		case CT_ENT:
			if (record->event.pressed) {
				tap_code16(C(KC_ENT));
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
		case EX_BT_L:
			if (record->event.pressed) {
				extra_sensor_button_left = true;
			} else {
				extra_sensor_button_left = false;
				extra_sensor_button_right = false;
			}
			return false;
			break;
		case EX_BT_R:
			if (record->event.pressed) {
				extra_sensor_button_right = true;
			} else {
				extra_sensor_button_right = false;
				extra_sensor_button_left = false;
			}
			return false;
			break;
		case SPACE:
			if (record->event.pressed) {
				if(layer_state_is(_MOUSE_R)){
					layer_off(_MOUSE_R);
				};
				if(layer_state_is(_MOUSE_L)){
					layer_off(_MOUSE_L);
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
				layer_off(_MOUSE_R);
				layer_off(_MOUSE_L);
			}
			return false;
			break;
		case TG(_MOUSE_R):
			if (record->event.pressed) {
				if (layer_state_is(_MOUSE_R)){
					mouse_keycode_tracker=0;
					mouseToggle=false;
				} else {
					mouse_keycode_tracker=1;
					mouseToggle=true;
				}

			}
			mouse_timer = timer_read();
			break;
		case TG(_MOUSE_L):
			if (record->event.pressed) {
				if (layer_state_is(_MOUSE_L)){
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
				if(layer_state_is(_MOUSE_R)||layer_state_is(_MOUSE_L)){
					layer_off(_MOUSE_R);
					layer_off(_MOUSE_L);
				};
			};
			skipKeyDebounce = false;
			mouse_keycode_tracker = 0;
			mouse_debounce_timer  = timer_read();
			break;

	};
	return true;
};



void user_timer(void) {
	combo_user_timer();
	lang_shift_user_timer();
};


void matrix_scan_user(void) {
	user_timer();
	if (idle_timer == 0) idle_timer = timer_read();
	if (timer_elapsed(mouse_timer) > 550 && !mouse_keycode_tracker && !mouseToggle) {
		if (layer_state_is(_MOUSE_R)||layer_state_is(_MOUSE_L)){
			layer_off(_MOUSE_R);
			layer_off(_MOUSE_L);
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
	pointing_device_set_cpi(7200);
    pointing_device_set_cpi_on_side(false, 8120);
	// pointing_device_set_cpi_on_side(true, 6000); //Set cpi on left side to a low value for slower scrolling.
    // pointing_device_set_cpi_on_side(false, 6000); //Set cpi on right side to a reasonable value for mousing.
	haptic_disable();
}

layer_state_t layer_state_set_user(layer_state_t state) {
	uint8_t response_raw[RAW_EPSIZE];
    memset(response_raw, 0, RAW_EPSIZE);
	switch (get_highest_layer(state)) {
	case _EN:
		response_raw[0] = 0;
		break;
	case _RU:
		response_raw[0] = 1;
		break;
	case _EXTRAZ:
		response_raw[0] = 2;
		break;
	case _QWERTY:
		response_raw[0] = 5;
		break;
	case _MOUSE_L:
		response_raw[0] = 3;
		break;
	case _MOUSE_R:
		response_raw[0] = 4;
		break;
	default:
		response_raw[0] = 0;
		break;
	};
    raw_hid_send(response_raw, RAW_EPSIZE);
	return state;
}

#ifdef RAW_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length) {
	if(data[0] == 0) {
		lang_toggle(1);
	} else if(data[0] == 1) {
		lang_toggle(0);
	};
};
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
	if (!process_record_user(keycode, record)) {
		return false;
	}
	return true;
};

#ifdef POINTING_DEVICE_ENABLE
void pointing_device_init_kb(void) {
    pointing_device_init_user();
};


report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
	if ((right_report.x || right_report.y)&&(timer_elapsed(mouse_timer) > 370||skipKeyDebounce==true)) {
		if (layer_state_is(_EXTRAZ)||layer_state_is(_QWERTY)){
			if (haptic_timer ==0){
				haptic_timer = timer_read();
			}
			if (layer_state_is(_QWERTY)){
				delta_x += right_report.x;
				if (delta_x > 350) {
					tap_code16(KC_RGHT);
					if (timer_elapsed(haptic_timer) > 30){
						DRV_pulse(sharp_click_30);
						haptic_timer = 0;
					};
					delta_x = 0;
				} else if (delta_x < -350) {
					tap_code16(KC_LEFT);
					if (timer_elapsed(haptic_timer) > 30){
						DRV_pulse(sharp_click_30);
						haptic_timer = 0;
					};
					delta_x = 0;
				};
			} else if (layer_state_is(_EXTRAZ)){
				delta_y += right_report.y;
				if (delta_y > 120) {
		            right_report.v = -1;
		            delta_y = 0;
		        } else if (delta_y < -120) {
		          	right_report.v = 1;
		            delta_y = 0;
		        };
    		};
			right_report.x = right_report.y = 0;
		} else {
			if (extra_sensor_button_left == true){
				delta_y += right_report.y;
				if (delta_y > 120) {
		         	tap_code(KC_DOWN);
		        	tap_code(KC_DOWN);
		        	tap_code(KC_DOWN);
		          	tap_code(KC_WH_D);
		          	delta_y = 0;
		        } else if (delta_y < -120) {
		        	tap_code(KC_UP);
		        	tap_code(KC_UP);
		        	tap_code(KC_UP);
		        	tap_code(KC_WH_U);
		          delta_y = 0;
		        };
		        right_report.x = right_report.y = 0;
	        } else if (extra_sensor_button_right == true){
	        	delta_y += right_report.y;
						if (delta_y > 120) {
		          right_report.v = -1;
		          delta_y = 0;
		        } else if (delta_y < -120) {
		        	right_report.v = 1;
		          delta_y = 0;
		        };
		        right_report.x = right_report.y = 0;
	        } else {
	        	mouse_timer = timer_read();
				if (!layer_state_is(_MOUSE_R)&&timer_elapsed(mouse_debounce_timer) > 370) {
					layer_off(_MOUSE_L);
					layer_on(_MOUSE_R);
				};
	        };
		};
		skipKeyDebounce = false;
	};
	if ((left_report.x || left_report.y)&&(timer_elapsed(mouse_timer) > 370||skipKeyDebounce==true)) {
		if (layer_state_is(_EXTRAZ)||layer_state_is(_QWERTY)){
			if (haptic_timer ==0){
				haptic_timer = timer_read();
			}
			if (layer_state_is(_QWERTY)){
				delta_y += left_report.y;
				if (delta_y > 350) {
					tap_code(KC_DOWN);
					if (timer_elapsed(haptic_timer) > 30){
						DRV_pulse(sharp_click_30);
						haptic_timer = 0;
					};
					delta_y = 0;
				} else if (delta_y < -350) {
					tap_code(KC_UP);
					if (timer_elapsed(haptic_timer) > 30){
						DRV_pulse(sharp_click_30);
						haptic_timer = 0;
					};
					delta_y = 0;
				};
			} else if (layer_state_is(_EXTRAZ)){
				delta_y += left_report.y;
				if (delta_y > 120) {
			        left_report.v = -1;
			        delta_y = 0;
		        } else if (delta_y < -120) {
		        	left_report.v = 1;
		         	delta_y = 0;
		        };
			};
			left_report.x = left_report.y = 0;
		} else {
			if (extra_sensor_button_left == true) {
				delta_y += left_report.y;
				if (delta_y > 120) {
			        left_report.v = -1;
			        delta_y = 0;
		        } else if (delta_y < -120) {
		        	left_report.v = 1;
		          	delta_y = 0;
		        };
    			left_report.x = left_report.y = 0;
			} else if (extra_sensor_button_right == true) {
				delta_y += left_report.y;
				if (delta_y > 120) {
		        	//left_report.v = -1;
		        	tap_code(KC_DOWN);
		        	tap_code(KC_DOWN);
		        	tap_code(KC_DOWN);
		        	tap_code(KC_WH_D);
		        	delta_y = 0;
		        } else if (delta_y < -120) {
		        	//left_report.v = 1;
		        	tap_code(KC_UP);
		        	tap_code(KC_UP);
		        	tap_code(KC_UP);
		        	tap_code(KC_WH_U);
		        	delta_y = 0;
		        };
		        left_report.x = left_report.y = 0;
			} else {
				mouse_timer = timer_read();
				if (!layer_state_is(_MOUSE_L)&&timer_elapsed(mouse_debounce_timer) > 370) {
					layer_off(_MOUSE_R);
					layer_on(_MOUSE_L);
				};
			};
		};
		skipKeyDebounce = false;
	};
	return pointing_device_combine_reports(left_report, right_report);
};

#endif

